#include "ota_update.h"

#include <string.h>

#define JOURNAL_MAGIC UINT32_C(0x4F54414A)
#define JOURNAL_SIZE 32U
#define IO_CHUNK 32U

static void put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static uint32_t get32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int disjoint(OtaFlashRegion a, OtaFlashRegion b)
{
    return (uint64_t)a.start + a.length <= b.start ||
           (uint64_t)b.start + b.length <= a.start;
}

static OtaUpdateResult fault(OtaUpdate *u, OtaUpdateResult result)
{
    u->state = OTA_UPDATE_RECOVERY_REQUIRED;
    u->last_error = result;
    return result;
}

static OtaUpdateResult flash_fault(OtaUpdate *u, OtaFlashResult result)
{
    u->last_flash_result = result;
    return fault(u, OTA_UPDATE_STORAGE_ERROR);
}

static int same_manifest(const OtaUpdate *u, const uint8_t *raw)
{
    return get32(raw + 12) == u->manifest.image_size &&
           get32(raw + 16) == u->manifest.image_crc32 &&
           get32(raw + 20) == u->manifest.firmware_version;
}

static int decode_record(const OtaUpdate *u, const uint8_t *raw,
                         uint32_t *sequence, OtaUpdateState *state)
{
    uint32_t crc;
    if (ota_image_crc32(raw, 24U, &crc) != OTA_IMAGE_OK ||
        get32(raw) != JOURNAL_MAGIC || get32(raw + 24) != crc ||
        !same_manifest(u, raw) || get32(raw + 4) == 0U ||
        get32(raw + 8) < (uint32_t)OTA_UPDATE_RECEIVING ||
        get32(raw + 8) > (uint32_t)OTA_UPDATE_APPLIED) {
        return 0;
    }
    *sequence = get32(raw + 4);
    *state = (OtaUpdateState)get32(raw + 8);
    return 1;
}

static OtaUpdateResult journal_write(OtaUpdate *u, OtaUpdateState state)
{
    uint8_t raw[JOURNAL_SIZE];
    uint32_t crc;
    uint32_t offset;
    OtaFlashResult fr;
    if (u->journal_sequence == UINT32_MAX) {
        return fault(u, OTA_UPDATE_JOURNAL_ERROR);
    }
    offset = u->journal.start +
             (u->journal_sequence % 2U) * u->flash->geometry.erase_unit;
    memset(raw, 0xFF, sizeof raw);
    put32(raw, JOURNAL_MAGIC);
    put32(raw + 4, u->journal_sequence + 1U);
    put32(raw + 8, (uint32_t)state);
    put32(raw + 12, u->manifest.image_size);
    put32(raw + 16, u->manifest.image_crc32);
    put32(raw + 20, u->manifest.firmware_version);
    (void)ota_image_crc32(raw, 24U, &crc);
    put32(raw + 24, crc);
    fr = ota_flash_erase(u->flash, u->journal, offset,
                         u->flash->geometry.erase_unit);
    if (fr == OTA_FLASH_OK) {
        fr = ota_flash_program(u->flash, u->journal, offset, raw, sizeof raw);
    }
    if (fr == OTA_FLASH_OK) {
        fr = ota_flash_verify(u->flash, u->journal, offset, raw, sizeof raw);
    }
    if (fr != OTA_FLASH_OK) {
        u->last_flash_result = fr;
        return fault(u, OTA_UPDATE_JOURNAL_ERROR);
    }
    ++u->journal_sequence;
    u->state = state;
    u->last_error = OTA_UPDATE_OK;
    return OTA_UPDATE_OK;
}

static OtaUpdateResult region_crc(OtaUpdate *u, OtaFlashRegion region,
                                   uint32_t *crc_out)
{
    uint8_t buffer[IO_CHUNK];
    uint32_t state = ota_crc32_begin();
    uint32_t pos = 0U;
    OtaFlashResult fr;
    while (pos < u->manifest.image_size) {
        uint32_t n = u->manifest.image_size - pos;
        if (n > sizeof buffer) {
            n = sizeof buffer;
        }
        fr = ota_flash_read(u->flash, region, region.start + pos, buffer, n);
        if (fr != OTA_FLASH_OK) {
            return flash_fault(u, fr);
        }
        (void)ota_crc32_update(&state, buffer, n);
        pos += n;
    }
    *crc_out = ota_crc32_finish(state);
    return OTA_UPDATE_OK;
}

static OtaUpdateResult check_crc(OtaUpdate *u, OtaFlashRegion region)
{
    uint32_t crc;
    OtaUpdateResult result = region_crc(u, region, &crc);
    if (result != OTA_UPDATE_OK) {
        return result;
    }
    if (crc != u->manifest.image_crc32) {
        u->state = OTA_UPDATE_INVALID_IMAGE;
        u->last_error = OTA_UPDATE_IMAGE_ERROR;
        return OTA_UPDATE_IMAGE_ERROR;
    }
    return OTA_UPDATE_OK;
}

OtaUpdateResult ota_update_init(OtaUpdate *u, OtaFlashPort *flash,
                                OtaFlashRegion active, OtaFlashRegion staging,
                                OtaFlashRegion journal,
                                const OtaImageManifest *manifest,
                                uint32_t minimum_version,
                                uint32_t max_block_size,
                                uint16_t block_crc_initial,
                                uint16_t block_crc_xorout)
{
    OtaFlashGeometry g;
    if (u == NULL || flash == NULL || manifest == NULL) {
        return OTA_UPDATE_INVALID_ARGUMENT;
    }
    g = flash->geometry;
    if (ota_flash_region_validate(flash, active) != OTA_FLASH_OK ||
        ota_flash_region_validate(flash, staging) != OTA_FLASH_OK ||
        ota_flash_region_validate(flash, journal) != OTA_FLASH_OK ||
        !disjoint(active, staging) || !disjoint(active, journal) ||
        !disjoint(staging, journal) ||
        g.program_unit > IO_CHUNK || JOURNAL_SIZE % g.program_unit != 0U ||
        g.erase_unit < JOURNAL_SIZE ||
        journal.length < 2U * (uint64_t)g.erase_unit ||
        max_block_size == 0U || max_block_size > OTA_BLOCK_MAX_SIZE ||
        ota_image_manifest_validate(manifest, active.length, minimum_version) != OTA_IMAGE_OK ||
        ota_image_manifest_validate(manifest, staging.length, minimum_version) != OTA_IMAGE_OK) {
        return OTA_UPDATE_INVALID_ARGUMENT;
    }
    *u = (OtaUpdate){0};
    u->flash = flash;
    u->active = active;
    u->staging = staging;
    u->journal = journal;
    u->manifest = *manifest;
    u->minimum_version = minimum_version;
    u->max_block_size = max_block_size;
    u->block_crc_initial = block_crc_initial;
    u->block_crc_xorout = block_crc_xorout;
    u->state = OTA_UPDATE_IDLE;
    return OTA_UPDATE_OK;
}

OtaUpdateResult ota_update_begin(OtaUpdate *u)
{
    OtaFlashResult fr;
    if (u == NULL || u->flash == NULL) {
        return OTA_UPDATE_INVALID_ARGUMENT;
    }
    u->recovery_can_apply = 0U;
    fr = ota_flash_erase(u->flash, u->staging, u->staging.start, u->staging.length);
    if (fr != OTA_FLASH_OK) {
        return flash_fault(u, fr);
    }
    fr = ota_flash_erase(u->flash, u->journal, u->journal.start, u->journal.length);
    if (fr != OTA_FLASH_OK) {
        return flash_fault(u, fr);
    }
    u->journal_sequence = 0U;
    u->last_block_result = ota_block_session_init(&u->blocks, u->staging.length,
        u->manifest.image_size, u->max_block_size, u->block_crc_initial,
        u->block_crc_xorout);
    if (u->last_block_result != OTA_BLOCK_OK) {
        return fault(u, OTA_UPDATE_INVALID_ARGUMENT);
    }
    return journal_write(u, OTA_UPDATE_RECEIVING);
}

OtaUpdateResult ota_update_receive(OtaUpdate *u, uint32_t offset,
                                    const uint8_t *data, size_t length,
                                    uint16_t expected_crc)
{
    OtaBlockSession candidate;
    OtaBlockResult br;
    uint32_t unit;
    size_t pos;
    if (u == NULL || u->flash == NULL) {
        return OTA_UPDATE_INVALID_ARGUMENT;
    }
    if (u->state != OTA_UPDATE_RECEIVING) {
        return OTA_UPDATE_INVALID_STATE;
    }
    candidate = u->blocks;
    br = ota_block_accept(&candidate, offset, data, length, expected_crc);
    u->last_block_result = br;
    if (br != OTA_BLOCK_OK && br != OTA_BLOCK_COMPLETE) {
        return OTA_UPDATE_BLOCK_REJECTED;
    }
    unit = u->flash->geometry.program_unit;
    if (offset % unit != 0U ||
        (length % unit != 0U && br != OTA_BLOCK_COMPLETE)) {
        u->last_block_result = OTA_BLOCK_INVALID_BLOCK_LENGTH;
        return OTA_UPDATE_BLOCK_REJECTED;
    }
    for (pos = 0U; pos < length; pos += unit) {
        uint8_t buffer[IO_CHUNK];
        size_t n = length - pos;
        OtaFlashResult fr;
        if (n > unit) {
            n = unit;
        }
        memset(buffer, 0xFF, unit);
        memcpy(buffer, data + pos, n);
        fr = ota_flash_program(u->flash, u->staging,
                               u->staging.start + offset + (uint32_t)pos,
                               buffer, unit);
        if (fr != OTA_FLASH_OK) {
            return flash_fault(u, fr);
        }
    }
    u->blocks = candidate;
    if (br == OTA_BLOCK_COMPLETE) {
        if (journal_write(u, OTA_UPDATE_RECEIVED) != OTA_UPDATE_OK) {
            return OTA_UPDATE_JOURNAL_ERROR;
        }
        return OTA_UPDATE_COMPLETE;
    }
    return OTA_UPDATE_OK;
}

OtaUpdateResult ota_update_verify_staging(OtaUpdate *u)
{
    OtaUpdateResult result;
    if (u == NULL || u->flash == NULL) {
        return OTA_UPDATE_INVALID_ARGUMENT;
    }
    if (u->state != OTA_UPDATE_RECEIVED) {
        return OTA_UPDATE_INVALID_STATE;
    }
    result = check_crc(u, u->staging);
    return result == OTA_UPDATE_OK ? journal_write(u, OTA_UPDATE_READY) : result;
}

OtaUpdateResult ota_update_apply(OtaUpdate *u)
{
    uint8_t buffer[IO_CHUNK];
    uint32_t unit;
    uint32_t pos;
    OtaFlashResult fr;
    OtaUpdateResult result;
    if (u == NULL || u->flash == NULL) {
        return OTA_UPDATE_INVALID_ARGUMENT;
    }
    if (u->state != OTA_UPDATE_READY) {
        return OTA_UPDATE_INVALID_STATE;
    }
    u->recovery_can_apply = 1U;
    result = check_crc(u, u->staging);
    if (result != OTA_UPDATE_OK) {
        return result;
    }
    result = journal_write(u, OTA_UPDATE_APPLYING);
    if (result != OTA_UPDATE_OK) {
        return result;
    }
    fr = ota_flash_erase(u->flash, u->active, u->active.start, u->active.length);
    if (fr != OTA_FLASH_OK) {
        return flash_fault(u, fr);
    }
    unit = u->flash->geometry.program_unit;
    for (pos = 0U; pos < u->manifest.image_size; pos += unit) {
        uint32_t n = u->manifest.image_size - pos;
        if (n > unit) {
            n = unit;
        }
        memset(buffer, 0xFF, unit);
        fr = ota_flash_read(u->flash, u->staging, u->staging.start + pos,
                            buffer, n);
        if (fr != OTA_FLASH_OK) {
            return flash_fault(u, fr);
        }
        fr = ota_flash_program(u->flash, u->active, u->active.start + pos,
                               buffer, unit);
        if (fr != OTA_FLASH_OK) {
            return flash_fault(u, fr);
        }
    }
    result = check_crc(u, u->active);
    if (result != OTA_UPDATE_OK) {
        return result;
    }
    return journal_write(u, OTA_UPDATE_APPLIED);
}

OtaUpdateResult ota_update_resume(OtaUpdate *u)
{
    uint8_t raw[JOURNAL_SIZE];
    uint32_t best_sequence = 0U;
    OtaUpdateState best_state = OTA_UPDATE_IDLE;
    unsigned int i;
    if (u == NULL || u->flash == NULL) {
        return OTA_UPDATE_INVALID_ARGUMENT;
    }
    u->state = OTA_UPDATE_RECOVERY_REQUIRED;
    u->recovery_can_apply = 0U;
    for (i = 0U; i < 2U; ++i) {
        uint32_t sequence;
        OtaUpdateState state;
        OtaFlashResult fr = ota_flash_read(u->flash, u->journal,
            u->journal.start + i * u->flash->geometry.erase_unit,
            raw, sizeof raw);
        if (fr != OTA_FLASH_OK) {
            return flash_fault(u, fr);
        }
        if (decode_record(u, raw, &sequence, &state) && sequence > best_sequence) {
            best_sequence = sequence;
            best_state = state;
        }
    }
    u->journal_sequence = best_sequence;
    if (best_sequence == 0U) {
        return fault(u, OTA_UPDATE_RECOVERY_NEEDED);
    }
    if (best_state == OTA_UPDATE_APPLIED) {
        OtaUpdateResult result = check_crc(u, u->active);
        if (result != OTA_UPDATE_OK) {
            return result;
        }
        u->state = OTA_UPDATE_APPLIED;
        u->last_error = OTA_UPDATE_OK;
        return OTA_UPDATE_OK;
    }
    if (best_state == OTA_UPDATE_READY || best_state == OTA_UPDATE_APPLYING) {
        OtaUpdateResult result = check_crc(u, u->staging);
        if (result != OTA_UPDATE_OK) {
            return result;
        }
        u->recovery_can_apply = 1U;
    }
    return fault(u, OTA_UPDATE_RECOVERY_NEEDED);
}

OtaUpdateResult ota_update_retry_apply(OtaUpdate *u)
{
    OtaUpdateResult result;
    if (u == NULL || u->flash == NULL) {
        return OTA_UPDATE_INVALID_ARGUMENT;
    }
    if (u->state != OTA_UPDATE_RECOVERY_REQUIRED ||
        u->recovery_can_apply != 1U) {
        return OTA_UPDATE_INVALID_STATE;
    }
    result = check_crc(u, u->staging);
    if (result != OTA_UPDATE_OK) {
        return result;
    }
    result = journal_write(u, OTA_UPDATE_READY);
    return result == OTA_UPDATE_OK ? ota_update_apply(u) : result;
}

int ota_update_boot_eligible(OtaUpdate *u)
{
    return u != NULL && u->flash != NULL &&
           u->state == OTA_UPDATE_APPLIED &&
           check_crc(u, u->active) == OTA_UPDATE_OK;
}
