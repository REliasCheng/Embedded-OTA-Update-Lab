# Flash Port and Host Simulator

The portable `OtaFlashPort` holds caller-supplied `read`, `erase`, and `program`
callbacks plus explicit total size, erase unit, and program unit. A region
identifies the only address interval a caller may access. Wrappers reject
invalid geometry, cross-region ranges, zero-length operations and misaligned
erase or program requests before invoking the backend; callback failures are
reported as `OTA_FLASH_IO_ERROR`. `ota_flash_verify` reads back and compares
data in 64-byte chunks, so it does not allocate a whole-image buffer.

`host_tools/ota_flash_sim.*` is a configurable memory-backed test backend. It
models erased bytes as `0xFF` and programming only from 1 bits to 0 bits.
Tests can fail reads or erases and inject a partial program failure after a
chosen number of bytes. A failed program may already have changed storage;
callers must not report it as an atomic success.

```sh
gcc -std=c11 -Wall -Wextra -Werror -pedantic -I include -I host_tools \
  src/ota_flash.c host_tools/ota_flash_sim.c tests/test_ota_flash.c \
  -o ota_flash_test
./ota_flash_test
```

The simulator's geometry is a test parameter, **not** an assertion about a
GD32F407VE sector map. The repository's 32/237/237/2/4 KiB Flash layout is a
reference design and has not been proven to align with target erase sectors.
No target register access, Keil build, hardware erase or firmware install is
performed by this module.
