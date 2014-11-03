/**
# -*- coding:UTF-8 -*-
*/

#include <ctype.h>
#include <stdio.h>
#include "logger.h"
#include "string_utility.h"

//debug
//#include <assert.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" 
{
#endif

static Logger _default_logger = {STDOUT_FILENO, LOGGER_ALL, 0, 0, "", 0, 0};
static int8_t s_is_logger_ok = 0;

Logger* default_logger = &_default_logger;

#define	LOG_FILE_NAME_FORMAT(file,name,num)	snprintf(file, sizeof(file), "%s.%d", name, num);


time_t last_logger_time = 0;
char logger_time_string_buffer[24];
void logger_update_time_string()
{
	struct tm _lg_tm;
	time_t _lg_t = time(NULL);
	if (_lg_t != last_logger_time)
	{
		last_logger_time = _lg_t;
		localtime_r(&_lg_t, &_lg_tm);
		sprintf(logger_time_string_buffer, "[%04u-%02u-%02u %02u:%02u:%02u]", _lg_tm.tm_year + 1900, _lg_tm.tm_mon + 1, _lg_tm.tm_mday, _lg_tm.tm_hour, _lg_tm.tm_min, _lg_tm.tm_sec);
	}
}

static int32_t get_new_log_file(Logger* self);
static int32_t s_thread_counter = 0;					//线程计数器
// static int32_t s_write_cnt = 0;						//解决多线程写日志文件同步磁盘问题。防数据丢失
// static pthread_mutex_t s_mutex;

int32_t logger_impl(/*const */Logger* self, const char* buf, uint32_t l)
{
	int32_t ret_val = 0;
	int32_t fd = 0;
 	uint32_t len = 0;
	//debug
#ifdef _DEBUG
// 	uint32_t file_size = 0;
// 	char file_name[1024] = {0};
// 	uint32_t file_index = 0;
// 	struct tm tt;
// 	time_t t;
#endif

	//支持标准输出与错误输出
	if (self->fd <= 2)
	{
		write(self->fd ,buf, l);
		return 0;
	}

	if (self->file_size+l > self->file_size_limit)
	{
		++s_thread_counter;
		if (1 == s_thread_counter && self->file_size+l > self->file_size_limit)
		{
/*
#ifdef _DEBUG
			//The adjustment  of  the file offset(lseek) and the write operation are performed as an atomic step.
			file_size = lseek(self->fd, 0, SEEK_CUR);
			file_index = (self->file_count-1) % self->file_count_limit + 1;
			LOG_FILE_NAME_FORMAT(file_name, self->file_name, file_index);
			if(-1 == file_size)
			{
				fprintf(stderr, "error!file_name:%s,cnt:%d,lseek errno:%s\n", file_name, self->file_count, strerror(errno));
			}

			fprintf(stdout, "----thread_id:0x%x in,file_name:%s,fd=%d,file_count:%02u,file_size:%u=%uk,real_size:%u=%uK\n", (uint32_t)pthread_self(),
				file_name, self->fd, self->file_count, self->file_size, self->file_size/1024, file_size, file_size/1024);
#endif
*/
			//切换文件。先缓存原fd，延迟关闭。以解决每次切换文件时，每个线程都会有一次写文件失败的几率。
			//不过现在最多还是会有一个线程有写文件失败的概率
			fd = self->fd;
			ret_val = get_new_log_file(self);
			if (0 != ret_val)
			{
				fprintf(stderr, "error!get_new_log_file fail!thread_id:0x%x\n", (uint32_t)pthread_self());
				close(fd);
				--s_thread_counter;
				return ret_val;
			}
			close(fd);
#ifdef _DEBUG
// 			t = time(NULL);
// 			localtime_r(&t, &tt);
// 			fprintf(stdout, "----thread_id:0x%x out,fd=%d,self->fd=%d,get_new_log_file [%02u:%02u:%02u] success!size=%u\n", (uint32_t)pthread_self(),fd, self->fd, tt.tm_hour, tt.tm_min, tt.tm_sec, self->file_size);
#endif
		}
		--s_thread_counter;
	}


	while (s_thread_counter > 0 /*|| s_write_cnt > 0*/)
	{
#ifdef _DEBUG
// 		fprintf(stdout, "--------------thread_id:0x%x sleep!line:%d\n", (uint32_t)pthread_self(),__LINE__);
#endif
		sleep(1);
	}

// 	++s_write_cnt;
	//如果不将write/fsync作为一个原子操作，那么丢失的数据会随线程数的增多而增多.
	len = write(self->fd ,buf, l);
#ifdef _DEBUG
	if (l != len)
	{
		//fprintf(stdout, "^^^^^^^^^^^thread_id:0x%x,fd=%d,len=%u,l=%d!\n", (uint32_t)pthread_self(),self->fd,len,l);
	}
#endif
	if (-1 == len)
	{
		//在切换文件过程中，最多还是会有一个线程有写文件失败的概率。
		//原因是一个线程陷入内核写文件过程中，可能会有另外一个线程在外面关闭描述符。
		if (s_is_logger_ok)
		{
			fprintf(stderr, "!!!!!!!!!!!!error!thread_id:0x%x,fd=%d,write log error!error:%s\n",(uint32_t)pthread_self(),self->fd, strerror(errno));
			s_is_logger_ok = 0;
		}
		return errno;
	}

	while (len < l)
	{//errno == EINTR		
		l -= len;
		buf += len;
		len = write(self->fd, buf, l);
	}

	//On kernels before 2.4, fsync() on big files can be inefficient.
	//An alternative might be to use the O_SYNC flag to open(2).
	//即使使用fsync，系统也是无法100%保证数据同步到磁盘的。单线程时，可以去掉该方法
/*	ret_val = fsync(self->fd);
	if (0 != ret_val)
	{
		fprintf(stderr, "!!!!error!thread_id:0x%x,fd=%d,write log error!error:%s\n",(uint32_t)pthread_self(),self->fd, strerror(errno));
// 		--s_write_cnt;
		return ret_val;
	}
*/
	self->file_size += len;
	s_is_logger_ok = 1;
// 	--s_write_cnt;
	return 0;
}

int32_t logger_initialize(Logger* self)
{
	if (NULL == self)
	{
		fprintf(stderr, "error!null point!\n");
		return ERR_INVALID_PARAMETER;
	}

	s_is_logger_ok = 1;
	memset(self, 0, sizeof(Logger));
	return 0;
}

int32_t logger_load_cfg(Logger* self, const char* cfg_file)
{
	int32_t ret_val = 0;
	char buf[1024] = {0};
	char file_size_unit = 0;
	int32_t pos = 0;
	//int32_t len = 0;
	char str[64] = {0};
	char* key = NULL;
	
	if (0 == is_path_exists(cfg_file))
	{
		return ERR_NOT_FOUND;
	}

	FILE* pFile = fopen(cfg_file, "r");
	if (NULL == pFile)
	{
		fprintf(stderr, "error!open log cfg_file file fail!ret=%d,error:%s\n", ret_val, strerror(errno));
		return errno;
	}

	while (NULL != fgets(buf, 1024, pFile))
	{
		pos = 0;
		//len = strlen(buf);
		while (0 != isspace(buf[pos]))
		{
			++pos;
		}

		if ('#' == buf[pos] || '/' == buf[pos])
			continue;

		key = strtok(&buf[pos], "=");
		if(key == NULL)
			continue;
		char *value = strtok(NULL, "=");
		if(value == NULL)
			continue;

		strtrim(value, strlen(value));
		if(0 == strcmp(key, "level"))
		{
			memset(str, 0, sizeof(str));
			sscanf(value, "%s", str);
			if(strcmp(str, "ALL")==0)
				self->level = LOGGER_ALL;
			else if(strcmp(str, "TRACE")==0)
				self->level = LOGGER_TRACE;
			else if(strcmp(str, "DEBUG")==0)
				self->level = LOGGER_DEBUG;
			else if(strcmp(str, "INFO")==0)
				self->level = LOGGER_INFO;
			else if(strcmp(str, "WARN")==0)
				self->level = LOGGER_WARN;
			else if(strcmp(str, "ERROR")==0)
				self->level = LOGGER_ERROR;
			else if(strcmp(str, "FATAL")==0)
				self->level = LOGGER_FATAL;
			else if(strcmp(str, "NULL")==0)
				self->level = LOGGER_NULL;
			else{
				fprintf(stderr, "no log level config!use defalut debug level!\n");
				self->level = LOGGER_DEBUG;
			}
		}
		else if (0 == strcmp(key, "max_file_size"))
		{
			ret_val = sscanf(value, "%d%[KMG]", &self->file_size_limit, &file_size_unit);
			if (2 != ret_val)
			{
				fprintf(stderr, "you had better input file_size_unit K,M,G.\n");
			}
			switch (file_size_unit)
			{
			case 'K':
				self->file_size_limit *= 1024;
				break;
			case 'M':
				self->file_size_limit *= 1024*1024;
				break;
			case 'G':
				self->file_size_limit *= 1024*1024*1024;
				break;
			default:
				self->file_size_limit *= 1024*1024;
			}
		}
		else if (0 == strcmp(key, "max_file_number"))
		{
			sscanf(value, "%d", &self->file_count_limit);
		}		
		else if (0 == strcmp(key, "file"))
		{
			strncpy(self->file_name, value, sizeof(self->file_name) -1);
		}		
	}

	if (0 == strlen(self->file_name))
	{
		fprintf(stderr, "error!log name is needed!,line:%d\n", __LINE__);
		strcpy(self->file_name, "./etm.log");
		return ERR_INVALID_PARAMETER;
	}
/*	//debug
#ifdef _DEBUG
	fprintf(stdout, "log level:%d,cnt:%d,size:%uByte=%uk,name:[%s],line:%d\n", self->level, self->file_count_limit, 
		self->file_size_limit, self->file_size_limit/1024, self->file_name, __LINE__);
#endif
*/
	if (0 == strcmp(self->file_name, "stdout"))
	{
		self->fd = 0;
	}
	else
	{
		ret_val = get_new_log_file(self);
		if (0 != ret_val)
		{
			fprintf(stderr, "error!get_new_log_file fail!ret=%d.\n", ret_val);
			return ret_val;
		}
	}

	return 0;
}

int32_t logger_get_dir(Logger* self, char* dir_path, uint32_t* len)
{
	uint32_t dir_len = 0;
	char* p_dir_end = strrchr(self->file_name, DIR_DELIMITER);
	if (NULL == p_dir_end)
	{
		dir_path[0] = 0;
		*len = 0;
		return 0;
	}

	dir_len = p_dir_end - self->file_name +1;
	if (dir_len >= *len)
	{
		*len = dir_len;
		return ERR_NOT_ENOUGH_BUFFER;
	}

	*len = dir_len;
	memcpy(dir_path, self->file_name, dir_len);
	return 0;
}

void logger_finalize(Logger* self)
{
	if (self->fd < 3)
	{
		return;
	}
/*
#ifdef _DEBUG
	int32_t ret_val = 0;
	uint32_t file_size = 0;
	uint32_t file_index = 0;
	char file_name[1024] = {0};

	file_index = (self->file_count-1) % self->file_count_limit + 1;
	LOG_FILE_NAME_FORMAT(file_name, self->file_name, file_index);
	file_size = lseek(self->fd, 0, SEEK_CUR);
	if(-1 == file_size)
	{
		fprintf(stderr, "error!file_name:%s,cnt:%d,stat errno:%s\n", file_name, self->file_count, strerror(errno));
	}

	fprintf(stdout, "----thread_id:0x%x in,file_name:%s,fd=%d,file_count:%02u,file_size:%u=%uk,real_size:%u=%uK\n", (uint32_t)pthread_self(), 
		file_name, self->fd, self->file_count, self->file_size, self->file_size/1024, file_size, file_size/1024);
#endif
*/
	close(self->fd);
	return ;
}

int32_t get_new_log_file(Logger* self)
{
	int32_t ret_val = 0;
	int32_t i = 0;
	int32_t offset = 0;
	char file[MAX_PATH] = {0};
	char file_new[MAX_PATH] = {0};

	memset(file, 0, sizeof(file));
	if (1 == self->file_count_limit)
	{
		self->fd = open(self->file_name, O_CREAT|O_WRONLY|O_TRUNC/*|O_SYNC*/, 0666);
		if (-1 == self->fd)
		{
			fprintf(stderr, "error!open file fail!ret=%d,error:%s\n", errno, strerror(errno));
			return -1;
		}
		self->file_size = 0;
#ifdef _DEBUG
// 		fprintf(stdout, "~~~~~~~~thread_id:0x%x,get_new_log_file,fd=%d,size=%u,line:%d!\n", (uint32_t)pthread_self(),self->fd,self->file_size, __LINE__);
#endif
		return 0;
	}

	if (self->file_count < self->file_count_limit)
	{
		//rename file
		for (i = self->file_count - 1; i >= 0; --i)
		{
			if (i > 0)
			{
				LOG_FILE_NAME_FORMAT(file, self->file_name, i);
			}
			else
			{
				strncpy(file, self->file_name, sizeof(file));
			}

			LOG_FILE_NAME_FORMAT(file_new, self->file_name, i+1);
			if (is_path_exists(file_new))
			{
				ret_val = unlink(file_new);
				if (0 != ret_val)
				{
					fprintf(stderr, "unlink file[%s] failed!errno:%d.\n", file_new, errno);
				}
			}

			ret_val = rename(file, file_new);
			if (0 != ret_val)
			{
				fprintf(stderr, "error!rename old_file[%s] new_file[%s] failed!ret=%d,error:%s\n", file, file_new, errno, strerror(errno));
			}
		}

		self->fd = open(self->file_name, O_CREAT|O_WRONLY|O_APPEND/*|O_SYNC*/, 0666);
		if (-1 == self->fd)
		{
			fprintf(stderr, "error!open file[%s] fail!ret=%d,error:%s\n", file, errno, strerror(errno));
			return -1;
		}

		offset = lseek(self->fd, 0, SEEK_END);
		if (-1 == offset)
		{
			fprintf(stderr, "error!lseek fail!errno=%d,error:%s\n", errno, strerror(errno));
			offset = 0;
		}

		self->file_size = offset;
		++self->file_count;
#ifdef _DEBUG
// 		fprintf(stdout, "~~~~~~~~thread_id:0x%x,get_new_log_file,fd=%d,size=%u,line:%d!\n", (uint32_t)pthread_self(),self->fd,self->file_size, __LINE__);
#endif
		return 0;
	}
	
	LOG_FILE_NAME_FORMAT(file, self->file_name, self->file_count_limit - 1);
	ret_val = unlink(file);
	if (0 != ret_val)
	{
		fprintf(stderr, "unlink file[%s] failed!errno:%d.\n", file, errno);
	}

	//rename file
	for (i = self->file_count_limit - 1; i > 0; --i)
	{
		if (i > 1)
		{
			LOG_FILE_NAME_FORMAT(file, self->file_name, i-1);
		}
		else
		{
			strncpy(file, self->file_name, sizeof(file));
		}
		LOG_FILE_NAME_FORMAT(file_new, self->file_name, i);
		ret_val = rename(file, file_new);
		if (0 != ret_val)
		{
			fprintf(stderr, "error!rename old_file[%s] new_file[%s] failed!ret=%d,error:%s\n", file, file_new, errno, strerror(errno));
		}
	}

	self->fd = open(self->file_name, O_CREAT|O_WRONLY|O_APPEND, 0666);
	if (-1 == self->fd)
	{
		fprintf(stderr, "error!open file[%s] fail!ret=%d,error:%s\n", self->file_name, errno, strerror(errno));
		return -1;
	}

	offset = lseek(self->fd, 0, SEEK_END);
	if (-1 == offset)
	{
		fprintf(stderr, "error!lseek fail!errno=%d,error:%s\n", errno, strerror(errno));
		offset = 0;
	}
	self->file_size = offset;
/*
#ifdef _DEBUG
	fprintf(stdout, "~~~~~~~~thread_id:0x%x,get_new_log_file,fd=%d,count:%d,size=%u,line:%d!\n", (uint32_t)pthread_self(),self->fd,self->file_count,self->file_size, __LINE__);
#endif
*/
	return 0;
}

#ifdef __cplusplus
}

#endif

