"""Plan byte ranges for the documented 512 KiB OTA Flash layout.

This is an arithmetic model. It performs no erase, write, copy, CRC check,
image authentication, boot handoff, or target-specific alignment check.
"""

from dataclasses import dataclass


ACTIVE_START = 0x08008000
ACTIVE_END = 0x08043400  # exclusive
STAGING_START = 0x08043400
STAGING_END = 0x0807E800  # exclusive
BLOCK_SIZE = 256


@dataclass(frozen=True)
class CopyRange:
    source: int
    destination: int
    length: int


def plan_image(image_size: int) -> tuple[CopyRange, ...]:
    """Return contiguous source/destination ranges for one positive image size.

    The image must fit both the staging and active partitions. Ranges use
    half-open byte boundaries, so the final range may end exactly at a
    partition boundary. The last block can be shorter than 256 bytes.
    """
    if isinstance(image_size, bool) or not isinstance(image_size, int):
        raise TypeError("image_size must be an integer byte count")
    capacity = min(ACTIVE_END - ACTIVE_START, STAGING_END - STAGING_START)
    if not 0 < image_size <= capacity:
        raise ValueError(f"image_size must be between 1 and {capacity} bytes")

    return tuple(
        CopyRange(STAGING_START + offset, ACTIVE_START + offset,
                  min(BLOCK_SIZE, image_size - offset))
        for offset in range(0, image_size, BLOCK_SIZE)
    )


def main() -> None:
    import argparse

    parser = argparse.ArgumentParser(
        description="Host-only byte-range plan; does not program Flash"
    )
    parser.add_argument("image_size", type=int, help="image size in bytes")
    args = parser.parse_args()
    try:
        blocks = plan_image(args.image_size)
    except ValueError as exc:
        parser.error(str(exc))
    for block in blocks:
        print(f"0x{block.source:08X} -> 0x{block.destination:08X}"
              f"  {block.length} bytes")


if __name__ == "__main__":
    main()
