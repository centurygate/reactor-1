/**
# -*- coding:UTF-8 -*-
*/

#include <stdlib.h>
#include <string.h>
#include "define.h"
#include "fs_utility.h"

#ifdef _WIN32
#include <Windows.h>
#endif

int32_t is_path_exists(const char* path)
{
	struct stat file_stat;
	int32_t ret = stat(path, &file_stat);
	if(ret != 0)
	{
		if (errno == ENOENT)
			return 0;
		else if (errno == ENOTDIR)
			return 0; // return 1;
		else return 3;
	}

	if (S_ISDIR(file_stat.st_mode))
		return 2;
	else return 1;
}

#define TEST_FILE_NAME ".____etm____test____file____"
int32_t is_dir_writable(const char* path)
{
	FILE* f;
	uint32_t path_len;
	char* test_file;

	if(path == NULL || *path==0) return 0;

	path_len = strlen(path);
	test_file = (char*)alloca(path_len + 32);
	if(test_file == NULL) return 0;

	memcpy(test_file, path, path_len);
	format_path(test_file, &path_len);
	if(test_file[path_len-1] != DIR_DELIMITER)
	{
		test_file[path_len++] = DIR_DELIMITER;
	}
	memcpy(test_file+path_len, TEST_FILE_NAME, strlen(TEST_FILE_NAME)+1);

	f = fopen(test_file, "w");
	if(f != NULL)
	{
		fclose(f);
		unlink(test_file);
		return 1;
	}
	else
	{
		return 0;
	}
}
#undef TEST_FILE_NAME

#define is_cur_or_parent_dir(name) ((name)[0]=='.' && ((name)[1] == 0 || ((name)[1] == '.' && (name)[2] == 0)))

int32_t format_path(char* path, uint32_t* path_len)
{
	uint32_t i;
	char* p_out = path;
	if(*p_out==LINUX_DELIMITER || *p_out==WIN_DELIMITER)
		*p_out = DIR_DELIMITER;
	++p_out;

	for( i = 1; i<*path_len; ++i)
	{
		if(path[i]==LINUX_DELIMITER || path[i]==WIN_DELIMITER)
		{
			if(p_out[-1] != DIR_DELIMITER)
				*p_out++ = DIR_DELIMITER;
		}
		else *p_out++ = path[i];
	}
	*path_len = p_out - path;
	*p_out = 0;
	return 0;
}

#ifdef __GNUC__

static int32_t remove_dir_impl(char* path, uint32_t path_len)
{
	struct dirent* find_data;
	DIR* h_find;

	path[path_len++] = DIR_DELIMITER;
	path[path_len] = 0;
	h_find = opendir(path);
	if(h_find == NULL) return errno;

	find_data = readdir(h_find);
	while(find_data != NULL)
	{
		if(find_data->d_type==DT_DIR)
		{
			if(!is_cur_or_parent_dir(find_data->d_name))
			{
				uint32_t len = strlen(find_data->d_name);
				if(len + path_len < MAX_PATH)
				{
					memcpy(path+path_len, find_data->d_name, len+1);
					remove_dir_impl(path, path_len + len);
				}
				else
				{
					//do nothing
				}
			}
		}
		else
		{
			uint32_t len = strlen(find_data->d_name);
			if(len + path_len < MAX_PATH)
			{
				memcpy(path+path_len, find_data->d_name, len+1);
				unlink(path);
			}
			else
			{
				//do nothing
			}
		}
		find_data = readdir(h_find);
	}
	closedir(h_find);

	path[path_len] = 0;
	rmdir(path);
	return 0;
}

static int32_t remove_empty_dir_impl(char* path, uint32_t path_len)
{
	int32_t is_empty = 1;
	struct dirent* find_data;
	DIR* h_find;

	path[path_len++] = DIR_DELIMITER;
	path[path_len] = 0;
	h_find = opendir(path);
	if(h_find == NULL) return errno;

	find_data = readdir(h_find);
	while(find_data != NULL)
	{
		if(find_data->d_type==DT_DIR)
		{
			if(!is_cur_or_parent_dir(find_data->d_name))
			{
				uint32_t len = strlen(find_data->d_name);
				if(len + path_len < MAX_PATH)
				{
					memcpy(path+path_len, find_data->d_name, len+1);
					if(remove_empty_dir_impl(path, path_len + len)!=0)
						is_empty = 0;
				}
				else
				{
					//do nothing
					is_empty = 0;
				}
			}
		}
		else is_empty = 0;

		find_data = readdir(h_find);
	}
	closedir(h_find);
	if(is_empty)
	{
		path[path_len] = 0;
		return 0==rmdir(path)?0:errno;
	}
	else return -1;
}

#elif defined(_WIN32)

int32_t remove_dir_impl(char* path, uint32_t path_len)
{
	SHFILEOPSTRUCT FileOp;
	path[path_len+1] = 0;

	FileOp.fFlags = FOF_NOCONFIRMATION;
	FileOp.hNameMappings = NULL;
	FileOp.hwnd = NULL;
	FileOp.lpszProgressTitle = NULL;
	FileOp.pFrom = path;
	FileOp.pTo = NULL;
	FileOp.wFunc = FO_DELETE;
	return SHFileOperation(&FileOp);
}

static int32_t remove_empty_dir_impl(char* path, uint32_t path_len)
{
	int32_t is_empty = 1;
	WIN32_FIND_DATA find_data;
	HANDLE h_find;

	path[path_len++] = DIR_DELIMITER;
	path[path_len++] = '*';
	path[path_len++] = '.';
	path[path_len++] = '*';
	path[path_len] = 0;
	h_find = FindFirstFile(path, &find_data);
	if(INVALID_HANDLE_VALUE == h_find) return GetLastError();

	path_len -= 3;
	path[path_len] = 0;

	for(;;)
	{
		if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(!is_cur_or_parent_dir(find_data.cFileName))
			{
				uint32_t len = strlen(find_data.cFileName);
				if(len + path_len < MAX_PATH)
				{
					memcpy(path+path_len, find_data.cFileName, len+1);
					if(remove_empty_dir_impl(path, path_len + len)!=0)
						is_empty = 0;
				}
				else
				{
					//do nothing
					is_empty = 0;
				}
			}
		}
		else is_empty = 0;

		if(!FindNextFile(h_find, &find_data)) break;
	}
	FindClose(h_find);
	if(is_empty)
	{
		path[path_len] = 0;
		return 0==rmdir(path)?0:errno;
	}
	else return -1;
}

#else
#error os not support
#endif

int32_t remove_dir(const char* path, uint32_t path_len)
{
	char* dir_path = (char*)alloca(MAX_PATH + 16);

	if(path_len < MAX_PATH)
	{
		memcpy(dir_path, path, path_len+1);
		format_path(dir_path, &path_len);
		if(dir_path[path_len-1] == DIR_DELIMITER)
			dir_path[--path_len] = 0;
		return remove_dir_impl(dir_path, path_len);
	}
	else return ERR_INVALID_PATH;
}

int32_t remove_empty_dir(const char* path, uint32_t path_len)
{
	char* dir_path = (char*)alloca(MAX_PATH + 16);

	if(path_len < MAX_PATH)
	{
		int32_t ret;
		memcpy(dir_path, path, path_len+1);
		format_path(dir_path, &path_len);
		if(dir_path[path_len-1] == DIR_DELIMITER)
			dir_path[--path_len] = 0;
		ret = remove_empty_dir_impl(dir_path, path_len);
		return ret!=-1 ? ret : 0;
	}
	else return ERR_INVALID_PATH;
}

int32_t create_dir(const char* path, uint32_t path_len)
{
	int32_t ret;
	char* p;
	char* pend;
	char* dir_path = (char*)alloca(path_len+1);
	memcpy(dir_path, path, path_len);
	dir_path[path_len++] = DIR_DELIMITER;
	ret = format_path(dir_path, &path_len);
	pend = dir_path + path_len;

	for(p = dir_path+1 ; p<pend; ++p)
	{
		if(*p == DIR_DELIMITER)
		{
			*p = 0;
			ret = mkdir(dir_path, 0777);
			*p = DIR_DELIMITER;
			if (ret != 0)
			{
				if(errno == EEXIST) ret = 0;
				else
				{
					ret = errno;
					if(errno != EACCES)
						return errno;
				}
			}
		}
	}
	return ret;
}
