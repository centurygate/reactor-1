/**
# -*- coding:UTF-8 -*-
*/

#ifndef _FS_UTILITY_H_
#define _FS_UTILITY_H_

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifndef MAX_NAME
#define MAX_NAME 255
#endif

#define WIN_DELIMITER '\\'
#define LINUX_DELIMITER '/'

#ifdef __GNUC__
#include <unistd.h>
#include <dirent.h>
#ifndef MAX_PATH
#define MAX_PATH 4096
#endif
#define DIR_DELIMITER LINUX_DELIMITER
#elif defined _WIN32
#include <direct.h>
#include <share.h>
#include <io.h>
#ifndef STDIN_FILENO
#define STDIN_FILENO    0   /* Standard input.  */
#define STDOUT_FILENO   1   /* Standard output.  */
#define STDERR_FILENO   2   /* Standard error output.  */

#define DIR_DELIMITER WIN_DELIMITER

#define mkdir(path, mode) _mkdir(path)

#endif
#else
#error os not support
#endif

#ifdef __cplusplus
extern "C" 
{
#endif

/**
 *检测文件或目录是否已存在
 *@return 当且仅当检测到该路径不存在时返回 0
 *        如果出现错误，返回 3
 *        如果是个文件，返回 1
 *        如果是个目录，返回 2
 */
int32_t is_path_exists(const char* path);

/**
 *检测目录是否可写
 *尝试在目标目录创建一个文件并删除
 *@return 当且仅当该目录存在并且创建文件成功时返回true
 *        如果在检测过程中出现错误，返回false
 */
int32_t is_dir_writable(const char* path);

/**
 *删除目录
 *删除目标目录及目录内所有文件
 *@return 成功返回0，否则返回错误码
 */
int32_t remove_dir(const char* path, uint32_t path_len);

/**
 *删除空目录
 *删除目标目录及其所有子目录中的空目录
 *如果不想递归删除空目录，请使用 rmdir(path);
 *@return 成功返回0，否则返回错误码
 */
int32_t remove_empty_dir(const char* path, uint32_t path_len);

/**
 *格式化路径
 *将路径中的目录分隔符转换到相应平台，并去除重复出现的目录分隔符
 *可以用于文件或目录
 *@return 总是成功
 */
int32_t format_path(char* path, uint32_t* path_len);

/**
 *创建目录
 *如果路径中的目录不存在，创建
 *@return 0表示成功
 */
int32_t create_dir(const char* path, uint32_t path_len);

#ifdef __cplusplus
}
#endif

#endif
