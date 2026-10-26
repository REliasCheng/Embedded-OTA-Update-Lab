#include "ota_flash.h"

#include <string.h>

static OtaFlashResult validate_port(const OtaFlashPort *port)
{
    OtaFlashGeometry g;
    if (port == NULL || port->context == NULL || port->read == NULL ||
        port->erase == NULL || port->program == NULL) {
        return OTA_FLASH_INVALID_ARGUMENT;
    }
    g = port->geometry;
    if (g.total_size == 0U || g.erase_unit == 0U || g.program_unit == 0U ||
        g.erase_unit % g.program_unit != 0U ||
        g.total_size % g.erase_unit != 0U) {
        return OTA_FLASH_INVALID_GEOMETRY;
    }
    return OTA_FLASH_OK;
}

OtaFlashResult ota_flash_region_validate(const OtaFlashPort *port,
                                          OtaFlashRegion region)
{
    OtaFlashResult result = validate_port(port);
    if (result != OTA_FLASH_OK) {
        return result;
    }
    if (region.length == 0U || region.start >= port->geometry.total_size ||
        region.length > port->geometry.total_size - region.start) {
        return OTA_FLASH_OUT_OF_RANGE;
    }
    if (region.start % port->geometry.erase_unit != 0U ||
        region.length % port->geometry.erase_unit != 0U) {
        return OTA_FLASH_MISALIGNED;
    }
    return OTA_FLASH_OK;
}

static OtaFlashResult validate_access(const OtaFlashPort *port,
                                      OtaFlashRegion region,
                                      uint32_t offset, size_t length)
{
    OtaFlashResult result = ota_flash_region_validate(port, region);
    if (result != OTA_FLASH_OK) {
        return result;
    }
    if (length == 0U) {
        return OTA_FLASH_INVALID_ARGUMENT;
    }
    if (offset < region.start || offset - region.start >= region.length ||
        (uint64_t)length > (uint64_t)(region.length - (offset - region.start))) {
        return OTA_FLASH_OUT_OF_RANGE;
    }
    return OTA_FLASH_OK;
}

OtaFlashResult ota_flash_read(const OtaFlashPort *port, OtaFlashRegion region,
                               uint32_t offset, uint8_t *out, size_t length)
{
    OtaFlashResult result = validate_access(port, region, offset, length);
    if (result != OTA_FLASH_OK) {
        return result;
    }
    if (out == NULL) {
        return OTA_FLASH_INVALID_ARGUMENT;
    }
    return port->read(port->context, offset, out, (uint32_t)length)
               ? OTA_FLASH_OK : OTA_FLASH_IO_ERROR;
}

OtaFlashResult ota_flash_erase(const OtaFlashPort *port, OtaFlashRegion region,
                                uint32_t offset, size_t length)
{
    OtaFlashResult result = validate_access(port, region, offset, length);
    if (result != OTA_FLASH_OK) {
        return result;
    }
    if (offset % port->geometry.erase_unit != 0U ||
        length % port->geometry.erase_unit != 0U) {
        return OTA_FLASH_MISALIGNED;
    }
    return port->erase(port->context, offset, (uint32_t)length)
               ? OTA_FLASH_OK : OTA_FLASH_IO_ERROR;
}

OtaFlashResult ota_flash_program(const OtaFlashPort *port, OtaFlashRegion region,
                                  uint32_t offset, const uint8_t *data,
                                  size_t length)
{
    OtaFlashResult result = validate_access(port, region, offset, length);
    if (result != OTA_FLASH_OK) {
        return result;
    }
    if (data == NULL) {
        return OTA_FLASH_INVALID_ARGUMENT;
    }
    if (offset % port->geometry.program_unit != 0U ||
        length % port->geometry.program_unit != 0U) {
        return OTA_FLASH_MISALIGNED;
    }
    return port->program(port->context, offset, data, (uint32_t)length)
               ? OTA_FLASH_OK : OTA_FLASH_IO_ERROR;
}

OtaFlashResult ota_flash_verify(const OtaFlashPort *port, OtaFlashRegion region,
                                 uint32_t offset, const uint8_t *expected,
                                 size_t length)
{
    uint8_t buffer[64];
    size_t consumed = 0U;
    OtaFlashResult result = validate_access(port, region, offset, length);
    if (result != OTA_FLASH_OK) {
        return result;
    }
    if (expected == NULL) {
        return OTA_FLASH_INVALID_ARGUMENT;
    }
    while (consumed < length) {
        size_t n = length - consumed;
        if (n > sizeof buffer) {
            n = sizeof buffer;
        }
        result = ota_flash_read(port, region, offset + (uint32_t)consumed,
                                buffer, n);
        if (result != OTA_FLASH_OK) {
            return result;
        }
        if (memcmp(buffer, expected + consumed, n) != 0) {
            return OTA_FLASH_VERIFY_MISMATCH;
        }
        consumed += n;
    }
    return OTA_FLASH_OK;
}
