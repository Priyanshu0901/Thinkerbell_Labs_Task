/*
 * debug_logger.c
 *
 *  Created on: Feb 1, 2025
 *  Last Modified on: Feb 16, 2025
 *      Author: Priyanshu Roy
 */

#include "stdio.h"
#include <string.h>

#include "main.h"
#include "debug_logger.h"

#define USE_FULL_ASSERT
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* INC_SRC_LIB_UTILS_DEBUG_LOGGER_C_ */

extern UART_HandleTypeDef huart2;

PUTCHAR_PROTOTYPE {
	HAL_UART_Transmit(&huart2, (uint8_t*) &ch, 1, 0xFFFF);
	return ch;
}

#define RESET_COLOR   "\033[0m"
#define COLOR_DEBUG   "\033[36m"  // Cyan
#define COLOR_INFO    "\033[32m"  // Green
#define COLOR_WARN    "\033[33m"  // Yellow
#define COLOR_ERROR   "\033[31m"  // Red
#define COLOR_FATAL   "\033[35m"  // Magenta

// Convert log level to string
const char* log_level_to_str(log_level_t level) {
	switch (level) {
	case LOG_DEBUG:
		return "DEBUG";
	case LOG_INFO:
		return "INFO";
	case LOG_WARN:
		return "WARN";
	case LOG_ERROR:
		return "ERROR";
	case LOG_FATAL:
		return "FATAL";
	default:
		return "UNKNOWN";
	}
}

const char* log_level_to_color(log_level_t level) {
	switch (level) {
	case LOG_DEBUG:
		return COLOR_DEBUG;
	case LOG_INFO:
		return COLOR_INFO;
	case LOG_WARN:
		return COLOR_WARN;
	case LOG_ERROR:
		return COLOR_ERROR;
	case LOG_FATAL:
		return COLOR_FATAL;
	default:
		return RESET_COLOR;
	}
}

void get_current_time(char *buffer, size_t size) {
	if (buffer == NULL || size < 8)
		return;  // Safety check

	uint32_t time_ms = HAL_GetTick();
	uint32_t seconds = time_ms / 1000;
	uint32_t millis = time_ms % 1000;

	snprintf(buffer, size - 1, "%lu.%03lu", seconds, millis);
	buffer[size - 1] = '\0';  // Null-terminate for safety
}

void log_message(const char *tag, log_level_t severity,
		const char *message_format, ...) {
	char log_buffer[256];  // Increased buffer size
	char time_buffer[20];

	get_current_time(time_buffer, sizeof(time_buffer));

	// Get the corresponding color
	const char *color = log_level_to_color(severity);

	// Print timestamp, tag, and severity with color
	int offset = snprintf(log_buffer, sizeof(log_buffer) - 2,
			"%s(%s)\t%s\t%s\t:\t", color, time_buffer,
			log_level_to_str(severity), tag);

	// Ensure snprintf did not exceed buffer
	if (offset < 0 || offset >= (int) sizeof(log_buffer) - 2) {
		return; // Prevent buffer overflow
	}

	// Process the variadic arguments
	va_list args;
	va_start(args, message_format);
	int remaining = sizeof(log_buffer) - offset - 2; // Leave space for \r\n
	vsnprintf(log_buffer + offset, remaining, message_format, args);
	va_end(args);

	// Ensure proper line termination
	strcat(log_buffer, "\r\n");

	// Print final log message
	printf("%s%s", log_buffer, RESET_COLOR);
	fflush(stdout); // Ensure immediate output
}

