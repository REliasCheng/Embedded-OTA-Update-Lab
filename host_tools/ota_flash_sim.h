#ifndef OTA_FLASH_SIM_H
#define OTA_FLASH_SIM_H

#include "ota_flash.h"

typedef struct {
    uint8_t *storage;
    uint32_t storage_size;
    int fail_read;
    int fail_erase;
    size_t fail_program_after; /* SIZE_MAX disables fault injection. */
} OtaFlashSim;

/* The caller supplies storage; initialization models erased Flash (0xFF). */
OtaFlashResult ota_flash_sim_init(OtaFlashSim *sim, uint8_t *storage,
                                   size_t storage_size,
                                   OtaFlashGeometry geometry,
                                   OtaFlashPort *port_out);

#endif
