#![no_std]
#![no_main]

use core::fmt::Write as _;
use core::panic::PanicInfo;
use heapless::String;

#[no_mangle]
#[link_section = ".text.entry"]
pub extern "C" fn blob_main(
    args_str: *const u8,
    response_buf: *mut u8,
    response_buf_len: u16,
) -> u8 {
    let args = unsafe { core::ffi::CStr::from_ptr(args_str) }
        .to_str()
        .unwrap_or("<invalid utf8>");

    let (output_str, return_code): (String<256>, u8) = match run(args) {
        Ok(s) => (s, 0u8),
        Err(s) => (s, 10u8),
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
    write!(out, "Hello world from Rust blob! args_str=<{}>", args)
        .map_err(|_| make_err("write!"))?;
    Ok(out)
}

fn make_err(msg: &str) -> String<256> {
    let mut s: String<256> = String::new();
    // If even this fails the string is returned empty, which is acceptable.
    let _ = s.push_str("ERR: ");
    let _ = s.push_str(msg);
    s
}

#[panic_handler]
fn panic(_: &PanicInfo) -> ! {
    loop {}
}
