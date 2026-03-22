#![no_std]
#![no_main]

use core::panic::PanicInfo;
use heapless::String;

#[no_mangle]
#[link_section = ".text.entry"]
pub unsafe extern "C" fn blob_main(
    args_str: *const u8,
    response_buf: *mut u8,
    response_buf_len: u16,
) -> u8 {
    // Reconstruct a str slice from the raw args pointer.
    // Walk until null terminator to find length.
    let args_len = {
        let mut i = 0usize;
        while !args_str.add(i).read() == 0 {
            i += 1;
        }
        i
    };
    let args = core::str::from_utf8(core::slice::from_raw_parts(args_str, args_len))
        .unwrap_or("<invalid utf8>");

    let (output_str, return_code): (String<256>, u8) = match run(args) {
        Ok(s) => (s, 0u8),
        Err(s) => (s, 10u8),
    };

    // Copy into response_buf, respecting response_buf_len.
    // Always leave room for a null terminator.
    let bytes = output_str.as_bytes();
    let max_payload = (response_buf_len as usize).saturating_sub(1);
    let copy_len = bytes.len().min(max_payload);
    for i in 0..copy_len {
        response_buf.add(i).write_volatile(bytes[i]);
    }
    response_buf.add(copy_len).write_volatile(0u8);

    return_code
}

fn run(args: &str) -> Result<String<256>, String<256>> {
    let mut out: String<256> = String::new();

    out.push_str("Hello world from blob! args=<")
        .map_err(|_| make_err("push args prefix"))?;
    out.push_str(args).map_err(|_| make_err("push args"))?;
    out.push('>')
        .map_err(|_| make_err("push closing bracket"))?;

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
