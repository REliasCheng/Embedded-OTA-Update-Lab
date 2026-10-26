#include "ota_flash.h"
#include "ota_flash_sim.h"

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
    if (!(test())) { ++fail_count; fprintf(stderr, "FAIL %s\n", #test); } \
} while (0)

static OtaFlashGeometry geometry(void)
{
    OtaFlashGeometry g = {512U, 64U, 4U};
    return g;
}

static OtaFlashRegion staging(void)
{
    OtaFlashRegion r = {256U, 128U};
    return r;
}

static int make_port(OtaFlashSim *sim, uint8_t *memory, OtaFlashPort *port)
{
    return ota_flash_sim_init(sim, memory, 512U, geometry(), port) ==
           OTA_FLASH_OK;
}

static int test_program_read_verify_erase(void)
{
    uint8_t memory[512];
    uint8_t data[8] = {0x01U, 0x02U, 0x03U, 0x04U,
                       0x05U, 0x06U, 0x07U, 0x08U};
    uint8_t result[8] = {0};
    OtaFlashSim sim;
    OtaFlashPort port;
    CHECK(make_port(&sim, memory, &port));
    CHECK(ota_flash_program(&port, staging(), 256U, data, sizeof data) ==
          OTA_FLASH_OK);
    CHECK(ota_flash_read(&port, staging(), 256U, result, sizeof result) ==
          OTA_FLASH_OK);
    CHECK(memcmp(result, data, sizeof data) == 0);
    CHECK(ota_flash_verify(&port, staging(), 256U, data, sizeof data) ==
          OTA_FLASH_OK);
    CHECK(ota_flash_erase(&port, staging(), 256U, 64U) == OTA_FLASH_OK);
    CHECK(memory[256] == 0xFFU && memory[263] == 0xFFU);
    return 1;
}

static int test_region_bounds_and_alignment(void)
{
    uint8_t memory[512];
    uint8_t data[8] = {0};
    OtaFlashSim sim;
    OtaFlashPort port;
    OtaFlashRegion invalid = {300U, 128U};
    CHECK(make_port(&sim, memory, &port));
    CHECK(ota_flash_region_validate(&port, staging()) == OTA_FLASH_OK);
    CHECK(ota_flash_region_validate(&port, invalid) == OTA_FLASH_MISALIGNED);
    CHECK(ota_flash_program(&port, staging(), 252U, data, 4U) ==
          OTA_FLASH_OUT_OF_RANGE);
    CHECK(ota_flash_program(&port, staging(), 380U, data, 8U) ==
          OTA_FLASH_OUT_OF_RANGE);
    CHECK(ota_flash_read(&port, staging(), UINT32_MAX, data, 4U) ==
          OTA_FLASH_OUT_OF_RANGE);
    CHECK(ota_flash_program(&port, staging(), 257U, data, 4U) ==
          OTA_FLASH_MISALIGNED);
    CHECK(ota_flash_program(&port, staging(), 256U, data, 3U) ==
          OTA_FLASH_MISALIGNED);
    CHECK(ota_flash_erase(&port, staging(), 260U, 64U) ==
          OTA_FLASH_MISALIGNED);
    CHECK(ota_flash_erase(&port, staging(), 256U, 60U) ==
          OTA_FLASH_MISALIGNED);
    CHECK(memory[256] == 0xFFU);
    return 1;
}

static int test_invalid_geometry_and_arguments(void)
{
    uint8_t memory[512];
    uint8_t data[4] = {0};
    OtaFlashSim sim;
    OtaFlashPort port;
    OtaFlashGeometry g = geometry();
    CHECK(ota_flash_sim_init(NULL, memory, 512U, g, &port) ==
          OTA_FLASH_INVALID_ARGUMENT);
    CHECK(ota_flash_sim_init(&sim, memory, 511U, g, &port) ==
          OTA_FLASH_INVALID_ARGUMENT);
    g.program_unit = 3U;
    CHECK(ota_flash_sim_init(&sim, memory, 512U, g, &port) ==
          OTA_FLASH_INVALID_GEOMETRY);
    CHECK(make_port(&sim, memory, &port));
    CHECK(ota_flash_program(&port, staging(), 256U, NULL, 4U) ==
          OTA_FLASH_INVALID_ARGUMENT);
    CHECK(ota_flash_read(&port, staging(), 256U, NULL, 4U) ==
          OTA_FLASH_INVALID_ARGUMENT);
    CHECK(ota_flash_verify(&port, staging(), 256U, NULL, 4U) ==
          OTA_FLASH_INVALID_ARGUMENT);
    CHECK(ota_flash_program(&port, staging(), 256U, data, 0U) ==
          OTA_FLASH_INVALID_ARGUMENT);
    CHECK(ota_flash_read(&port, staging(), 256U, data, SIZE_MAX) ==
          OTA_FLASH_OUT_OF_RANGE);
    CHECK(ota_flash_region_validate(NULL, staging()) ==
          OTA_FLASH_INVALID_ARGUMENT);
    return 1;
}

static int test_program_rejects_zero_to_one(void)
{
    uint8_t memory[512];
    uint8_t zeros[4] = {0};
    uint8_t ones[4] = {0xFFU, 0xFFU, 0xFFU, 0xFFU};
    OtaFlashSim sim;
    OtaFlashPort port;
    CHECK(make_port(&sim, memory, &port));
    CHECK(ota_flash_program(&port, staging(), 256U, zeros, 4U) ==
          OTA_FLASH_OK);
    CHECK(ota_flash_program(&port, staging(), 256U, ones, 4U) ==
          OTA_FLASH_IO_ERROR);
    CHECK(memory[256] == 0U);
    return 1;
}

static int test_injected_partial_program_failure(void)
{
    uint8_t memory[512];
    uint8_t data[8] = {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U};
    OtaFlashSim sim;
    OtaFlashPort port;
    CHECK(make_port(&sim, memory, &port));
    sim.fail_program_after = 3U;
    CHECK(ota_flash_program(&port, staging(), 256U, data, 8U) ==
          OTA_FLASH_IO_ERROR);
    CHECK(memcmp(memory + 256U, data, 3U) == 0);
    CHECK(memory[259] == 0xFFU && memory[263] == 0xFFU);
    sim.fail_program_after = SIZE_MAX;
    CHECK(ota_flash_program(&port, staging(), 256U, data, 8U) ==
          OTA_FLASH_OK);
    CHECK(ota_flash_verify(&port, staging(), 256U, data, 8U) ==
          OTA_FLASH_OK);
    return 1;
}

static int test_erase_read_and_verify_faults(void)
{
    uint8_t memory[512];
    uint8_t data[4] = {0};
    OtaFlashSim sim;
    OtaFlashPort port;
    CHECK(make_port(&sim, memory, &port));
    sim.fail_erase = 1;
    CHECK(ota_flash_erase(&port, staging(), 256U, 64U) == OTA_FLASH_IO_ERROR);
    sim.fail_erase = 0;
    sim.fail_read = 1;
    CHECK(ota_flash_read(&port, staging(), 256U, data, 4U) ==
          OTA_FLASH_IO_ERROR);
    CHECK(ota_flash_verify(&port, staging(), 256U, data, 4U) ==
          OTA_FLASH_IO_ERROR);
    sim.fail_read = 0;
    CHECK(ota_flash_verify(&port, staging(), 256U, data, 4U) ==
          OTA_FLASH_VERIFY_MISMATCH);
    return 1;
}

int main(void)
{
    RUN(test_program_read_verify_erase);
    RUN(test_region_bounds_and_alignment);
    RUN(test_invalid_geometry_and_arguments);
    RUN(test_program_rejects_zero_to_one);
    RUN(test_injected_partial_program_failure);
    RUN(test_erase_read_and_verify_faults);
    printf("OTA flash: %u passed, %u failed\n",
           run_count - fail_count, fail_count);
    return fail_count == 0U ? 0 : 1;
}
