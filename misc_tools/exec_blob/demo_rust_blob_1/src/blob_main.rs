#![no_std]
#![no_main]

use core::panic::PanicInfo;

// Force this function to the dedicated entry section, mirroring .text.entry in C.
// The linker script will place .text.entry at offset 0x00000000.
#[no_mangle]
#[link_section = ".text.entry"]
pub extern "C" fn blob_main(
    _args_str: *const u8,
    response_buf: *mut u8,
    _response_buf_len: u16,
) -> u8 {
    // SAFETY: Caller guarantees response_buf is valid and large enough.
    // We write manually to avoid any str/slice machinery that might pull in core bloat.
    unsafe {
        let msg = b"Hello world from blob\0";
        for (i, &byte) in msg.iter().enumerate() {
            response_buf.add(i).write_volatile(byte);
        }
    }
    0
}

#[panic_handler]
fn panic(_: &PanicInfo) -> ! {
    loop {}
}
