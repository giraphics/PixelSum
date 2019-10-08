#pragma once
#include <stdio.h>

#include <time.h>
#include <string.h>

static inline char* currentTime();

#define _FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

#define NO_LOG          0x00
#define ERROR_LEVEL     0x01
#define INFO_LEVEL      0x02
#define DEBUG_LEVEL     0x03
#define PRINT_LOG(format, ...)      fprintf(stderr, format, __VA_ARGS__)
#define LOG_FMT             "%s | %-7s | %-15s | %s:%d | "
#define LOG_ARGS(LOG_TAG)   currentTime(), LOG_TAG, _FILE, __FUNCTION__, __LINE__
#define NEWLINE     "\n"

#define ERROR_STR   "ERROR"
#define INFO_STR    "INFO"
#define DEBUG_STR   "DEBUG"

#define LOG_DEBUG(msg, args...)     PRINT_LOG(LOG_FMT msg NEWLINE, LOG_ARGS(DEBUG_STR), ## args)
#define LOG_ERROR(msg, args...)     PRINT_LOG(LOG_FMT msg NEWLINE, LOG_ARGS(ERROR_STR), ## args)
#define LOG_IF_ERROR(condition, msg, args...) if (condition) PRINT_LOG(LOG_FMT msg NEWLINE, LOG_ARGS(ERROR_STR), ## args)
#define LOG_INFO(msg, args...)      PRINT_LOG(LOG_FMT msg NEWLINE, LOG_ARGS(INFO_STR), ## args)

static inline char* currentTime()
{
    static char buffer[64];
    time_t lTime;
    struct tm* timeInfo;

    time(&lTime);
    timeInfo = localtime(&lTime);

    strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeInfo);

    return buffer;
}
