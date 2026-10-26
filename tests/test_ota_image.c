#include "ota_image.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned int run_count;
static unsigned int fail_count;

#define CHECK(condition) do {                                             \
    if (!(condition)) {                                                   \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 0;                                                         \
    }                                                                     \
} while (0)

#define RUN(test) do {                                                     \
    ++run_count;                                                           \
    if (!(test())) {                                                       \
        ++fail_count;                                                      \
        fprintf(stderr, "FAIL %s\n", #test);                             \
    }                                                                     \
} while (0)

static OtaImageManifest example_manifest(void)
{
    OtaImageManifest manifest = {OTA_IMAGE_MAGIC, OTA_IMAGE_FORMAT_VERSION,
                                  4U, 9U, UINT32_C(0xCBF43926)};
    return manifest;
}

static int test_crc32_published_check_value(void)
{
    const uint8_t bytes[] = "123456789";
    uint32_t crc = 0U;
    CHECK(ota_image_crc32(bytes, 9U, &crc) == OTA_IMAGE_OK);
    CHECK(crc == UINT32_C(0xCBF43926));
    return 1;
}

static int test_crc32_incremental_matches_published_value(void)
{
    const uint8_t bytes[] = "123456789";
    uint32_t state = ota_crc32_begin();
    CHECK(ota_crc32_update(&state, bytes, 3U) == OTA_IMAGE_OK);
    CHECK(ota_crc32_update(&state, bytes + 3U, 6U) == OTA_IMAGE_OK);
    CHECK(ota_crc32_finish(state) == UINT32_C(0xCBF43926));
    return 1;
}

static int test_manifest_format_size_and_version(void)
{
    OtaImageManifest manifest = example_manifest();
    CHECK(ota_image_manifest_validate(&manifest, 9U, 4U) == OTA_IMAGE_OK);
    CHECK(ota_image_manifest_validate(&manifest, 8U, 4U) ==
          OTA_IMAGE_INVALID_SIZE);
    CHECK(ota_image_manifest_validate(&manifest, 0U, 4U) ==
          OTA_IMAGE_INVALID_ARGUMENT);
    CHECK(ota_image_manifest_validate(&manifest, 9U, 5U) ==
          OTA_IMAGE_VERSION_REJECTED);
    manifest.magic = 0U;
    CHECK(ota_image_manifest_validate(&manifest, 9U, 0U) ==
          OTA_IMAGE_UNSUPPORTED_FORMAT);
    manifest = example_manifest();
    manifest.format_version = 2U;
    CHECK(ota_image_manifest_validate(&manifest, 9U, 0U) ==
          OTA_IMAGE_UNSUPPORTED_FORMAT);
    manifest = example_manifest();
    manifest.image_size = 0U;
    CHECK(ota_image_manifest_validate(&manifest, 9U, 0U) ==
          OTA_IMAGE_INVALID_SIZE);
    CHECK(ota_image_manifest_validate(NULL, 9U, 0U) ==
          OTA_IMAGE_INVALID_ARGUMENT);
    return 1;
}

static int test_valid_and_corrupt_image(void)
{
    const uint8_t bytes[] = "123456789";
    uint8_t corrupt[9];
    OtaImageManifest manifest = example_manifest();
    CHECK(ota_image_verify_buffer(&manifest, 9U, 4U, bytes, 9U) ==
          OTA_IMAGE_OK);
    memcpy(corrupt, bytes, sizeof corrupt);
    corrupt[4] ^= 1U;
    CHECK(ota_image_verify_buffer(&manifest, 9U, 4U, corrupt, 9U) ==
          OTA_IMAGE_CRC_MISMATCH);
    return 1;
}

static int test_truncated_and_extra_bytes(void)
{
    const uint8_t bytes[] = "123456789X";
    OtaImageManifest manifest = example_manifest();
    CHECK(ota_image_verify_buffer(&manifest, 9U, 0U, bytes, 8U) ==
          OTA_IMAGE_LENGTH_MISMATCH);
    CHECK(ota_image_verify_buffer(&manifest, 9U, 0U, bytes, 10U) ==
          OTA_IMAGE_LENGTH_MISMATCH);
    CHECK(ota_image_verify_buffer(&manifest, 9U, 0U, bytes, 0U) ==
          OTA_IMAGE_LENGTH_MISMATCH);
    return 1;
}

static int test_crc_invalid_input_preserves_output(void)
{
    const uint8_t byte = 'A';
    uint32_t crc = UINT32_C(0x12345678);
    CHECK(ota_image_crc32(NULL, 1U, &crc) == OTA_IMAGE_INVALID_ARGUMENT);
    CHECK(crc == UINT32_C(0x12345678));
    CHECK(ota_image_crc32(&byte, 0U, &crc) == OTA_IMAGE_INVALID_SIZE);
    CHECK(crc == UINT32_C(0x12345678));
    CHECK(ota_image_crc32(&byte, 1U, NULL) == OTA_IMAGE_INVALID_ARGUMENT);
    return 1;
}

static int test_incremental_invalid_input_preserves_state(void)
{
    uint32_t state = ota_crc32_begin();
    CHECK(ota_crc32_update(&state, NULL, 1U) == OTA_IMAGE_INVALID_ARGUMENT);
    CHECK(state == ota_crc32_begin());
    CHECK(ota_crc32_update(NULL, NULL, 0U) == OTA_IMAGE_INVALID_ARGUMENT);
    CHECK(ota_crc32_update(&state, NULL, 0U) == OTA_IMAGE_OK);
    CHECK(state == ota_crc32_begin());
    return 1;
}

static int test_verify_checks_manifest_before_buffer(void)
{
    OtaImageManifest manifest = example_manifest();
    CHECK(ota_image_verify_buffer(&manifest, 9U, 4U, NULL, 9U) ==
          OTA_IMAGE_INVALID_ARGUMENT);
    CHECK(ota_image_verify_buffer(&manifest, 8U, 4U, NULL, 9U) ==
          OTA_IMAGE_INVALID_SIZE);
    CHECK(ota_image_verify_buffer(NULL, 9U, 4U, NULL, 9U) ==
          OTA_IMAGE_INVALID_ARGUMENT);
    return 1;
}

int main(void)
{
    RUN(test_crc32_published_check_value);
    RUN(test_crc32_incremental_matches_published_value);
    RUN(test_manifest_format_size_and_version);
    RUN(test_valid_and_corrupt_image);
    RUN(test_truncated_and_extra_bytes);
    RUN(test_crc_invalid_input_preserves_output);
    RUN(test_incremental_invalid_input_preserves_state);
    RUN(test_verify_checks_manifest_before_buffer);
    printf("OTA image: %u passed, %u failed\n",
           run_count - fail_count, fail_count);
    return fail_count == 0U ? 0 : 1;
}
