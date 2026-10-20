import unittest

from host_tools.flash_plan import (
    ACTIVE_END,
    ACTIVE_START,
    BLOCK_SIZE,
    STAGING_START,
    STAGING_END,
    plan_image,
)


class FlashPlanTests(unittest.TestCase):
    def test_reference_partition_boundaries(self):
        self.assertEqual(ACTIVE_START, 0x08008000)
        self.assertEqual(ACTIVE_END, 0x08043400)
        self.assertEqual(STAGING_START, 0x08043400)
        self.assertEqual(STAGING_END, 0x0807E800)
        self.assertEqual(ACTIVE_END - ACTIVE_START, 237 * 1024)
        self.assertEqual(STAGING_END - STAGING_START, 237 * 1024)
        self.assertEqual(BLOCK_SIZE, 256)

    def test_one_byte_tail(self):
        blocks = plan_image(1)
        self.assertEqual([(b.source, b.destination, b.length) for b in blocks],
                         [(STAGING_START, ACTIVE_START, 1)])

    def test_exact_block(self):
        blocks = plan_image(BLOCK_SIZE)
        self.assertEqual(len(blocks), 1)
        self.assertEqual(blocks[0].length, BLOCK_SIZE)

    def test_full_block_and_tail(self):
        blocks = plan_image(BLOCK_SIZE + 1)
        self.assertEqual([(b.source, b.destination, b.length) for b in blocks],
                         [(STAGING_START, ACTIVE_START, BLOCK_SIZE),
                          (STAGING_START + BLOCK_SIZE,
                           ACTIVE_START + BLOCK_SIZE, 1)])

    def test_capacity_exactly_fits_and_never_crosses_a_region(self):
        capacity = STAGING_END - STAGING_START
        blocks = plan_image(capacity)
        self.assertEqual(len(blocks), capacity // BLOCK_SIZE)
        self.assertEqual(blocks[-1].source + blocks[-1].length, STAGING_END)
        self.assertEqual(blocks[-1].destination + blocks[-1].length,
                         ACTIVE_START + capacity)
        self.assertEqual(sum(block.length for block in blocks), capacity)

    def test_rejects_invalid_image_sizes_before_planning(self):
        for size in (0, -1, STAGING_END - STAGING_START + 1,
                     True, 1.5, "256", None):
            with self.subTest(size=size), self.assertRaises((TypeError, ValueError)):
                plan_image(size)

    def test_every_block_is_contiguous_and_within_both_regions(self):
        for size in (1, BLOCK_SIZE - 1, BLOCK_SIZE,
                     BLOCK_SIZE + 1, 2 * BLOCK_SIZE + 13):
            with self.subTest(size=size):
                blocks = plan_image(size)
                self.assertEqual(blocks[0].source, STAGING_START)
                self.assertEqual(blocks[0].destination, ACTIVE_START)
                for previous, current in zip(blocks, blocks[1:]):
                    self.assertEqual(previous.source + previous.length,
                                     current.source)
                    self.assertEqual(previous.destination + previous.length,
                                     current.destination)
                self.assertTrue(all(0 < block.length <= BLOCK_SIZE
                                    for block in blocks))
                self.assertEqual(blocks[-1].source + blocks[-1].length,
                                 STAGING_START + size)
                self.assertEqual(blocks[-1].destination + blocks[-1].length,
                                 ACTIVE_START + size)


if __name__ == "__main__":
    unittest.main()
