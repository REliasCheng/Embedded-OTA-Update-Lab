#ifndef OTA_FLASH_H
#define OTA_FLASH_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t total_size;
    uint32_t erase_unit;
    uint32_t program_unit;
} OtaFlashGeometry;

typedef struct {
    uint32_t start;
    uint32_t length;
} OtaFlashRegion;

typedef struct {
    void *context;
    int (*read)(void *context, uint32_t offset, uint8_t *out, uint32_t length);
    int (*erase)(void *context, uint32_t offset, uint32_t length);
    int (*program)(void *context, uint32_t offset, const uint8_t *data,
                   uint32_t length);
    OtaFlashGeometry geometry;
} OtaFlashPort;

typedef enum {
    OTA_FLASH_OK = 0,
    OTA_FLASH_INVALID_ARGUMENT,
    OTA_FLASH_INVALID_GEOMETRY,
    OTA_FLASH_OUT_OF_RANGE,
    OTA_FLASH_MISALIGNED,
    OTA_FLASH_IO_ERROR,
    OTA_FLASH_VERIFY_MISMATCH
} OtaFlashResult;

OtaFlashResult ota_flash_region_validate(const OtaFlashPort *port,
                                          OtaFlashRegion region);
OtaFlashResult ota_flash_read(const OtaFlashPort *port, OtaFlashRegion region,
                               uint32_t offset, uint8_t *out, size_t length);
OtaFlashResult ota_flash_erase(const OtaFlashPort *port, OtaFlashRegion region,
                                uint32_t offset, size_t length);
OtaFlashResult ota_flash_program(const OtaFlashPort *port, OtaFlashRegion region,
                                  uint32_t offset, const uint8_t *data,
                                  size_t length);
OtaFlashResult ota_flash_verify(const OtaFlashPort *port, OtaFlashRegion region,
                                 uint32_t offset, const uint8_t *expected,
                                 size_t length);

#endif
