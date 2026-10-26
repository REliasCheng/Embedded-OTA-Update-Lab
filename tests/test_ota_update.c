#include "ota_update.h"
#include "ota_flash_sim.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define IMAGE_SIZE 257U
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); return 0; } } while (0)
#define RUN(x) do { ++runs; if (!(x())) { ++fails; fprintf(stderr, "FAIL %s\n", #x); } } while (0)

static unsigned int runs, fails;
static const OtaFlashRegion active = {0U, 1024U};
static const OtaFlashRegion staging = {1024U, 1024U};
static const OtaFlashRegion journal = {2048U, 128U};

typedef struct {
    uint8_t memory[4096];
    uint8_t image[IMAGE_SIZE];
    OtaFlashSim sim;
    OtaFlashPort port;
    OtaImageManifest manifest;
    OtaUpdate update;
} Fixture;

static int setup(Fixture *f)
{
    uint32_t crc;
    size_t i;
    for (i = 0U; i < IMAGE_SIZE; ++i) {
        f->image[i] = (uint8_t)(i * 7U + 3U);
    }
    CHECK(ota_image_crc32(f->image, IMAGE_SIZE, &crc) == OTA_IMAGE_OK);
    f->manifest = (OtaImageManifest){OTA_IMAGE_MAGIC, OTA_IMAGE_FORMAT_VERSION,
                                     2U, IMAGE_SIZE, crc};
    CHECK(ota_flash_sim_init(&f->sim, f->memory, sizeof f->memory,
                             (OtaFlashGeometry){4096U, 64U, 4U},
                             &f->port) == OTA_FLASH_OK);
    CHECK(ota_update_init(&f->update, &f->port, active, staging, journal,
                          &f->manifest, 1U, 256U, 0xFFFFU, 0U) == OTA_UPDATE_OK);
    return 1;
}

static int receive_image(Fixture *f)
{
    uint16_t crc;
    CHECK(ota_update_begin(&f->update) == OTA_UPDATE_OK);
    CHECK(ota_block_crc16(f->image, 256U, 0xFFFFU, 0U, &crc) == OTA_BLOCK_OK);
    CHECK(ota_update_receive(&f->update, 0U, f->image, 256U, crc) == OTA_UPDATE_OK);
    CHECK(ota_block_crc16(f->image + 256U, 1U, 0xFFFFU, 0U, &crc) == OTA_BLOCK_OK);
    CHECK(ota_update_receive(&f->update, 256U, f->image + 256U, 1U, crc) == OTA_UPDATE_COMPLETE);
    return 1;
}

static int test_normal_flow(void)
{
    Fixture f;
    CHECK(setup(&f));
    CHECK(receive_image(&f));
    CHECK(!ota_update_boot_eligible(&f.update));
    CHECK(ota_update_verify_staging(&f.update) == OTA_UPDATE_OK);
    CHECK(ota_update_apply(&f.update) == OTA_UPDATE_OK);
    CHECK(memcmp(f.memory + active.start, f.image, IMAGE_SIZE) == 0);
    CHECK(ota_update_boot_eligible(&f.update));
    CHECK(ota_update_receive(&f.update, 0U, f.image, 1U, 0U) == OTA_UPDATE_INVALID_STATE);
    return 1;
}

static int test_crc_reject_does_not_advance(void)
{
    Fixture f;
    uint16_t crc;
    CHECK(setup(&f));
    CHECK(ota_update_begin(&f.update) == OTA_UPDATE_OK);
    CHECK(ota_block_crc16(f.image, 256U, 0xFFFFU, 0U, &crc) == OTA_BLOCK_OK);
    CHECK(ota_update_receive(&f.update, 0U, f.image, 256U,
                             (uint16_t)(crc ^ 1U)) == OTA_UPDATE_BLOCK_REJECTED);
    CHECK(f.update.blocks.accepted_bytes == 0U);
    CHECK(f.memory[staging.start] == 0xFFU);
    CHECK(ota_update_receive(&f.update, 0U, f.image, 256U, crc) == OTA_UPDATE_OK);
    CHECK(f.update.blocks.accepted_bytes == 256U);
    return 1;
}

static int test_out_of_order_and_alignment(void)
{
    Fixture f;
    uint16_t crc;
    CHECK(setup(&f));
    CHECK(ota_update_begin(&f.update) == OTA_UPDATE_OK);
    CHECK(ota_block_crc16(f.image, 1U, 0xFFFFU, 0U, &crc) == OTA_BLOCK_OK);
    CHECK(ota_update_receive(&f.update, 4U, f.image, 1U, crc) == OTA_UPDATE_BLOCK_REJECTED);
    CHECK(ota_update_receive(&f.update, 0U, f.image, 1U, crc) == OTA_UPDATE_BLOCK_REJECTED);
    CHECK(f.update.blocks.accepted_bytes == 0U);
    return 1;
}

static int test_corrupt_staging_rejected_before_active_erase(void)
{
    Fixture f;
    CHECK(setup(&f));
    CHECK(receive_image(&f));
    f.memory[staging.start + 8U] ^= 1U;
    CHECK(ota_update_verify_staging(&f.update) == OTA_UPDATE_IMAGE_ERROR);
    CHECK(f.update.state == OTA_UPDATE_INVALID_IMAGE);
    CHECK(f.memory[active.start] == 0xFFU);
    CHECK(!ota_update_boot_eligible(&f.update));
    return 1;
}

static int test_program_failure_blocks_progress(void)
{
    Fixture f;
    uint16_t crc;
    CHECK(setup(&f));
    CHECK(ota_update_begin(&f.update) == OTA_UPDATE_OK);
    CHECK(ota_block_crc16(f.image, 256U, 0xFFFFU, 0U, &crc) == OTA_BLOCK_OK);
    f.sim.programmed_bytes = 0U;
    f.sim.fail_program_after = 3U;
    CHECK(ota_update_receive(&f.update, 0U, f.image, 256U, crc) == OTA_UPDATE_STORAGE_ERROR);
    CHECK(f.update.blocks.accepted_bytes == 0U);
    CHECK(f.update.state == OTA_UPDATE_RECOVERY_REQUIRED);
    CHECK(!ota_update_boot_eligible(&f.update));
    CHECK(ota_update_retry_apply(&f.update) == OTA_UPDATE_INVALID_STATE);
    CHECK(ota_update_receive(&f.update, 0U, f.image, 256U, crc) == OTA_UPDATE_INVALID_STATE);
    return 1;
}

static int test_interrupted_apply_and_explicit_retry(void)
{
    Fixture f;
    OtaUpdate rebooted;
    CHECK(setup(&f));
    CHECK(receive_image(&f));
    CHECK(ota_update_verify_staging(&f.update) == OTA_UPDATE_OK);
    f.sim.programmed_bytes = 0U;
    f.sim.fail_program_after = 40U;
    CHECK(ota_update_apply(&f.update) == OTA_UPDATE_STORAGE_ERROR);
    CHECK(f.update.state == OTA_UPDATE_RECOVERY_REQUIRED);
    CHECK(!ota_update_boot_eligible(&f.update));
    f.sim.fail_program_after = SIZE_MAX;
    CHECK(ota_update_init(&rebooted, &f.port, active, staging, journal,
                          &f.manifest, 1U, 256U, 0xFFFFU, 0U) == OTA_UPDATE_OK);
    CHECK(ota_update_resume(&rebooted) == OTA_UPDATE_RECOVERY_NEEDED);
    CHECK(rebooted.state == OTA_UPDATE_RECOVERY_REQUIRED);
    CHECK(ota_update_retry_apply(&rebooted) == OTA_UPDATE_OK);
    CHECK(ota_update_boot_eligible(&rebooted));
    return 1;
}

static int test_applied_resume_and_corruption(void)
{
    Fixture f;
    OtaUpdate rebooted;
    CHECK(setup(&f));
    CHECK(receive_image(&f));
    CHECK(ota_update_verify_staging(&f.update) == OTA_UPDATE_OK);
    CHECK(ota_update_apply(&f.update) == OTA_UPDATE_OK);
    CHECK(ota_update_init(&rebooted, &f.port, active, staging, journal,
                          &f.manifest, 1U, 256U, 0xFFFFU, 0U) == OTA_UPDATE_OK);
    CHECK(ota_update_resume(&rebooted) == OTA_UPDATE_OK);
    CHECK(ota_update_boot_eligible(&rebooted));
    f.memory[active.start + 6U] ^= 1U;
    CHECK(!ota_update_boot_eligible(&rebooted));
    CHECK(rebooted.state == OTA_UPDATE_INVALID_IMAGE);
    return 1;
}

static int test_corrupt_journal_rejected(void)
{
    Fixture f;
    OtaUpdate rebooted;
    CHECK(setup(&f));
    CHECK(ota_update_begin(&f.update) == OTA_UPDATE_OK);
    f.memory[journal.start + 24U] ^= 1U;
    CHECK(ota_update_init(&rebooted, &f.port, active, staging, journal,
                          &f.manifest, 1U, 256U, 0xFFFFU, 0U) == OTA_UPDATE_OK);
    CHECK(ota_update_resume(&rebooted) == OTA_UPDATE_RECOVERY_NEEDED);
    CHECK(!ota_update_boot_eligible(&rebooted));
    return 1;
}

static int test_metadata_and_region_rejection(void)
{
    Fixture f;
    OtaUpdate bad;
    CHECK(setup(&f));
    CHECK(ota_update_init(&bad, &f.port, active, active, journal,
                          &f.manifest, 1U, 256U, 0xFFFFU, 0U) == OTA_UPDATE_INVALID_ARGUMENT);
    f.manifest.firmware_version = 0U;
    CHECK(ota_update_init(&bad, &f.port, active, staging, journal,
                          &f.manifest, 1U, 256U, 0xFFFFU, 0U) == OTA_UPDATE_INVALID_ARGUMENT);
    return 1;
}

static int test_journal_write_failure(void)
{
    Fixture f;
    CHECK(setup(&f));
    f.sim.fail_program_after = 0U;
    CHECK(ota_update_begin(&f.update) == OTA_UPDATE_JOURNAL_ERROR);
    CHECK(f.update.state == OTA_UPDATE_RECOVERY_REQUIRED);
    CHECK(ota_update_retry_apply(&f.update) == OTA_UPDATE_INVALID_STATE);
    CHECK(!ota_update_boot_eligible(&f.update));
    f.sim.fail_program_after = SIZE_MAX;
    CHECK(ota_update_begin(&f.update) == OTA_UPDATE_OK);
    return 1;
}

static int test_erase_failure_and_recovery(void)
{
    Fixture f;
    CHECK(setup(&f));
    f.sim.fail_erase = 1;
    CHECK(ota_update_begin(&f.update) == OTA_UPDATE_STORAGE_ERROR);
    CHECK(f.update.state == OTA_UPDATE_RECOVERY_REQUIRED);
    f.sim.fail_erase = 0;
    CHECK(receive_image(&f));
    CHECK(ota_update_verify_staging(&f.update) == OTA_UPDATE_OK);
    f.sim.fail_erase_at = active.start;
    CHECK(ota_update_apply(&f.update) == OTA_UPDATE_STORAGE_ERROR);
    CHECK(!ota_update_boot_eligible(&f.update));
    f.sim.fail_erase_at = UINT32_MAX;
    CHECK(ota_update_retry_apply(&f.update) == OTA_UPDATE_OK);
    CHECK(ota_update_boot_eligible(&f.update));
    return 1;
}

static int test_read_failure_and_incomplete_reset(void)
{
    Fixture f;
    OtaUpdate rebooted;
    CHECK(setup(&f));
    CHECK(ota_update_begin(&f.update) == OTA_UPDATE_OK);
    CHECK(ota_update_init(&rebooted, &f.port, active, staging, journal,
                          &f.manifest, 1U, 256U, 0xFFFFU, 0U) == OTA_UPDATE_OK);
    CHECK(ota_update_resume(&rebooted) == OTA_UPDATE_RECOVERY_NEEDED);
    CHECK(!ota_update_boot_eligible(&rebooted));
    CHECK(receive_image(&f));
    f.sim.fail_read = 1;
    CHECK(ota_update_verify_staging(&f.update) == OTA_UPDATE_STORAGE_ERROR);
    CHECK(f.update.state == OTA_UPDATE_RECOVERY_REQUIRED);
    f.sim.fail_read = 0;
    CHECK(ota_update_begin(&f.update) == OTA_UPDATE_OK);
    return 1;
}

static int test_repeated_update_and_active_corruption(void)
{
    Fixture f;
    CHECK(setup(&f));
    CHECK(receive_image(&f));
    CHECK(ota_update_verify_staging(&f.update) == OTA_UPDATE_OK);
    CHECK(ota_update_apply(&f.update) == OTA_UPDATE_OK);
    CHECK(ota_update_begin(&f.update) == OTA_UPDATE_OK);
    CHECK(!ota_update_boot_eligible(&f.update));
    CHECK(receive_image(&f));
    CHECK(ota_update_verify_staging(&f.update) == OTA_UPDATE_OK);
    CHECK(ota_update_apply(&f.update) == OTA_UPDATE_OK);
    CHECK(ota_update_boot_eligible(&f.update));
    return 1;
}

static int test_exact_capacity_and_invalid_sizes(void)
{
    Fixture f;
    OtaUpdate update;
    uint8_t image[1024];
    uint32_t crc;
    uint16_t block_crc;
    unsigned int i;
    CHECK(setup(&f));
    for (i = 0U; i < sizeof image; ++i) {
        image[i] = (uint8_t)(i ^ 0x5AU);
    }
    CHECK(ota_image_crc32(image, sizeof image, &crc) == OTA_IMAGE_OK);
    f.manifest.image_size = 0U;
    CHECK(ota_update_init(&update, &f.port, active, staging, journal,
        &f.manifest, 1U, 256U, 0xFFFFU, 0U) == OTA_UPDATE_INVALID_ARGUMENT);
    f.manifest.image_size = 1025U;
    CHECK(ota_update_init(&update, &f.port, active, staging, journal,
        &f.manifest, 1U, 256U, 0xFFFFU, 0U) == OTA_UPDATE_INVALID_ARGUMENT);
    f.manifest.image_size = sizeof image;
    f.manifest.image_crc32 = crc;
    CHECK(ota_update_init(&update, &f.port, active, staging, journal,
        &f.manifest, 1U, 256U, 0xFFFFU, 0U) == OTA_UPDATE_OK);
    CHECK(ota_update_begin(&update) == OTA_UPDATE_OK);
    for (i = 0U; i < sizeof image; i += 256U) {
        CHECK(ota_block_crc16(image + i, 256U, 0xFFFFU, 0U,
                              &block_crc) == OTA_BLOCK_OK);
        CHECK(ota_update_receive(&update, i, image + i, 256U,
                  block_crc) == (i == 768U ? OTA_UPDATE_COMPLETE : OTA_UPDATE_OK));
    }
    CHECK(ota_update_verify_staging(&update) == OTA_UPDATE_OK);
    CHECK(ota_update_apply(&update) == OTA_UPDATE_OK);
    CHECK(memcmp(f.memory + active.start, image, sizeof image) == 0);
    return 1;
}

static int test_duplicate_and_overflow_offsets(void)
{
    Fixture f;
    uint16_t crc;
    CHECK(setup(&f));
    CHECK(ota_update_begin(&f.update) == OTA_UPDATE_OK);
    CHECK(ota_block_crc16(f.image, 256U, 0xFFFFU, 0U, &crc) == OTA_BLOCK_OK);
    CHECK(ota_update_receive(&f.update, 0U, f.image, 256U, crc) == OTA_UPDATE_OK);
    CHECK(ota_update_receive(&f.update, 0U, f.image, 256U, crc) == OTA_UPDATE_BLOCK_REJECTED);
    CHECK(ota_update_receive(&f.update, UINT32_MAX, f.image, 256U,
                             crc) == OTA_UPDATE_BLOCK_REJECTED);
    CHECK(f.update.blocks.accepted_bytes == 256U);
    return 1;
}

static int test_applied_journal_partial_write_fails_closed(void)
{
    Fixture f;
    OtaUpdate rebooted;
    CHECK(setup(&f));
    CHECK(receive_image(&f));
    CHECK(ota_update_verify_staging(&f.update) == OTA_UPDATE_OK);
    /* APPLYING record (32) + rounded image (260) + 3 record bytes. */
    f.sim.programmed_bytes = 0U;
    f.sim.fail_program_after = 295U;
    CHECK(ota_update_apply(&f.update) == OTA_UPDATE_JOURNAL_ERROR);
    CHECK(memcmp(f.memory + active.start, f.image, IMAGE_SIZE) == 0);
    CHECK(!ota_update_boot_eligible(&f.update));
    f.sim.fail_program_after = SIZE_MAX;
    CHECK(ota_update_init(&rebooted, &f.port, active, staging, journal,
                          &f.manifest, 1U, 256U, 0xFFFFU, 0U) == OTA_UPDATE_OK);
    CHECK(ota_update_resume(&rebooted) == OTA_UPDATE_RECOVERY_NEEDED);
    CHECK(!ota_update_boot_eligible(&rebooted));
    CHECK(ota_update_retry_apply(&rebooted) == OTA_UPDATE_OK);
    CHECK(ota_update_boot_eligible(&rebooted));
    return 1;
}

int main(void)
{
    RUN(test_normal_flow);
    RUN(test_crc_reject_does_not_advance);
    RUN(test_out_of_order_and_alignment);
    RUN(test_corrupt_staging_rejected_before_active_erase);
    RUN(test_program_failure_blocks_progress);
    RUN(test_interrupted_apply_and_explicit_retry);
    RUN(test_applied_resume_and_corruption);
    RUN(test_corrupt_journal_rejected);
    RUN(test_metadata_and_region_rejection);
    RUN(test_journal_write_failure);
    RUN(test_erase_failure_and_recovery);
    RUN(test_read_failure_and_incomplete_reset);
    RUN(test_repeated_update_and_active_corruption);
    RUN(test_exact_capacity_and_invalid_sizes);
    RUN(test_duplicate_and_overflow_offsets);
    RUN(test_applied_journal_partial_write_fails_closed);
    printf("OTA update: %u tests, %u failures\n", runs, fails);
    return fails == 0U ? 0 : 1;
}
