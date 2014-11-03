/**
# -*- coding:UTF-8 -*-
*/

#include "settings.h"
#include "string_utility.h"
#include "define.h"
// #include "logger.h"

#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <assert.h>

static void		setting_push_back_set_item(double_list_t* plist, const char* key, const char *val, char** p_comment_line, uint32_t* plen);
static void*	setting_get_set_item_pos(Settings* self, const char* section, const char* key, double_list_t** pos_list);
static int32_t	setting_get_new_node(Settings* self, set_item** pitem);
char*	setting_get_line_end(const char* p_line)
{
	char* p_end = strchr(p_line, '\r');
	if (NULL == p_end)
	{
		return strchr(p_line, '\n');
	}

	return p_end;
}

static int32_t	setting_read_buffer_by_line(Settings* self, char* p_line, int32_t line_len, double_list_t** pp_cur_list, 
											char** pp_comment_line, uint32_t* p_comment_len, char* p_cur_section, uint32_t section_len)
{
	int32_t			ret_val = 0;
	//char*			pos = 0;
	char*			ptmp = NULL;
	int32_t			is_need_free = 0;
	set_item*		pitem = NULL;
#ifdef _DEBUG
	double_list_t*	pos_list = NULL;
#endif

	char *p_val = p_line;
	if (line_len < 2) return ret_val;//skip empty line
	skip_space(p_val);

	//comment lines
	if (*p_val == ';' || '#' == *p_val)
	{
		if (NULL == *pp_comment_line)
		{
			STRING_ASSIGN((*pp_comment_line), (*p_comment_len), p_line, line_len, ret_val);
			if (0 != ret_val)
			{
				fprintf(stderr, "realloc fail:%s!\n", strerror(errno));
			}
			return ret_val;
		}

		STRING_APPEND((*pp_comment_line), (*p_comment_len), p_line, line_len, ret_val);
		if (0 != ret_val)
		{
			fprintf(stderr, "realloc fail:%s!\n", strerror(errno));
		}

		return ret_val;
	}

	//section item
	if ('[' == *p_val)
	{
		ptmp = strchr(p_val, ']');
		if (NULL == ptmp)
		{
			fprintf(stderr, "syntactic error!content:%s.\n", p_val);
			assert(0);
			return ERR_INVALID_PARAMETER;
		}

		++p_val;
		ret_val = setting_get_new_node(self, &pitem);
		if (0 != ret_val)
		{
			return ret_val;
		}

		*ptmp = '\0';
		pitem->pkey = strdup(p_val);
		if (NULL == pitem->pkey)
		{
			fprintf(stderr, "strdup fail:%s!\n", strerror(errno));
			return ERR_ETM_OUT_OF_MEMORY;
		}
		
#ifdef _DEBUG
		if (ptmp == pitem->pkey)
		{
			fprintf(stdout, "-------syntax error!p_cur_section null.\n");
		}
		memset(p_cur_section, 0, section_len);
		strncpy(p_cur_section, pitem->pkey, section_len-1);

		if (NULL != setting_get_set_item_pos(self, "", pitem->pkey, &pos_list))
		{
			fprintf(stdout, "found repeat p_cur_section item:%s.\n", pitem->pkey);
		}
#endif
		if (NULL != *pp_comment_line)
		{
			pitem->pnote = *pp_comment_line;
			*pp_comment_line = NULL;
			p_comment_len = 0;
		}

		*pp_cur_list = pitem->plist;
		return ret_val;
	}

	//global items
	if (0 == self->setting_items._list_size)
	{
		ret_val = setting_get_new_node(self, &pitem);
		if (0 != ret_val)
		{
			return ret_val;
		}

		if (NULL != *pp_comment_line)
		{
			pitem->pnote = *pp_comment_line;
			*pp_comment_line = NULL;
			p_comment_len = 0;
		}

		pitem->pkey = strdup("");
		if (NULL == pitem->pkey)
		{
			fprintf(stderr, "strdup fail:%s!\n", strerror(errno));
			return ERR_ETM_OUT_OF_MEMORY;
		}
		*pp_cur_list = pitem->plist;
	}

	while (*p_val != '\0')
	{
		if (*p_val == '=')
		{
			*p_val = '\0';
			//erase '\n'
/*
			while (isspace(*(++p_val)) && p_val < p_line + line_len - 1);
			pos = p_val + strlen(p_val) - 1;
			while (isspace(*pos) && pos > p_val)
			{
				*pos = '\0';
				--pos;
			}
*/
			++p_val;
			//ptmp = strchr(p_val, '\n');
			ptmp = setting_get_line_end(p_val);
			if (NULL == ptmp)
			{
				fprintf(stderr, "unknown error!content:%s.\n", p_val);
				assert(0);
				return ERR_INVALID_PARAMETER;
			}

			if (ptmp == p_val)
			{
				p_val = strdup("");
				if (NULL == p_val)
				{
					fprintf(stderr, "strdup fail:%s!\n", strerror(errno));
					assert(0);
					return ERR_ETM_OUT_OF_MEMORY;
				}
				is_need_free = 1;
			}
			else
			{
				*ptmp = '\0';
			}
			
#ifdef _DEBUG
			if (NULL != setting_get_set_item_pos(self, p_cur_section, p_line, &pos_list))
			{
				fprintf(stdout, "found repeat config p_cur_section::item.%s::%s.\n", p_cur_section, p_line);
				break;
			}
#endif
			setting_push_back_set_item(*pp_cur_list, p_line, p_val, pp_comment_line, p_comment_len);
			if (1 == is_need_free)
			{
				free(p_val);
				is_need_free = 0;
			}
			
			break;
		}

		++p_val;
	}

	return ret_val;
}

int32_t setting_initialize(Settings* self)
{
	self->changed = 0;
	self->file_name = NULL;
	double_list_init(&self->setting_items);
	return 0;
}

int32_t setting_load_cfg(Settings* self, const char* cfg_file)
{
	FILE*			settings_file;
	int32_t			ret_val = 0;
	char			buffer[1024];
	double_list_t*	p_cur_list = NULL;
	char*			p_comment_line = NULL;
	uint32_t		comment_len = 0;
	char			section[1024] = {0};

	if (self->file_name == NULL)
	{
		self->file_name = strdup(cfg_file);
	}

	settings_file = fopen(cfg_file, "r");

	if (settings_file == NULL)
	{
		fprintf(stderr, "fopen [%s] fail:%s.\n", cfg_file, strerror(errno));
		return errno;
	}

	while (fgets(buffer, sizeof(buffer), settings_file))
	{
		ret_val = setting_read_buffer_by_line(self, buffer, strlen(buffer), &p_cur_list, 
			&p_comment_line, &comment_len, section, sizeof(section));
		assert(ret_val == 0);
	}

	ret_val = fclose(settings_file);
	if (ret_val != 0){
		ret_val = errno;
	}

	return ret_val;
}

int32_t setting_load_cfg_from_memory(Settings* self, const char* buffer, uint32_t buf_len)
{
	int32_t			ret_val = 0;
	double_list_t*	p_cur_list = NULL;
	char*			p_comment_line = NULL;
	uint32_t		comment_len = 0;
	const char*		p_line = buffer;
	char*			p_next = NULL;
	uint32_t		line_len = 0;
	uint32_t		read_cnt = 0;
	char			section[1024] = {0};
	char			line_buffer[1024] = {0};

	while (read_cnt < buf_len)
	{
		memset(line_buffer, 0, sizeof(line_buffer));
		p_next = strchr(p_line, '\n');//widowns格式依然如此判定。内部处理\r\n
		++p_next;
		line_len = p_next - p_line;
		memcpy(line_buffer, p_line, line_len);
		ret_val = setting_read_buffer_by_line(self, line_buffer, line_len, &p_cur_list, 
			&p_comment_line, &comment_len, section, sizeof(section));
		assert(ret_val == 0);
		read_cnt += line_len;
		p_line = p_next;
	}
	
	return ret_val;
}

void setting_push_back_set_item(double_list_t* plist, const char* key, const char* val, char** p_comment_line, uint32_t* plen){
	set_item* pitem = calloc(1, sizeof(set_item));
	if (NULL == pitem)
	{
		fprintf(stderr, "calloc fail:%s!\n", strerror(errno));
		return;
	}

	double_list_push_back(plist, (void*)pitem);
	pitem->pkey = strdup(key);
	if (NULL == pitem->pkey)
	{
		fprintf(stderr, "strdup fail:%s!\n", strerror(errno));
		return;
	}

	pitem->pvalue = strdup(val);
	if (NULL == pitem->pvalue)
	{
		fprintf(stderr, "strdup fail:%s!\n", strerror(errno));
		return;
	}

	if (NULL != *p_comment_line)
	{
		pitem->pnote = *p_comment_line;
		*p_comment_line = NULL;
		plen = 0;
	}
}

void setting_finalize(Settings* self)
{
	double_list_iterator_t iter, end, sub_list_iter, sub_list_end;
	set_item*		pitem = NULL;
	set_item*		sub_pitem = NULL;

	iter = double_list_begin(&self->setting_items);
	end = double_list_end(&self->setting_items);

	setting_flush(self);

	for (; !double_list_iterator_equal(iter, end); double_list_iterator_next(&iter))
	{
		pitem = (set_item*)((double_list_node_t*)iter.list_data)->_data;
		if (NULL != pitem && NULL != pitem->plist)
		{
			sub_list_iter = double_list_begin(pitem->plist);
			sub_list_end = double_list_end(pitem->plist);
			//search sub list
			for (; !double_list_iterator_equal(sub_list_iter, sub_list_end); double_list_iterator_next(&sub_list_iter))
			{
				sub_pitem = (set_item*)((double_list_node_t*)sub_list_iter.list_data)->_data;
				if (NULL != sub_pitem->pnote)
				{
					free(sub_pitem->pnote);
					sub_pitem->pnote = NULL;
				}

				if (NULL != sub_pitem->pkey)
				{
					free(sub_pitem->pkey);
					sub_pitem->pkey = NULL;
				}

				if (NULL != sub_pitem->pvalue)
				{
					free(sub_pitem->pvalue);
					sub_pitem->pvalue = NULL;
				}
				free(sub_pitem);
			}

		}

		if (NULL != pitem->pkey)
		{
			free(pitem->pkey);
			pitem->pkey = NULL;
		}

		if (NULL != pitem->pvalue)
		{
			free(pitem->pvalue);
			pitem->pvalue = NULL;
		}

		if (NULL != pitem->pnote)
		{
			free(pitem->pnote);
			pitem->pnote = NULL;
		}

		if (NULL != pitem->plist)
		{
			double_list_clear(pitem->plist);
			free(pitem->plist);
			pitem->plist = NULL;
		}

		free(pitem);
	}

	free(self->file_name);
	self->file_name = NULL;
	double_list_clear(&self->setting_items);
}

int32_t setting_refresh(Settings* self)//no need to implement, for the time being
{
	return 0;
}

int32_t setting_flush(Settings* self)
{
	FILE*			settings_file;
	double_list_iterator_t iter, end, sub_list_iter, sub_list_end;
	set_item*		pitem = NULL;

	iter = double_list_begin(&self->setting_items);
	end = double_list_end(&self->setting_items);

	if (!self->changed)
		return 0;

	settings_file = fopen(self->file_name, "w");

	if (settings_file == NULL){
		return errno;
	}

	for (; !double_list_iterator_equal(iter, end); double_list_iterator_next(&iter))
	{
		pitem = (set_item*)((double_list_node_t*)iter.list_data)->_data;
		if (NULL != pitem->pnote)
		{//格式控制。使得注释紧连配置项，两者间没有空行。并且section之间有空行。
			if (NULL == pitem->pvalue && 0 < strlen(pitem->pkey))//comment for section
			{
				fprintf(settings_file, "\n");
			}
			fprintf(settings_file, "%s", pitem->pnote);
		}

		//section items
		if (strlen(pitem->pkey) > 0)
		{
			if (NULL == pitem->pnote)
			{
				fprintf(settings_file, "\n");
			}
			fprintf(settings_file, "[%s]\n", pitem->pkey);
		}

		if (NULL != pitem->plist)
		{
			//flush sub list
			sub_list_iter = double_list_begin(pitem->plist);
			sub_list_end = double_list_end(pitem->plist);
			for (; !double_list_iterator_equal(sub_list_iter, sub_list_end); double_list_iterator_next(&sub_list_iter))
			{
				//specific config items
				pitem = (set_item*)((double_list_node_t*)sub_list_iter.list_data)->_data;
				if (NULL != pitem->pnote)
				{
					fprintf(settings_file, "%s", pitem->pnote);
				}

				if (0 <= strlen(pitem->pkey) && 0 <= strlen(pitem->pvalue))
				{
					fprintf(settings_file, "%s=%s\n", pitem->pkey, pitem->pvalue);
				}
			}
		}
	}

	fclose(settings_file);

	self->changed = 0;
	return 0;
}

void setting_set_string(Settings* self, const char* section, const char* name, const char* value)
{
	double_list_t*		pos_list = NULL;
	int32_t				ret_val = 0;
	set_item*			pitem = NULL, *p_section_item;
	pitem = (set_item*)setting_get_set_item_pos(self, section, name, &pos_list);
	if (NULL != pitem)
	{
		if (0 == strcmp(pitem->pvalue, value)) return;
		free(pitem->pvalue);
		pitem->pvalue = NULL;
		pitem->pvalue = strdup(value);
		if (NULL == pitem->pvalue)
		{
			fprintf(stderr, "strdup fail:%s!\n", strerror(errno));
			return;
		}

		self->changed = 1;
		return;
	}

	//no item found. add new item
	pitem = (set_item*)calloc(1, sizeof(set_item));
	if (NULL == pitem)
	{
		fprintf(stderr, "calloc fail!\n");
		return;
	}

	pitem->pkey = strdup(name);
	if (NULL == pitem->pkey)
	{
		fprintf(stderr, "strdup fail:%s!\n", strerror(errno));
		return;
	}

	pitem->pvalue = strdup(value);
	if (NULL == pitem->pvalue)
	{
		fprintf(stderr, "strdup fail:%s!\n", strerror(errno));
		return;
	}

	if (pos_list == &self->setting_items)
	{
		//new section
		ret_val = setting_get_new_node(self, &p_section_item);
		if (0 != ret_val)
		{
			fprintf(stderr, "setting_get_new_node fail!\n");
			return;
		}

		p_section_item->pkey = strdup(section);
		if (NULL == p_section_item->pkey)
		{
			fprintf(stderr, "strdup fail:%s!\n", strerror(errno));
			return;
		}

		double_list_push_back(p_section_item->plist, pitem);
	}
	else
	{
		double_list_push_back(pos_list, pitem);
	}
	self->changed = 1;
}

void setting_set_int32(Settings* self, const char* section, const char* name, int32_t value)
{
	char buff[64] = { 0 };
	i32toa(value, buff);
	setting_set_string(self, section, name, buff);
}


void setting_set_int64(Settings* self, const char* section, const char* name, int64_t value)
{
	char buff[64] = { 0 };
	i64toa(value, buff);
	setting_set_string(self, section, name, buff);
}

int32_t setting_get_int32(Settings* self, const char* section, const char* name, int32_t default_value)
{
	const char *str = setting_get_string(self, section, name, NULL);
	if (str == NULL){
		return default_value;
	}
	else{
		return atoi32(str);
	}
}


int64_t setting_get_int64(Settings* self, const char* section, const char* name, int64_t default_value)
{
	const char *str = setting_get_string(self, section, name, NULL);
	if (str == NULL){
		return default_value;
	}
	else{
		return atoi64(str);
	}
}


const char* setting_get_string(Settings* self, const char* section, const char* name, const char* default_value)
{
	double_list_t*		plist = NULL;
	set_item* pitem = (set_item*)setting_get_set_item_pos(self, section, name, &plist);
	if (NULL != pitem)
	{
		return (const char*)pitem->pvalue;
	}

	return default_value;
}

void* setting_get_set_item_pos(Settings* self, const char* section, const char* key, double_list_t** pos_list)
{
	double_list_iterator_t	iter, end, sub_list_iter, sub_list_end;
	set_item*		pitem = NULL;

	iter = double_list_begin(&self->setting_items);
	end = double_list_end(&self->setting_items);
	*pos_list = NULL;

	for (; !double_list_iterator_equal(iter, end); double_list_iterator_next(&iter))
	{
		pitem = (set_item*)((double_list_node_t*)iter.list_data)->_data;
		if (0 != strcmp(pitem->pkey, section))//check section items
		{
			continue;
		}

		*pos_list = pitem->plist;
		//search config items
		sub_list_iter = double_list_begin(pitem->plist);
		sub_list_end = double_list_end(pitem->plist);
		for (; !double_list_iterator_equal(sub_list_iter, sub_list_end); double_list_iterator_next(&sub_list_iter))
		{
			pitem = (set_item*)((double_list_node_t*)sub_list_iter.list_data)->_data;
			if (0 == strcmp(key, pitem->pkey))
			{
				return pitem;
			}
		}
	}

	if (NULL == *pos_list)//new section
	{
		*pos_list = &self->setting_items;
	}
	return NULL;
}

int32_t setting_get_new_node(Settings* self, set_item** pitem)
{
	*pitem = calloc(1, sizeof(set_item));
	if (NULL == *pitem)
	{
		fprintf(stderr, "calloc fail:%s!\n", strerror(errno));
		return ERR_ETM_OUT_OF_MEMORY;
	}

	double_list_push_back(&self->setting_items, *pitem);

	(*pitem)->plist = calloc(1, sizeof(double_list_t));
	if (NULL == (*pitem)->plist)
	{
		fprintf(stderr, "calloc fail:%s!\n", strerror(errno));
		return ERR_ETM_OUT_OF_MEMORY;
	}

	return 0;
}

