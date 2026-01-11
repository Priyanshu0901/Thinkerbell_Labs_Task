/*
 * debug_logger.h
 *
 *  Created on: Feb 1, 2025
 *  Last Modified on: Feb 16, 2025
 *      Author: Priyanshu Roy
 */

#ifndef INC_INC_LIB_UTILS_LOGGER_SERVICE_DEBUG_LOGGER_H_
#define INC_INC_LIB_UTILS_LOGGER_SERVICE_DEBUG_LOGGER_H_

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

// Define log levels
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} log_level_t;

// Variadic logging function
void log_message(const char*, log_level_t, const char*, ...);

#endif /* INC_INC_LIB_UTILS_LOGGER_SERVICE_DEBUG_LOGGER_H_ */
