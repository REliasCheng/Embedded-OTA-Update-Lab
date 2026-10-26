#include "ota_block_validator.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned int tests_run;
static unsigned int tests_failed;

#define CHECK(condition) do {                                             \
    if (!(condition)) {                                                   \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 0;                                                         \
    }                                                                     \
} while (0)

#define RUN(test) do {                                                     \
    ++tests_run;                                                           \
    if (!(test())) {                                                       \
        ++tests_failed;                                                    \
        fprintf(stderr, "FAIL %s\n", #test);                             \
    }                                                                     \
} while (0)

static int same_session(const OtaBlockSession *a, const OtaBlockSession *b)
{
    return a->region_capacity == b->region_capacity &&
           a->image_size == b->image_size &&
           a->accepted_bytes == b->accepted_bytes &&
           a->max_block_size == b->max_block_size &&
           a->crc_initial == b->crc_initial &&
           a->crc_xorout == b->crc_xorout &&
           a->initialized == b->initialized;
}

static int init_arc(OtaBlockSession *session, uint32_t capacity,
                    uint32_t image_size, uint32_t block_size)
{
    return ota_block_session_init(session, capacity, image_size, block_size,
                                  0U, 0U) == OTA_BLOCK_OK;
}

static int accept_with_computed_crc(OtaBlockSession *session, uint32_t offset,
                                    const uint8_t *data, size_t length,
                                    OtaBlockResult expected_result)
{
    uint16_t crc = 0U;
    CHECK(ota_block_crc16(data, length, session->crc_initial,
                          session->crc_xorout, &crc) == OTA_BLOCK_OK);
    CHECK(ota_block_accept(session, offset, data, length, crc) ==
          expected_result);
    return 1;
}

static int reject_without_change(OtaBlockSession *session, uint32_t offset,
                                 const uint8_t *data, size_t length,
                                 uint16_t crc, OtaBlockResult expected)
{
    OtaBlockSession before = *session;
    CHECK(ota_block_accept(session, offset, data, length, crc) == expected);
    CHECK(same_session(session, &before));
    return 1;
}

static int test_crc_known_vectors(void)
{
    static const uint8_t vector[] = "123456789";
    uint16_t crc = 0U;
    CHECK(ota_block_crc16(vector, 9U, 0U, 0U, &crc) == OTA_BLOCK_OK);
    CHECK(crc == UINT16_C(0xBB3D)); /* CRC-16/ARC published check value */
    CHECK(ota_block_crc16(vector, 9U, UINT16_C(0xFFFF), 0U, &crc) ==
          OTA_BLOCK_OK);
    CHECK(crc == UINT16_C(0x4B37)); /* CRC-16/MODBUS check value */
    return 1;
}

static int test_crc_single_byte_and_configuration(void)
{
    static const uint8_t data[] = {'A'};
    uint16_t crc = 0U;
    CHECK(ota_block_crc16(data, 1U, 0U, 0U, &crc) == OTA_BLOCK_OK);
    CHECK(crc == UINT16_C(0x30C0));
    CHECK(ota_block_crc16(data, 1U, UINT16_C(0xFFFF), 0U, &crc) ==
          OTA_BLOCK_OK);
    CHECK(crc == UINT16_C(0x707F));
    CHECK(ota_block_crc16(data, 1U, 0U, UINT16_C(0xFFFF), &crc) ==
          OTA_BLOCK_OK);
    CHECK(crc == UINT16_C(0xCF3F));
    return 1;
}

static int test_crc_bad_inputs_do_not_write_output(void)
{
    const uint8_t byte = 'A';
    uint16_t crc = UINT16_C(0xCAFE);
    CHECK(ota_block_crc16(NULL, 1U, 0U, 0U, &crc) ==
          OTA_BLOCK_INVALID_PARAMETER);
    CHECK(crc == UINT16_C(0xCAFE));
    CHECK(ota_block_crc16(&byte, 1U, 0U, 0U, NULL) ==
          OTA_BLOCK_INVALID_PARAMETER);
    CHECK(ota_block_crc16(&byte, 0U, 0U, 0U, &crc) ==
          OTA_BLOCK_INVALID_BLOCK_LENGTH);
    CHECK(ota_block_crc16(&byte, SIZE_MAX, 0U, 0U, &crc) ==
          OTA_BLOCK_INVALID_BLOCK_LENGTH);
    CHECK(crc == UINT16_C(0xCAFE));
    return 1;
}

static int test_init_rejects_invalid_values_without_mutation(void)
{
    OtaBlockSession session;
    OtaBlockSession before;
    CHECK(ota_block_session_init(NULL, 1U, 1U, 1U, 0U, 0U) ==
          OTA_BLOCK_INVALID_PARAMETER);
    CHECK(init_arc(&session, 100U, 10U, 4U));
    before = session;
    CHECK(ota_block_session_init(&session, 0U, 1U, 1U, 0U, 0U) ==
          OTA_BLOCK_INVALID_IMAGE_SIZE);
    CHECK(same_session(&session, &before));
    CHECK(ota_block_session_init(&session, 10U, 0U, 1U, 0U, 0U) ==
          OTA_BLOCK_INVALID_IMAGE_SIZE);
    CHECK(same_session(&session, &before));
    CHECK(ota_block_session_init(&session, 10U, 11U, 1U, 0U, 0U) ==
          OTA_BLOCK_INVALID_IMAGE_SIZE);
    CHECK(same_session(&session, &before));
    CHECK(ota_block_session_init(&session, 10U, 1U, 0U, 0U, 0U) ==
          OTA_BLOCK_INVALID_BLOCK_LENGTH);
    CHECK(same_session(&session, &before));
    CHECK(ota_block_session_init(&session, 10U, 1U,
                                 OTA_BLOCK_MAX_SIZE + 1U, 0U, 0U) ==
          OTA_BLOCK_INVALID_BLOCK_LENGTH);
    CHECK(same_session(&session, &before));
    return 1;
}

static int test_single_block_and_completion(void)
{
    static const uint8_t data[] = "123456789";
    OtaBlockSession session;
    CHECK(init_arc(&session, 9U, 9U, 9U));
    CHECK(session.accepted_bytes == 0U);
    CHECK(!ota_block_session_is_complete(&session));
    CHECK(ota_block_accept(&session, 0U, data, 9U, UINT16_C(0xBB3D)) ==
          OTA_BLOCK_COMPLETE);
    CHECK(session.accepted_bytes == 9U);
    CHECK(ota_block_session_is_complete(&session));
    CHECK(reject_without_change(&session, 9U, data, 1U, 0U,
                                OTA_BLOCK_ALREADY_COMPLETE));
    return 1;
}

static int test_256_block_plus_tail(void)
{
    uint8_t data[257] = {0};
    OtaBlockSession session;
    CHECK(init_arc(&session, 257U, 257U, 256U));
    CHECK(accept_with_computed_crc(&session, 0U, data, 256U, OTA_BLOCK_OK));
    CHECK(session.accepted_bytes == 256U);
    CHECK(accept_with_computed_crc(&session, 256U, data + 256U, 1U,
                                   OTA_BLOCK_COMPLETE));
    CHECK(ota_block_session_is_complete(&session));
    return 1;
}

static int test_multiple_full_blocks_and_tail(void)
{
    uint8_t data[525] = {0};
    OtaBlockSession session;
    CHECK(init_arc(&session, 600U, 525U, 256U));
    CHECK(accept_with_computed_crc(&session, 0U, data, 256U, OTA_BLOCK_OK));
    CHECK(accept_with_computed_crc(&session, 256U, data + 256U, 256U,
                                   OTA_BLOCK_OK));
    CHECK(accept_with_computed_crc(&session, 512U, data + 512U, 13U,
                                   OTA_BLOCK_COMPLETE));
    CHECK(session.accepted_bytes == 525U);
    return 1;
}

static int test_exact_256_and_configurable_block_size(void)
{
    uint8_t data[256] = {0};
    OtaBlockSession session;
    CHECK(init_arc(&session, 256U, 256U, 256U));
    CHECK(accept_with_computed_crc(&session, 0U, data, 256U,
                                   OTA_BLOCK_COMPLETE));
    CHECK(init_arc(&session, 300U, 6U, 3U));
    CHECK(accept_with_computed_crc(&session, 0U, data, 3U, OTA_BLOCK_OK));
    CHECK(accept_with_computed_crc(&session, 3U, data + 3U, 3U,
                                   OTA_BLOCK_COMPLETE));
    return 1;
}

static int test_largest_permitted_block(void)
{
    uint8_t data[OTA_BLOCK_MAX_SIZE] = {0};
    OtaBlockSession session;
    CHECK(init_arc(&session, OTA_BLOCK_MAX_SIZE, OTA_BLOCK_MAX_SIZE,
                   OTA_BLOCK_MAX_SIZE));
    CHECK(accept_with_computed_crc(&session, 0U, data, sizeof data,
                                   OTA_BLOCK_COMPLETE));
    return 1;
}

static int test_uninitialized_and_corrupt_session(void)
{
    static const uint8_t byte = 'A';
    OtaBlockSession session = {0};
    CHECK(reject_without_change(&session, 0U, &byte, 1U, UINT16_C(0x30C0),
                                OTA_BLOCK_UNINITIALIZED));
    CHECK(!ota_block_session_is_complete(NULL));
    CHECK(!ota_block_session_is_complete(&session));
    CHECK(ota_block_accept(NULL, 0U, &byte, 1U, 0U) ==
          OTA_BLOCK_INVALID_PARAMETER);
    CHECK(init_arc(&session, 5U, 1U, 1U));
    session.accepted_bytes = 2U;
    CHECK(reject_without_change(&session, 0U, &byte, 1U, 0U,
                                OTA_BLOCK_INVALID_STATE));
    CHECK(!ota_block_session_is_complete(&session));
    return 1;
}

static int test_null_zero_oversize_and_remaining_length(void)
{
    static const uint8_t data[] = "123456789";
    OtaBlockSession session;
    CHECK(init_arc(&session, 9U, 9U, 9U));
    CHECK(reject_without_change(&session, 0U, NULL, 1U, 0U,
                                OTA_BLOCK_INVALID_PARAMETER));
    CHECK(reject_without_change(&session, 0U, data, 0U, 0U,
                                OTA_BLOCK_INVALID_BLOCK_LENGTH));
    CHECK(reject_without_change(&session, 0U, data, 10U, 0U,
                                OTA_BLOCK_INVALID_BLOCK_LENGTH));
    CHECK(reject_without_change(&session, 0U, data, SIZE_MAX, 0U,
                                OTA_BLOCK_INVALID_BLOCK_LENGTH));
    CHECK(accept_with_computed_crc(&session, 0U, data, 8U, OTA_BLOCK_OK));
    CHECK(reject_without_change(&session, 8U, data, 2U, 0U,
                                OTA_BLOCK_OUT_OF_BOUNDS));
    CHECK(accept_with_computed_crc(&session, 8U, data + 8U, 1U,
                                   OTA_BLOCK_COMPLETE));
    return 1;
}

static int test_offsets_duplicates_overlap_and_recovery(void)
{
    static const uint8_t data[] = "123456789";
    OtaBlockSession session;
    uint16_t crc = 0U;
    CHECK(init_arc(&session, 9U, 9U, 9U));
    CHECK(reject_without_change(&session, 10U, data, 1U, 0U,
                                OTA_BLOCK_OUT_OF_BOUNDS));
    CHECK(reject_without_change(&session, 9U, data, 1U, 0U,
                                OTA_BLOCK_OUT_OF_BOUNDS));
    CHECK(reject_without_change(&session, 1U, data, 1U, 0U,
                                OTA_BLOCK_OUT_OF_ORDER));
    CHECK(accept_with_computed_crc(&session, 0U, data, 3U, OTA_BLOCK_OK));
    CHECK(reject_without_change(&session, 0U, data, 3U, 0U,
                                OTA_BLOCK_DUPLICATE_OR_OVERLAP));
    CHECK(reject_without_change(&session, 2U, data + 2U, 3U, 0U,
                                OTA_BLOCK_DUPLICATE_OR_OVERLAP));
    CHECK(reject_without_change(&session, 4U, data + 4U, 1U, 0U,
                                OTA_BLOCK_OUT_OF_ORDER));
    CHECK(ota_block_crc16(data + 3U, 3U, 0U, 0U, &crc) == OTA_BLOCK_OK);
    CHECK(reject_without_change(&session, 3U, data + 3U, 3U,
                                (uint16_t)(crc ^ 1U),
                                OTA_BLOCK_CRC_MISMATCH));
    CHECK(accept_with_computed_crc(&session, 3U, data + 3U, 6U,
                                   OTA_BLOCK_COMPLETE));
    return 1;
}

static int test_near_uint32_max_and_subtraction_guard(void)
{
    static const uint8_t data[4] = {1U, 2U, 3U, 4U};
    OtaBlockSession session;
    CHECK(init_arc(&session, UINT32_MAX, UINT32_MAX, 4U));
    CHECK(reject_without_change(&session, UINT32_MAX, data, 1U, 0U,
                                OTA_BLOCK_OUT_OF_BOUNDS));
    /* Model a near-end receiver state without allocating a 4 GiB image. */
    session.accepted_bytes = UINT32_MAX - 2U;
    CHECK(reject_without_change(&session, UINT32_MAX - 2U, data, 4U, 0U,
                                OTA_BLOCK_OUT_OF_BOUNDS));
    CHECK(accept_with_computed_crc(&session, UINT32_MAX - 2U, data, 2U,
                                   OTA_BLOCK_COMPLETE));
    return 1;
}

static int test_reinitialization_contract(void)
{
    static const uint8_t data[] = {'A'};
    OtaBlockSession session;
    CHECK(init_arc(&session, 10U, 2U, 1U));
    CHECK(accept_with_computed_crc(&session, 0U, data, 1U, OTA_BLOCK_OK));
    CHECK(ota_block_session_init(&session, 2U, 1U, 1U,
                                 UINT16_C(0xFFFF), 0U) == OTA_BLOCK_OK);
    CHECK(session.accepted_bytes == 0U);
    CHECK(session.image_size == 1U);
    CHECK(session.crc_initial == UINT16_C(0xFFFF));
    CHECK(!ota_block_session_is_complete(&session));
    CHECK(ota_block_accept(&session, 0U, data, 1U, UINT16_C(0x707F)) ==
          OTA_BLOCK_COMPLETE);
    return 1;
}

int main(void)
{
    RUN(test_crc_known_vectors);
    RUN(test_crc_single_byte_and_configuration);
    RUN(test_crc_bad_inputs_do_not_write_output);
    RUN(test_init_rejects_invalid_values_without_mutation);
    RUN(test_single_block_and_completion);
    RUN(test_256_block_plus_tail);
    RUN(test_multiple_full_blocks_and_tail);
    RUN(test_exact_256_and_configurable_block_size);
    RUN(test_largest_permitted_block);
    RUN(test_uninitialized_and_corrupt_session);
    RUN(test_null_zero_oversize_and_remaining_length);
    RUN(test_offsets_duplicates_overlap_and_recovery);
    RUN(test_near_uint32_max_and_subtraction_guard);
    RUN(test_reinitialization_contract);
    printf("OTA block validator: %u passed, %u failed\n",
           tests_run - tests_failed, tests_failed);
    return tests_failed == 0U ? 0 : 1;
}
