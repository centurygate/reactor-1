/**
# -*- coding:UTF-8 -*-
*/

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "define.h"
#include "double_list.h"
#include <stdint.h>

typedef struct s_settings_t
{
	char*	file_name;
	double_list_t	setting_items;
	int8_t	changed;
} Settings;

typedef struct setting_item
{
	char*	pkey;
	char*	pvalue;
	char*	pnote;
	double_list_t*	plist;
}set_item;

int32_t setting_initialize(Settings* self);
void setting_finalize(Settings* self);
int32_t setting_load_cfg(Settings* self, const char* cfg_file);
int32_t setting_load_cfg_from_memory(Settings* self, const char* buffer, uint32_t buf_len);
int32_t setting_flush(Settings* self);
int32_t setting_refresh(Settings* self);

const char* setting_get_string(Settings* self, const char* section, const char* name, const char* default_value); 
int32_t setting_get_int32(Settings* self, const char* section, const char* name, int32_t default_value);
int64_t setting_get_int64(Settings* self, const char* section, const char* name, int64_t default_value);

void setting_set_string(Settings* self, const char* section, const char* name, const char* value); 
void setting_set_int32(Settings* self, const char* section, const char* name, int32_t value);
void setting_set_int64(Settings* self, const char* section, const char* name, int64_t value);

#endif
