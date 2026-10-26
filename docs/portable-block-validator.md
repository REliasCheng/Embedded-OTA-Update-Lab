# Portable Firmware Block Validator

This repository includes a small, original C11 receive validator. It checks a
declared image length against a caller-supplied capacity, requires strictly
contiguous blocks, verifies each block's CRC, and advances the caller-owned
session only after all checks pass. It does not keep or write the block data.

## Build and run on a host

```sh
gcc -std=c11 -Wall -Wextra -Werror -pedantic -I include \
  src/ota_block_validator.c tests/test_ota_block_validator.c \
  -o ota_block_validator_test
./ota_block_validator_test
```

Substitute `clang` for `gcc` to run the same tests with Clang. The existing
Python planner remains separate:

```sh
python3 -m unittest discover -s tests -v
```

## Minimal use

```c
#include "ota_block_validator.h"

#include <stdint.h>

OtaBlockSession session;
const uint8_t first_block[] = {'A'};
OtaBlockResult result;

result = ota_block_session_init(&session, 237U * 1024U, 1U, 256U,
                                0U, 0U); /* CRC-16/ARC parameters */
if (result == OTA_BLOCK_OK) {
    /* 0x30C0 is the independently checked CRC of "A" with these settings. */
    result = ota_block_accept(&session, 0U, first_block, 1U, 0x30C0U);
}
/* result == OTA_BLOCK_COMPLETE means all declared bytes were accepted. */
```

The example capacity comes from this repository's reference layout. A real
caller must supply the capacity appropriate for its own target and must keep
the block buffer valid throughout `ota_block_accept`.

## API contract

- `ota_block_session_init` requires a non-NULL session, positive capacity and
  image size, image size no larger than capacity, and a positive maximum block
  length no greater than `OTA_BLOCK_MAX_SIZE` (4096 bytes). This bound is a
  local memory policy, not a YMODEM or MQTT packet size. A failed init does not
  alter an existing session; a successful re-init resets progress to zero.
- `ota_block_crc16` computes reflected polynomial `0xA001` with explicit
  initial and final XOR values. `0x0000/0x0000` gives CRC-16/ARC; `0xFFFF/0x0000`
  gives CRC-16/MODBUS. It accepts a nonempty buffer of at most 4096 bytes and
  writes the result only on success. These parameters do not establish
  compatibility with any particular device or transport.
- `ota_block_accept` requires `offset == accepted_bytes`, a non-NULL buffer,
  `0 < length <= max_block_size`, and a block ending within the declared image.
  It returns `OTA_BLOCK_OK` for an accepted intermediate block and
  `OTA_BLOCK_COMPLETE` when the declared byte count is reached. Repeats,
  overlap, gaps, boundary errors, and CRC mismatches return distinct results.
- Every rejection leaves the session fields unchanged. The caller may resend
  the correct block at the same offset. A completed session refuses further
  data with `OTA_BLOCK_ALREADY_COMPLETE`; only an explicit successful re-init
  starts another image. `ota_block_session_is_complete` returns false for a
  NULL, uninitialized, or invalid session.

The caller owns the session and must not modify its fields between API calls.
It must also provide a valid buffer of at least `length` bytes: C cannot prove
the size of a non-NULL pointer. The module uses no dynamic allocation and no
Flash, serial, network, or hardware API.

The Python `flash_plan.py` models byte ranges for one documented Flash layout.
This C module validates an in-memory receive sequence and block CRC using
caller-supplied capacity. Neither module erases or programs Flash, parses
YMODEM/MQTT messages, authenticates an image, provides rollback, or proves a
target boot or update. Progress is RAM-only, with no persistent resume.
