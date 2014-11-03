/**
# -*- coding:UTF-8 -*-
*/

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "define.h"
#include "fs_utility.h"

#ifdef _WIN32
#ifndef localtime_r
#define localtime_r(unix_time, tms) localtime_s(tms, unix_time)
#endif
#ifndef snprintf
#define snprintf(buffer, buffer_size, fmt, ...) _snprintf(buffer, buffer_size, fmt, ##__VA_ARGS__)
#endif
#endif

#ifndef __FUNCSIG__
#ifdef  __PRETTY_FUNCTION__
#define __FUNCSIG__ __PRETTY_FUNCTION__
#elif defined __FUNCTION__
#define __FUNCSIG__ __FUNCTION__
#else
#define __FUNCSIG__ __func__
#endif
#endif

#define LOGGER_LINE_SIZE 4096

#ifdef __cplusplus
extern "C" 
{
#endif

enum e_logger_level_t
{
	LOGGER_ALL = 0,
	LOGGER_TRACE = 2,
	LOGGER_DEBUG = 3,
	LOGGER_INFO = 4,
	LOGGER_WARN = 5,
	LOGGER_ERROR = 6,
	LOGGER_FATAL = 7,
	LOGGER_NULL = 8,
};

typedef struct s_logger_t
{
	int			fd;
	int32_t		level;
	uint32_t	file_size_limit;
	uint32_t	file_count_limit;
	char		file_name[MAX_PATH];//just name format
	uint32_t	file_count;			//current log file count
	uint32_t	file_size;			//current log file size
} Logger;

extern Logger* default_logger;

int32_t logger_impl(/*const */Logger* self, const char* buf, uint32_t l);

extern char logger_time_string_buffer[24];
void logger_update_time_string();

#define _logger_wrapper(logger, logger_level, file, line, func, fmt, ...) \
if (LOGGER_##logger_level >= logger->level)\
{\
	char _lg_buf[LOGGER_LINE_SIZE]; \
	int32_t _lg_l; \
	logger_update_time_string(); \
	_lg_l = snprintf(_lg_buf, LOGGER_LINE_SIZE, "["#logger_level"] %s [%s:%u: %s] "fmt"\n", logger_time_string_buffer, file, line, func, ##__VA_ARGS__); \
	if(_lg_l>0)logger_impl(logger,_lg_buf,_lg_l<LOGGER_LINE_SIZE?_lg_l:LOGGER_LINE_SIZE-1); \
}\
else\
{\
}

#define logger_trace(logger, fmt, ...) _logger_wrapper(logger, TRACE, __FILE__, __LINE__, __FUNCSIG__, fmt, ##__VA_ARGS__)
#define logger_debug(logger, fmt, ...) _logger_wrapper(logger, DEBUG, __FILE__, __LINE__, __FUNCSIG__, fmt, ##__VA_ARGS__)
#define logger_info(logger, fmt, ...) _logger_wrapper(logger, INFO, __FILE__, __LINE__, __FUNCSIG__, fmt, ##__VA_ARGS__)
#define logger_warn(logger, fmt, ...) _logger_wrapper(logger, WARN, __FILE__, __LINE__, __FUNCSIG__, fmt, ##__VA_ARGS__)
#define logger_error(logger, fmt, ...) _logger_wrapper(logger, ERROR, __FILE__, __LINE__, __FUNCSIG__, fmt, ##__VA_ARGS__)
#define logger_fatal(logger, fmt, ...) _logger_wrapper(logger, FATAL, __FILE__, __LINE__, __FUNCSIG__, fmt, ##__VA_ARGS__)

#define TRACE_FUNCTION(func, ...) \
{\
	struct timeval _t_s;\
	struct timeval _t_e;\
	gettimeofday(&_t_s, NULL);\
	func(__VA_ARGS__); \
	gettimeofday(&_t_e, NULL); \
	logger_debug(default_logger, "%s return, cost:%u.%06us", #func, (uint32_t)(_t_e.tv_sec - _t_s.tv_sec - (_t_e.tv_usec < _t_s.tv_usec)), (uint32_t)(_t_e.tv_usec < _t_s.tv_usec)*1000000 + _t_e.tv_usec - _t_s.tv_usec)); \
}

#define TRACE_FUNCTION_CHECK_RET(ret, func, ...) \
{\
	struct timeval _t_s; \
	struct timeval _t_e; \
	gettimeofday(&_t_s, NULL); \
	ret = func(__VA_ARGS__); \
	gettimeofday(&_t_e, NULL); \
	logger_debug(default_logger, "%s return:%d, cost:%u.%06us", #func, ret, (uint32_t)(_t_e.tv_sec - _t_s.tv_sec - (_t_e.tv_usec < _t_s.tv_usec)), (uint32_t)((_t_e.tv_usec < _t_s.tv_usec)*1000000 + _t_e.tv_usec - _t_s.tv_usec)); \
}

#ifdef _DEBUG
#define logger_assert(expr) \
if (!(expr))\
{\
	int32_t _logger_error_number = errno;\
	logger_fatal(default_logger, "Assertion '"#expr"' failed. errno:%d[%s]", _logger_error_number, strerror(_logger_error_number)); \
	fprintf(stderr, "%s: %u: %s: Assertion '"#expr"' failed.\n", __FILE__, __LINE__, __FUNCSIG__); \
	abort();\
}

#define logger_assert_ret_val(ret) \
if (ret != 0)\
{\
	logger_fatal(default_logger, #ret" = %d.", ret); \
	fprintf(stderr, "%s: %u: %s: "#ret" = %d\n", __FILE__, __LINE__, __FUNCSIG__, ret); \
	abort(); \
}

#define logger_assert_false() \
{\
	logger_fatal(default_logger, "Assertion fatal error."); \
	fprintf(stderr, "%s: %u: %s: Assertion fatal error.\n", __FILE__, __LINE__, __FUNCSIG__); \
	abort(); \
}
#else
#define logger_assert(expr)
#define logger_assert_ret_val(ret)
#define logger_assert_false()
#endif

int32_t logger_initialize(Logger* self);
void logger_finalize(Logger* self);
int32_t logger_load_cfg(Logger* self, const char* cfg_file);
int32_t logger_get_dir(Logger* self, char* dir_path, uint32_t* len);

#ifdef __cplusplus
}
#endif

#endif
