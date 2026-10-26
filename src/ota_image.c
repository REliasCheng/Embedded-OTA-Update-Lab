#include "ota_image.h"

OtaImageResult ota_image_manifest_validate(const OtaImageManifest *manifest,
                                            uint32_t capacity,
                                            uint32_t minimum_version)
{
    if (manifest == NULL || capacity == 0U) {
        return OTA_IMAGE_INVALID_ARGUMENT;
    }
    if (manifest->magic != OTA_IMAGE_MAGIC ||
        manifest->format_version != OTA_IMAGE_FORMAT_VERSION) {
        return OTA_IMAGE_UNSUPPORTED_FORMAT;
    }
    if (manifest->image_size == 0U || manifest->image_size > capacity) {
        return OTA_IMAGE_INVALID_SIZE;
    }
    if (manifest->firmware_version < minimum_version) {
        return OTA_IMAGE_VERSION_REJECTED;
    }
    return OTA_IMAGE_OK;
}

uint32_t ota_crc32_begin(void)
{
    return UINT32_C(0xFFFFFFFF);
}

OtaImageResult ota_crc32_update(uint32_t *state, const uint8_t *data,
                                size_t length)
{
    uint32_t crc;
    size_t index;
    unsigned int bit;
    if (state == NULL || (data == NULL && length != 0U)) {
        return OTA_IMAGE_INVALID_ARGUMENT;
    }
    crc = *state;
    for (index = 0U; index < length; ++index) {
        crc ^= (uint32_t)data[index];
        for (bit = 0U; bit < 8U; ++bit) {
            crc = (crc & 1U) != 0U
                      ? (crc >> 1U) ^ UINT32_C(0xEDB88320)
                      : crc >> 1U;
        }
    }
    *state = crc;
    return OTA_IMAGE_OK;
}

uint32_t ota_crc32_finish(uint32_t state)
{
    return state ^ UINT32_C(0xFFFFFFFF);
}

OtaImageResult ota_image_crc32(const uint8_t *data, size_t length,
                               uint32_t *crc_out)
{
    uint32_t state;
    OtaImageResult result;
    if (crc_out == NULL || data == NULL) {
        return OTA_IMAGE_INVALID_ARGUMENT;
    }
    if (length == 0U) {
        return OTA_IMAGE_INVALID_SIZE;
    }
    state = ota_crc32_begin();
    result = ota_crc32_update(&state, data, length);
    if (result != OTA_IMAGE_OK) {
        return result;
    }
    *crc_out = ota_crc32_finish(state);
    return OTA_IMAGE_OK;
}

OtaImageResult ota_image_verify_buffer(const OtaImageManifest *manifest,
                                        uint32_t capacity,
                                        uint32_t minimum_version,
                                        const uint8_t *data, size_t length)
{
    uint32_t actual_crc;
    OtaImageResult result = ota_image_manifest_validate(manifest, capacity,
                                                         minimum_version);
    if (result != OTA_IMAGE_OK) {
        return result;
    }
    if ((uint64_t)length != (uint64_t)manifest->image_size) {
        return OTA_IMAGE_LENGTH_MISMATCH;
    }
    if (data == NULL) {
        return OTA_IMAGE_INVALID_ARGUMENT;
    }
    result = ota_image_crc32(data, length, &actual_crc);
    if (result != OTA_IMAGE_OK) {
        return result;
    }
    return actual_crc == manifest->image_crc32 ? OTA_IMAGE_OK
                                                : OTA_IMAGE_CRC_MISMATCH;
}
