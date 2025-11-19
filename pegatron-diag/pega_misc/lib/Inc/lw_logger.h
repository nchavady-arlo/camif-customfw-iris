#ifndef _LOGGER_LIGHT_WEIGHT_H_
#define _LOGGER_LIGHT_WEIGHT_H_

#include <syslog.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

typedef enum lwlog_levels {
    LWLL_MIN = 0,
    LWLL_EMERG = LWLL_MIN,
    LWLL_ALERT,
    LWLL_CRIT,
    LWLL_ERROR,
    LWLL_WARNING,
    LWLL_NOTICE,
    LWLL_INFO,
    LWLL_DEBUG,
    LWLL_TRACE,
    LWLL_MAX
} lwlog_levels_t;

// #define TEST_CONSOLE
#define LOG_LEVEL_2_CONSOLE LWLL_NOTICE
#ifdef TEST_CONSOLE
#define log_2_console(leve, x) x;
#elif defined(LOG_LEVEL_2_CONSOLE)
#define log_2_console(level, x)         \
    if (level <= LOG_LEVEL_2_CONSOLE) { \
        x;                              \
    }
#else
#define log_2_console(level, x)
#endif

#define _LOG(level, fmt, args...)                                                                                                   \
    do {                                                                                                                            \
        syslog(LWLL_##level, "[%s:%d]%s():" fmt, __FILENAME__, __LINE__, __func__, ##args);                                         \
        log_2_console(LWLL_##level, fprintf(stdout, "[" #level "][%s:%d]%s():" fmt "\n", __FILENAME__, __LINE__, __func__, ##args)) \
    } while (0)

#define _LOG_TRACE(fmt, args...)   _LOG(TRACE, fmt, ##args)
#define _LOG_DEBUG(fmt, args...)   _LOG(DEBUG, fmt, ##args)
#define _LOG_INFO(fmt, args...)    _LOG(INFO, fmt, ##args)
#define _LOG_NOTICE(fmt, args...)  _LOG(NOTICE, fmt, ##args)
#define _LOG_WARNING(fmt, args...) _LOG(WARNING, fmt, ##args)
#define _LOG_ERROR(fmt, args...)   _LOG(ERROR, fmt, ##args)
#define _LOG_CRIT(fmt, args...)    _LOG(CRIT, fmt, ##args)
#define _LOG_ALERT(fmt, args...)   _LOG(ALERT, fmt, ##args)

#endif  // !_LOGGER_LIGHT_WEIGHT_H_
