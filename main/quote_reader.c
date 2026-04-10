#include "quote_reader.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "esp_log.h"
#include "esp_littlefs.h"
#include "esp_random.h"

static const char *TAG = "QUOTE_RDR";

// ─── Binary format constants (must match gen_quotes_bin.py) ──────────────────

#define BQDB_MAGIC "BQDB"
#define BQDB_VERSION 0x01
#define HEADER_SIZE 7                                        // magic(4) + version(1) + count(2)
#define INDEX_ENTRIES 1440                                   // one per minute of the day
#define INDEX_ENTRY_SIZE 5                                   // offset(uint32_t) + count(uint8_t)
#define INDEX_TOTAL_BYTES (INDEX_ENTRIES * INDEX_ENTRY_SIZE) // 7200
#define RECORDS_START (HEADER_SIZE + INDEX_TOTAL_BYTES)      // 7207

#define MOUNT_POINT "/quotes"
#define PARTITION_LABEL "storage"
#define QUOTES_PATH MOUNT_POINT "/quotes.bin"

// ─── Module state ─────────────────────────────────────────────────────────────

// Time index cached in RAM: 1440 entries × 5 bytes = 7200 bytes.
// Each entry: [0..3] uint32_t offset (LE), [4] uint8_t count.
static uint8_t s_index[INDEX_TOTAL_BYTES];
static FILE *s_file = NULL;
static bool s_mounted = false;

// ─── Internal helpers ─────────────────────────────────────────────────────────

// Read exactly n bytes or return false.
static bool fread_exact(void *buf, size_t n, FILE *f) {
    return fread(buf, 1, n, f) == n;
}

// Skip n bytes forward in the file.
static bool fskip(FILE *f, long n) {
    return fseek(f, n, SEEK_CUR) == 0;
}

// Decode the 5-byte index entry for minute `idx` into offset/count.
static inline void index_decode(int idx, uint32_t *offset, uint8_t *count) {
    const uint8_t *p = &s_index[idx * INDEX_ENTRY_SIZE];
    // Little-endian uint32_t
    *offset =
        (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    *count = p[4];
}

// ─── Public API ───────────────────────────────────────────────────────────────

esp_err_t quote_reader_init(void) {
    if (s_file) {
        ESP_LOGW(TAG, "Already initialized");
        return ESP_OK;
    }

    // Mount LittleFS partition "storage" at /quotes
    esp_vfs_littlefs_conf_t conf = {
        .base_path = MOUNT_POINT,
        .partition_label = PARTITION_LABEL,
        .format_if_mount_failed = false,
        .dont_mount = false,
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount LittleFS (%s): %s", PARTITION_LABEL, esp_err_to_name(ret));
        return ret;
    }
    s_mounted = true;

    size_t total = 0, used = 0;
    esp_littlefs_info(PARTITION_LABEL, &total, &used);
    ESP_LOGI(TAG, "LittleFS mounted: %u/%u bytes used", (unsigned)used, (unsigned)total);

    // Open the quotes binary
    s_file = fopen(QUOTES_PATH, "rb");
    if (!s_file) {
        ESP_LOGE(TAG, "Cannot open %s", QUOTES_PATH);
        esp_vfs_littlefs_unregister(PARTITION_LABEL);
        s_mounted = false;
        return ESP_FAIL;
    }

    // Validate header: magic + version
    char magic[4];
    if (!fread_exact(magic, 4, s_file) || memcmp(magic, BQDB_MAGIC, 4) != 0) {
        ESP_LOGE(TAG, "Bad magic in %s", QUOTES_PATH);
        fclose(s_file);
        s_file = NULL;
        esp_vfs_littlefs_unregister(PARTITION_LABEL);
        s_mounted = false;
        return ESP_ERR_INVALID_VERSION;
    }

    uint8_t version;
    uint16_t count_le;
    if (!fread_exact(&version, 1, s_file) || version != BQDB_VERSION) {
        ESP_LOGE(TAG, "Unsupported version 0x%02x (expected 0x%02x)", version, BQDB_VERSION);
        fclose(s_file);
        s_file = NULL;
        esp_vfs_littlefs_unregister(PARTITION_LABEL);
        s_mounted = false;
        return ESP_ERR_INVALID_VERSION;
    }
    fread_exact(&count_le, 2, s_file); // total record count (informational)
    uint16_t total_records = count_le; // already LE on ESP32

    // Cache the full time index into RAM
    if (!fread_exact(s_index, INDEX_TOTAL_BYTES, s_file)) {
        ESP_LOGE(TAG, "Failed to read time index");
        fclose(s_file);
        s_file = NULL;
        esp_vfs_littlefs_unregister(PARTITION_LABEL);
        s_mounted = false;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "quote_reader ready: %u records, index cached (%d bytes)", total_records,
             INDEX_TOTAL_BYTES);
    return ESP_OK;
}

esp_err_t quote_read(uint8_t hour, uint8_t minute, quote_result_t *out) {
    if (!out)
        return ESP_ERR_INVALID_ARG;
    if (!s_file)
        return ESP_ERR_INVALID_STATE;

    if (hour > 23 || minute > 59)
        return ESP_ERR_INVALID_ARG;

    int idx = (int)hour * 60 + (int)minute;
    uint32_t offset;
    uint8_t count;
    index_decode(idx, &offset, &count);

    if (count == 0) {
        ESP_LOGW(TAG, "No quotes for %02u:%02u", hour, minute);
        return ESP_ERR_NOT_FOUND;
    }

    // Pick a random record index
    uint32_t pick = esp_random() % (uint32_t)count;

    // Seek to the first record for this time slot
    if (fseek(s_file, (long)offset, SEEK_SET) != 0) {
        ESP_LOGE(TAG, "Seek to offset %lu failed", (unsigned long)offset);
        return ESP_FAIL;
    }

    // Skip 'pick' records then read the chosen one
    for (uint32_t i = 0; i < pick; i++) {
        uint16_t qlen;
        uint8_t tlen, alen;
        if (!fread_exact(&qlen, 2, s_file) || !fread_exact(&tlen, 1, s_file) ||
            !fread_exact(&alen, 1, s_file)) {
            ESP_LOGE(TAG, "Failed reading record header (skip %lu)", (unsigned long)i);
            return ESP_FAIL;
        }
        if (!fskip(s_file, (long)qlen + tlen + alen)) {
            ESP_LOGE(TAG, "Failed skipping record body");
            return ESP_FAIL;
        }
    }

    // Read the chosen record
    uint16_t qlen;
    uint8_t tlen, alen;
    if (!fread_exact(&qlen, 2, s_file) || !fread_exact(&tlen, 1, s_file) ||
        !fread_exact(&alen, 1, s_file)) {
        ESP_LOGE(TAG, "Failed reading chosen record header");
        return ESP_FAIL;
    }

    // Bounds check against out struct sizes
    size_t qcopy = qlen < sizeof(out->quote) - 1 ? qlen : sizeof(out->quote) - 1;
    size_t tcopy = tlen < sizeof(out->title) - 1 ? tlen : sizeof(out->title) - 1;
    size_t acopy = alen < sizeof(out->author) - 1 ? alen : sizeof(out->author) - 1;

    if (fread(out->quote, 1, qcopy, s_file) != qcopy)
        return ESP_FAIL;
    out->quote[qcopy] = '\0';
    if (qcopy < qlen)
        fskip(s_file, (long)(qlen - qcopy)); // skip remainder

    if (fread(out->title, 1, tcopy, s_file) != tcopy)
        return ESP_FAIL;
    out->title[tcopy] = '\0';
    if (tcopy < tlen)
        fskip(s_file, (long)(tlen - tcopy));

    if (fread(out->author, 1, acopy, s_file) != acopy)
        return ESP_FAIL;
    out->author[acopy] = '\0';

    ESP_LOGD(TAG, "quote_read(%02u:%02u) pick=%lu/%u: \"%s\" — %s", hour, minute,
             (unsigned long)pick, count, out->title, out->author);
    return ESP_OK;
}

void quote_reader_deinit(void) {
    if (s_file) {
        fclose(s_file);
        s_file = NULL;
    }
    if (s_mounted) {
        esp_vfs_littlefs_unregister(PARTITION_LABEL);
        s_mounted = false;
    }
}
