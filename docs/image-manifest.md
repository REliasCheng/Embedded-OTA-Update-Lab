# Image Manifest and Whole-Image Error Detection

`include/ota_image.h` declares a small caller-owned `OtaImageManifest` with a
format tag, format version, numeric firmware version, image byte count, and
expected CRC32. This C struct is **not** an on-wire or persistent format. A
future transport or storage adapter must define and validate its own byte
encoding before constructing the struct.

The core validates the tag and supported format version, a nonzero image size
within the caller's capacity, and an optional numeric minimum firmware
version. That version comparison is software policy only: it does not provide
hardware-enforced anti-rollback. `ota_image_verify_buffer` additionally
requires an exact byte count and matching CRC-32/ISO-HDLC for the entire image.
Its incremental CRC interface allows a later storage reader to verify a large
image in bounded chunks. The published `123456789` check value is `0xCBF43926`.

```sh
gcc -std=c11 -Wall -Wextra -Werror -pedantic -I include \
  src/ota_image.c tests/test_ota_image.c -o ota_image_test
./ota_image_test
```

The existing block validator detects errors in one received block; its
`OTA_BLOCK_COMPLETE` result means only that the declared byte count was
accepted. The image manifest core does not receive blocks or write Flash.
Only a later integration may connect block receipt, storage readback, and
whole-image verification. Neither CRC16 nor CRC32 authenticates a firmware
publisher or protects against deliberate modification.
