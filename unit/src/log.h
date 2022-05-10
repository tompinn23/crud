#pragma once

#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

enum un_log_mode {
    LOG_ERR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG,
    LOG_MODE_LAST
};

#ifdef __GNUC__
#define _UN_ATTRIB_PRINTF(start, end) __attribute__((format(printf, start, end)))
#else
#define _UN_ATTRIB_PRINTF(start, end)
#endif

void _un_log(enum un_log_mode mode, const char* fmt, ...) _UN_ATTRIB_PRINTF(2 ,3);
void _un_vlog(enum un_log_mode mode, const char* fmt, va_list args) _UN_ATTRIB_PRINTF(2, 0);

#ifdef UN_LOG_SRC_DIR
#define UN_FILENAME ((const char*)__FILE__ + sizeof(UN_LOG_SRC_DIR) - 1)
#else
#define UN_FILENAME __FILE__
#endif

#define un_log(mode, fmt, ...) _un_log(mode, "[%s:%d] " fmt, UN_FILENAME, __LINE__, ##__VA_ARGS__)
#define un_vlog(mode, fmt, args) _un_vlog("[%s:%d] " fmt, UN_FILENAME, __LINE__, args)
#define un_log_errno(mode, fmt, ...) _un_log(mode, "[%s:%d] " fmt ": %s", UN_FILENAME, __LINE__, ##__VA_ARGS__, strerror(errno))
