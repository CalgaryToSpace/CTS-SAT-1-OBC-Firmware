#![no_std]
#![no_main]

use core::panic::PanicInfo;
use heapless::String;
use ufmt::uwrite;

#[unsafe(no_mangle)]
#[unsafe(link_section = ".text.entry")]
pub extern "C" fn blob_main(
    args_str: *const u8,
    response_buf: *mut u8,
    response_buf_len: u16,
) -> u8 {
    let (output_str, return_code): (String<256>, u8) =
        match unsafe { core::ffi::CStr::from_ptr(args_str) }.to_str() {
            Ok(safe_args_str) => match run(safe_args_str) {
                Ok(safe_output_str) => (safe_output_str, 0u8),
                Err(safe_err_str) => (safe_err_str, 10u8),
            },
            Err(_) => (make_err("Invalid args_str - not UTF-8"), 99u8),
        };

    // Copy into response_buf as a CStr, respecting response_buf_len.
    let bytes = output_str.as_bytes();
    let max_payload = (response_buf_len as usize).saturating_sub(1);
    let copy_len = bytes.len().min(max_payload);
    unsafe {
        core::ptr::copy_nonoverlapping(bytes.as_ptr(), response_buf, copy_len);
        response_buf.add(copy_len).write(0u8);
    }

    return_code
}

fn run(args: &str) -> Result<String<256>, String<256>> {
    let mut out: String<256> = String::new();
    uwrite!(out, "Hello world from Rust blob! args_str=<{}>", args)
        .map_err(|_| make_err("uwrite! macro failed"))?;
    Ok(out)
}

fn make_err(msg: &str) -> String<256> {
    let mut s: String<256> = String::new();
    _ = uwrite!(s, "ERROR: {}", msg);
    s
}

#[panic_handler]
fn panic(_: &PanicInfo) -> ! {
    loop {}
}
