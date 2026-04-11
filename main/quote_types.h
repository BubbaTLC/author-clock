#ifndef QUOTE_TYPES_H
#define QUOTE_TYPES_H

/**
 * @brief Shared quote data structure used by quote_reader and display_mgr.
 */
typedef struct {
    char quote[1024];
    char title[128];
    char author[128];
    char timestring[128];
} quote_result_t;

#endif // QUOTE_TYPES_H
