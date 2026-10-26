#include "ota_flash_sim.h"

#include <string.h>

static int sim_range(const OtaFlashSim *sim, uint32_t offset, uint32_t length)
{
    return offset <= sim->storage_size &&
           length <= sim->storage_size - offset;
}

static int sim_read(void *context, uint32_t offset, uint8_t *out,
                    uint32_t length)
{
    OtaFlashSim *sim = context;
    if (sim->fail_read || !sim_range(sim, offset, length)) {
        return 0;
    }
    memcpy(out, sim->storage + offset, length);
    return 1;
}

static int sim_erase(void *context, uint32_t offset, uint32_t length)
{
    OtaFlashSim *sim = context;
    if (sim->fail_erase || !sim_range(sim, offset, length)) {
        return 0;
    }
    memset(sim->storage + offset, 0xFF, length);
    return 1;
}

static int sim_program(void *context, uint32_t offset, const uint8_t *data,
                       uint32_t length)
{
    OtaFlashSim *sim = context;
    uint32_t i;
    if (!sim_range(sim, offset, length)) {
        return 0;
    }
    for (i = 0U; i < length; ++i) {
        uint8_t old_value = sim->storage[offset + i];
        if ((size_t)i == sim->fail_program_after ||
            (uint8_t)(old_value & data[i]) != data[i]) {
            return 0;
        }
        sim->storage[offset + i] = data[i];
    }
    return 1;
}

OtaFlashResult ota_flash_sim_init(OtaFlashSim *sim, uint8_t *storage,
                                   size_t storage_size,
                                   OtaFlashGeometry geometry,
                                   OtaFlashPort *port_out)
{
    OtaFlashPort candidate;
    if (sim == NULL || storage == NULL || port_out == NULL ||
        storage_size == 0U || (uint64_t)storage_size < geometry.total_size) {
        return OTA_FLASH_INVALID_ARGUMENT;
    }
    candidate = (OtaFlashPort){sim, sim_read, sim_erase, sim_program, geometry};
    if (geometry.total_size == 0U || geometry.erase_unit == 0U ||
        geometry.program_unit == 0U ||
        geometry.erase_unit % geometry.program_unit != 0U ||
        geometry.total_size % geometry.erase_unit != 0U) {
        return OTA_FLASH_INVALID_GEOMETRY;
    }
    *sim = (OtaFlashSim){storage, geometry.total_size, 0, 0, SIZE_MAX};
    memset(storage, 0xFF, geometry.total_size);
    *port_out = candidate;
    return OTA_FLASH_OK;
}
