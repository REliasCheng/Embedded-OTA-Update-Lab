# Host OTA update flow

This is an original, transport-independent C11 Host model. It composes the
existing block validator, image manifest CRC32, Flash port, and fault-injectable
simulator. It does not contain a GD32 Flash driver, YMODEM or MQTT parser, a
bootloader, or a device-safe power-loss protocol.

## Reproduce

From the repository root on a machine with GCC:

```sh
gcc -std=c11 -Wall -Wextra -Werror -pedantic -I include -I host_tools \
  src/ota_block_validator.c src/ota_image.c src/ota_flash.c src/ota_update.c \
  host_tools/ota_flash_sim.c tools/ota_host_demo.c -o ota_host_demo
./ota_host_demo normal
./ota_host_demo corrupt
./ota_host_demo recovery
```

The three paths respectively show completed copy and readback, rejected
Staging corruption, and an interrupted Active copy that remains in recovery
until an explicit retry. `tests/test_ota_update.c` exercises those paths and
additional invalid input, bounds, Journal, reset, and Flash failure cases.
The GitHub workflow builds and runs the demo and tests with GCC, Clang, ASan,
and UBSan. Python range tests remain separate.

## Contract

`ota_update_init` requires disjoint, erase-aligned Active, Staging, and
Journal regions. Each is validated by the Flash port. The Journal must span
at least two erase units. The Host engine restricts the program unit to 32
bytes or fewer and requires it to divide the 32-byte Journal record. The
manifest must fit both image regions and pass its numeric minimum-version
policy. These conditions do **not** validate a target MCU's sector layout.

`begin` erases Staging and the Journal, initializes the existing sequential
block validator, and writes `RECEIVING`. It does not erase Active. Each
accepted block is validated on a copy of the session; only after the Flash
program operations succeed is accepted progress committed in RAM. A CRC or
ordering error leaves the current receive state and accepted-byte count
unchanged. A storage failure enters `RECOVERY_REQUIRED`. A short final block
is padded with `0xFF` to the program unit; only the manifest's exact image
length participates in the whole-image CRC.

After `RECEIVED`, `verify_staging` rereads the full image and checks CRC32.
Only then does it record `READY`. `apply` rechecks Staging, persists
`APPLYING`, erases Active, copies from Staging, rereads Active for CRC32, and
persists `APPLIED`. Every storage result is checked. A failed copy cannot be
reported as applied.

The Journal uses two alternate erase-unit slots, each with a 32-byte record:
magic, sequence, state, image length, image CRC32, firmware version, record
CRC32, and reserved `0xFF` bytes. Fields are encoded little endian rather
than written as a C struct. `resume` selects the highest valid matching
record; missing or corrupt records fail closed. `APPLIED` is accepted only
after Active readback matches the manifest. `READY` or `APPLYING` can be
recovered only through explicit `retry_apply`, which rechecks Staging and
copies again. A failed begin or interrupted receive is not eligible for
`retry_apply` and must restart with `begin`. An interrupted receive has no persistent accepted-byte count
and must be restarted with `begin`.

`boot_eligible` means only that this Host state machine has an `APPLIED`
record and that Active bytes still match the manifest CRC32. It does not
validate Cortex-M vectors, provenance, signatures, runtime behavior, or
physical boot safety. CRC32 detects accidental corruption and is not an
authentication mechanism. The numeric minimum-version check is not hardware
anti-rollback. Single Active plus Staging has no atomic rollback guarantee;
power loss during an actual device erase/program may require external recovery.

The Flash simulator's partial-write injection models a callback reporting
failure after writing some bytes. It is not evidence about a specific GD32
device's power-loss, wear, or programming semantics. The Journal's two-slot
ordering is a conservative Host design and cannot be promoted to device
power-fail safety without an MCU-specific adapter, sector proof, and hardware
fault-injection evidence.
