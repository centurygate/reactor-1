/*******************************************************************************
 * File Name:   hp_torrent_parser.h
 * Author:      WANG CHANGQING
 * Date:        2011-07-11
 * Notes:       Used to parse torrent.
 ******************************************************************************/

#include "torrent_parser.h"
#include "error_code.h"
#include "encoding_conversion.h"
#include "string_utility.h"
#include "fs_utility.h"
#include <stdio.h>
#include <assert.h>

static const int BENCODING_STRING_MAX_LEN = 64 * 1024 * 1024;
static const int64_t TORRENT_PARSER_INVALID_FILE_SIZE = -1;

static int tp_store_dict(TorrentParser *p_torrent, bencoding_dict* p_dict);
static int tp_store_list(TorrentParser *p_torrent, bencoding_list* p_list);

static TorrentSubFileInfo* tp_find_incompleted_file(TorrentParser* p_torrent);

static int tp_add_announce(TorrentParser *p_torrent, bencoding_string *p_ann);
static bencoding_int* bencoding_integer_create(bencoding_item_base *p_parent);
static bencoding_string* bencoding_string_create(bencoding_item_base *p_parent);
static bencoding_list* bencoding_list_create(bencoding_item_base *p_parent);
static bencoding_dict* bencoding_dict_create(bencoding_item_base *p_parent);

static void bencoding_dict_destroy(bencoding_dict *p_dict);
static void bencoding_list_destroy(bencoding_list *p_list);
static void bencoding_string_destroy(bencoding_string *p_string);

static int bencoding_list_add_item(bencoding_list *p_list, bencoding_item_base *p_item);

static TorrentSubFileInfo* tp_torrent_referred_file_create(uint32_t index);

int32_t parse_torrent_from_memory(TorrentParser *torrent,const char *dst_encoding, const char *data,uint32_t length)
{
	const int TORRENT_PARSER_PRESUME_TORRENT_MIN_LEN = 2;

	tp_torrent_init(torrent);

	if(length < TORRENT_PARSER_PRESUME_TORRENT_MIN_LEN){
		return ERR_TORRENT_PARSER_BAD_TORRENT;
	}

	torrent->dst_encoding = (char *)malloc(strlen(dst_encoding)+1);
	if(torrent->dst_encoding == NULL){
		return ERR_ETM_OUT_OF_MEMORY;
	}
	else{
		strcpy(torrent->dst_encoding,dst_encoding);
	}

	parse_torrent_part(torrent,data,length);
	return tp_finish(torrent);;
}


int32_t parse_torrent(TorrentParser *torrent, const char *torrent_path,const char *dst_encoding)
{
	const int TORRENT_PARSER_PRESUME_TORRENT_MIN_LEN = 2;
	const size_t PARSING_BUFF_LEN = 81920;
	char* parsing_buff;

	size_t read_len;
	size_t result_len;
	size_t bytes_not_parsed;

	FILE*  torrent_file;
	
	int32_t   result = 0;

	tp_torrent_init(torrent);
	
	parsing_buff = (char *)malloc(PARSING_BUFF_LEN);
	if(parsing_buff == NULL){
		return ERR_ETM_OUT_OF_MEMORY;
	}
	
	torrent_file = fopen(torrent_path,"r");
	if(torrent_file == NULL){
		free(parsing_buff);
		return ERR_TORRENT_PARSER_BAD_TORRENT;
	}	

	fseek(torrent_file,0L,SEEK_END);
	bytes_not_parsed = ftell(torrent_file);
	fseek(torrent_file,0L,SEEK_SET);

	if(bytes_not_parsed < TORRENT_PARSER_PRESUME_TORRENT_MIN_LEN){
		fclose(torrent_file);
		free(parsing_buff);
		return ERR_TORRENT_PARSER_INCOMPLETE;
	}

	torrent->dst_encoding = (char *)malloc(strlen(dst_encoding)+1);
	if(torrent->dst_encoding == NULL){
		fclose(torrent_file);
		free(parsing_buff);
		return ERR_ETM_OUT_OF_MEMORY;
	}
	else{
		strcpy(torrent->dst_encoding,dst_encoding);
	}

	while (bytes_not_parsed > 0) {
		read_len = bytes_not_parsed>=PARSING_BUFF_LEN?PARSING_BUFF_LEN:bytes_not_parsed;
		bytes_not_parsed-=read_len;

		result_len = fread(parsing_buff,1,read_len,torrent_file);

		if(result_len!=read_len){
			result = ERR_TORRENT_PARSER_BAD_READ;
			break;
		}

		result_len = parse_torrent_part(torrent,parsing_buff,read_len);
		if(result_len < 0){
			result = result_len;
			break;
		}
	}

	
	if (result == 0) {
		result = tp_finish(torrent);
	} 

	fclose(torrent_file);
	free(parsing_buff);

	return result;
}

int tp_finish(TorrentParser *torrent) 
{
	encoding_conversion_func_t tp_convert_encoding = NULL;
	TorrentSubFileInfo *p_sub_file;
	char *dst_buff;
	uint32_t dst_len = MAX_PATH;
	int32_t result;
	
	if (torrent->parsing_related._p_item != NULL) {
		return ERR_TORRENT_PARSER_BAD_TORRENT;
	}
	
	if (torrent->_piece_hash_ptr == NULL 
		|| torrent->_title_name == NULL
		|| torrent->_piece_length == 0) {
		return ERR_TORRENT_PARSER_INCOMPLETE;
	}

	if (torrent->_file_num == 0) {
		p_sub_file = tp_find_incompleted_file(torrent);
		if (p_sub_file == NULL) {
			return ERR_ETM_OUT_OF_MEMORY;
		}

		p_sub_file->_file_name = (char *)malloc(torrent->_title_name_len+1);
		p_sub_file->_file_path = (char *)malloc(1);

		if (p_sub_file->_file_name == NULL || p_sub_file->_file_path == NULL) {
			return ERR_ETM_OUT_OF_MEMORY;
		} else {
			p_sub_file->_file_size = torrent->_file_total_size;
			memcpy(p_sub_file->_file_name, torrent->_title_name,
					torrent->_title_name_len + 1);
			p_sub_file->_file_name_len = torrent->_title_name_len;
			p_sub_file->_file_path[0] = 0;
		}
	} 
	else 
	{
		p_sub_file = torrent->_list_tail;
		torrent->_file_total_size = 
			p_sub_file->_file_size + p_sub_file->_file_offset;
	}

	/* convert encoding */
	if (torrent->src_encoding != NULL
		&& stricmp(torrent->src_encoding,torrent->dst_encoding)==0){
		return 0;
	}

	if(torrent->src_encoding == NULL){
		if(stricmp(torrent->dst_encoding,UTF8_ENCODING)==0){
			tp_convert_encoding = to_utf8;
		}
		else if(stricmp(torrent->dst_encoding,GBK_ENCODING)==0){
			tp_convert_encoding = to_gbk;
		}
		else if(stricmp(torrent->dst_encoding,BIG5_ENCODING)==0){
			tp_convert_encoding = to_big5;
		}
	}
	else if(stricmp(torrent->src_encoding,UTF8_ENCODING)==0){
		if(stricmp(torrent->dst_encoding,GBK_ENCODING)==0){
			tp_convert_encoding = utf8_to_gbk;
		}
		else if(stricmp(torrent->dst_encoding,BIG5_ENCODING)==0){
			tp_convert_encoding = utf8_to_big5;
		}
	}
	else if(stricmp(torrent->src_encoding,GBK_ENCODING)==0){
		if(stricmp(torrent->dst_encoding,UTF8_ENCODING)==0){
			tp_convert_encoding = gbk_to_utf8;
		}
		else if(stricmp(torrent->dst_encoding,BIG5_ENCODING)==0){
			tp_convert_encoding = gbk_to_big5;
		}
	}
	else if(stricmp(torrent->src_encoding,BIG5_ENCODING)==0){
		if(stricmp(torrent->dst_encoding,UTF8_ENCODING)==0){
			tp_convert_encoding = big5_to_utf8;
		}
		else if(stricmp(torrent->dst_encoding,GBK_ENCODING)==0){
			tp_convert_encoding = big5_to_gbk;
		}
	}
	if(tp_convert_encoding == NULL) return 0;

	dst_buff =(char *)alloca(dst_len);
	//convert title name to destination encoding
	result = tp_convert_encoding(torrent->_title_name,torrent->_title_name_len,dst_buff,&dst_len);
	if(result == 0){
		torrent->_title_name = (char *)realloc(torrent->_title_name,dst_len+1);
		memcpy(torrent->_title_name,dst_buff,dst_len);
		torrent->_title_name_len = dst_len;
		torrent->_title_name[torrent->_title_name_len] = '\0';
	}
	else{
		//assert(0);
		//break;
	}

	for(p_sub_file = torrent->_file_list;p_sub_file != NULL;p_sub_file = p_sub_file->_p_next){
		if(p_sub_file->_file_path_len>0){
			dst_len = MAX_PATH;
			result = tp_convert_encoding(p_sub_file->_file_path,p_sub_file->_file_path_len,dst_buff,&dst_len);
			if(result == 0){
				p_sub_file->_file_path = (char *)realloc(p_sub_file->_file_path, dst_len+1);
				memcpy(p_sub_file->_file_path,dst_buff,dst_len);
				p_sub_file->_file_path_len = dst_len;
				p_sub_file->_file_path[p_sub_file->_file_path_len] = '\0';
			}
			else{
				//assert(0);
				//break;
			}
		}

		//convert file name to destination encoding
		dst_len = MAX_PATH;
		result = tp_convert_encoding(p_sub_file->_file_name,p_sub_file->_file_name_len,dst_buff,&dst_len);
		if(result == 0){
			p_sub_file->_file_name = (char *)realloc(p_sub_file->_file_name, dst_len+1);
			memcpy(p_sub_file->_file_name,dst_buff,dst_len);
			p_sub_file->_file_name_len = dst_len;
			p_sub_file->_file_name[p_sub_file->_file_name_len] = '\0';
		}
		else{
			//assert(0);
			//break;
		}
	}
	return 0; //result
}

/**
 * prerequisites: p_torrent != NULL &&p_dict != NULL && p_dict->_key != NULL && p_dict->_item != NULL
 */
int tp_store_dict(TorrentParser *p_torrent, bencoding_dict* p_dict) {
	const char* dict_key = p_dict->_key->_str;

	bencoding_item_base *p_item = p_dict->_item;
	int store_result = 0;

	if (p_item->_type == BENCODING_ITEM_INTEGER
			|| p_item->_type == BENCODING_ITEM_NEGINT) {
		bencoding_int *p_int = (bencoding_int *) p_item;
			
		if (p_dict->_base._p_parent == NULL)   
		{
			if (strcmp(dict_key, "piece length") == 0) {
				p_torrent->_piece_length = p_int->_value;
			}
			free(p_int);
			p_dict->_item = NULL;
			return 0;
		}

		//p_dict is not the root dictionary
		if (p_dict->_base._p_parent->_type == BENCODING_ITEM_LIST) {
			//p_dict's parent is a list
			bencoding_list *p_list =
					(bencoding_list *) p_dict->_base._p_parent;
			if (p_list->_base._p_parent->_type == BENCODING_ITEM_DICT) {
				//expecting p_list's parent is the dict referred by key "info" in the root dict
				bencoding_dict *p_info_dict =
						(bencoding_dict *) p_list->_base._p_parent;
				if (strcmp(p_info_dict->_key->_str, "files") == 0) {
					if (strcmp(dict_key, "length") == 0) {
						TorrentSubFileInfo* p_file =
								tp_find_incompleted_file(p_torrent);
						if (p_file == NULL) {
							store_result = -1;
						} else {
							p_file->_file_size = p_int->_value;
						}
					} else {
						//ignore the item
					}
				} else {
					//ignore the item.
				}
			} else {
				store_result = -1;
			}
		} 
		else 
		{
			//p_dict's parent is a BENCODING dictionary
			if (strcmp(dict_key, "piece length") == 0) {
				p_torrent->_piece_length = p_int->_value;
			}
			//expecting it's a single file torrent.
			if (p_dict->_base._p_parent->_p_parent == NULL
					&& strcmp(
							((bencoding_dict *) p_dict->_base._p_parent)->_key->_str,
							"info") == 0) {
				if (strcmp(dict_key, "length") == 0) {
					p_torrent->_file_total_size = p_int->_value;
				}
			}
		}
			free(p_int);
		
	} 
	else if (p_item->_type == BENCODING_ITEM_STRING) 
	{
		if (p_dict->_base._p_parent != NULL) {
			bencoding_string *p_str = (bencoding_string *) p_item;
			if (p_dict->_base._p_parent->_type == BENCODING_ITEM_LIST) {
				free(p_str->_str);
			} else {
				//p_dict's parent is a bencoding dictionary
				if (strcmp(dict_key, "name") == 0) {
					p_torrent->_title_name = p_str->_str;
					p_torrent->_title_name_len = p_str->_valid_len;
				} else if (strcmp(dict_key, "pieces") == 0) {
					p_torrent->_piece_hash_ptr = (uint8_t *) p_str->_str;
					p_torrent->_piece_hash_len = p_str->_valid_len;
				}
			}
			free(p_str);
		}
		else 
		{
			bencoding_string *p_str = (bencoding_string *) p_item;
			//p_dict is the root dict of the torrent
			if (strcmp(dict_key, "announce") == 0) {
				p_str->_base._p_next = NULL;
				tp_add_announce(p_torrent, p_str);
				p_str = NULL;
			} else if (strcmp(dict_key, "encoding") == 0) {
				p_torrent->src_encoding = p_str->_str;
				p_str->_str = NULL;
			} else {
			}

			if (p_str != NULL) {
				if(p_str->_str!=NULL)free(p_str->_str);
				free(p_str);
			}
		}
	} 
	else
	{
		free(p_item);
	} 

	free(p_dict->_key->_str);
	free(p_dict->_key);
	p_dict->_key = NULL;
	p_dict->_item = NULL;
	return store_result;
}

static int tp_store_path(TorrentParser *p_torrent, bencoding_list* p_list)
{
	bencoding_string   *p_str;
	TorrentSubFileInfo *p_file;

	int path_phase;
	int path_len;
	char *merged_path;

	p_file = tp_find_incompleted_file(p_torrent);

	if(p_file == NULL){
		return -1;
	}
	
	p_str = (bencoding_string *) p_list->_item;
	path_phase = 0;
	path_len = 0;
	
	while (p_str->_base._p_next != NULL) {
		path_len += p_str->_valid_len;
		++path_phase;
		p_str = (bencoding_string *) p_str->_base._p_next;
	}

	//store file name
	p_file->_file_name_len = p_str->_valid_len;
	p_file->_file_name = p_str->_str;
	p_str->_str = NULL;
	


	if(path_phase == 0){
		p_file->_file_path = (char *)malloc(1);
		p_file->_file_path_len = 0;
		p_file->_file_path[0] = '\0';
	}
	else if(path_phase == 1){
		p_str = (bencoding_string *) p_list->_item;
		p_file->_file_path = p_str->_str;
		p_str->_str = NULL;
		p_file->_file_path_len = p_str->_valid_len;
	}
	else{
		merged_path = (char *)malloc(path_len + path_phase);
		p_str = (bencoding_string *) p_list->_item;
		path_len = 0;
		
		while (p_str->_base._p_next != NULL) 
		{
			memcpy(merged_path+path_len,p_str->_str,p_str->_valid_len);
			path_len += p_str->_valid_len;
			merged_path[path_len++] = '/';
			p_str = (bencoding_string *) p_str->_base._p_next;
		}
		merged_path[--path_len]='\0';
		p_file->_file_path = merged_path;
		p_file->_file_path_len = path_len;
	}

	p_str = (bencoding_string *) p_list->_item;
	while (p_str->_base._p_next != NULL) {
		bencoding_string *p_del = p_str;
		p_str = (bencoding_string *) p_str->_base._p_next;
		if(p_del->_str != NULL){
			free(p_del->_str);
		}

		free(p_del);
	}

	return 0;
}
int tp_store_list(TorrentParser *p_torrent, bencoding_list* p_list) {
	int store_result = 0;

	if (p_list->_base._p_parent->_type == BENCODING_ITEM_LIST) 
	{
		bencoding_item_base *p_parent_list = p_list->_base._p_parent;
		bencoding_dict *p_dict = (bencoding_dict *) p_parent_list->_p_parent;
		
		if (p_parent_list->_p_parent->_type != BENCODING_ITEM_DICT
			||p_parent_list->_p_parent->_p_parent != NULL) 
		{
			bencoding_item_base *p_node = p_list->_item;
			while (p_node != NULL) 
			{
				bencoding_item_base *p_del = p_node;
				p_node = p_node->_p_next;
				if (p_del->_type == BENCODING_ITEM_STRING) {
					free(((bencoding_string *) p_del)->_str);
				}

				free(p_del);
			}
			p_list->_item = NULL;
			return -1;
		} 
		
		if (strcmp(p_dict->_key->_str, "announce-list") == 0) {
			bencoding_string *p_str = (bencoding_string *) p_list->_item;
			p_str->_base._p_next = NULL;
			tp_add_announce(p_torrent, p_str);
			p_list->_item = NULL;
		} 
		else 
		{
			//dht nodes is possible here.
			bencoding_item_base *p_node = p_list->_item;
			while (p_node != NULL) 
			{
				bencoding_item_base *p_del = p_node;
				p_node = p_node->_p_next;
				if (p_del->_type == BENCODING_ITEM_STRING) {
					free(((bencoding_string *) p_del)->_str);
				}

				free(p_del);
			}
			p_list->_item = NULL;
		}
			
	} 
	else 
	{
		//p_list's parent is a dictionary
		bencoding_dict *p_dict = (bencoding_dict *) p_list->_base._p_parent;
		//expecting path
		if (p_dict->_base._p_parent != NULL
				&& strcmp(p_dict->_key->_str, "path") == 0) 
		{
			tp_store_path(p_torrent,(bencoding_list *)p_dict->_item);
		} 
		else 
		{
			bencoding_item_base *p_node = p_list->_item;
			bencoding_item_base *p_del;

			while (p_node != NULL) {
				p_del = p_node;
				p_node = p_node->_p_next;

				if (p_del->_type == BENCODING_ITEM_STRING) {
					free(((bencoding_string *) p_del)->_str);
				}
				
				free(p_del);
			}
		}
	}

	p_list->_item = NULL;
	return store_result;
}

int parse_torrent_part(TorrentParser *p_torrent, const char * src,size_t len) {
	bencoding_item_base* p_item = p_torrent->parsing_related._p_item;
	size_t parse_cur = 0;

	long long sha1_start = -1;
	long long sha1_end = -1;

	int parsing_result = 0; //1 stands for illegal char in integer

	for (parse_cur = 0; parse_cur < len; ++parse_cur) {
		const char input_char = src[parse_cur];

		if (p_item == NULL) {
			break;
		} 
		else if (p_item->_type == BENCODING_ITEM_INTEGER || p_item->_type == BENCODING_ITEM_NEGINT) 
		{
			bencoding_int *p_int = (bencoding_int *) p_item;
			if (input_char >= '0' && input_char <= '9') 
			{
				++p_int->_base._len;
				p_int->_value = p_int->_value * 10 + input_char - '0';
			} 
			else if (input_char == '-') 
			{
				p_int->_base._type = BENCODING_ITEM_NEGINT;
			}
			else if (input_char == 'e') 
			{
				if (p_item->_type == BENCODING_ITEM_NEGINT)
					p_int->_value = 0 - p_int->_value;

				p_item = p_item->_p_parent;

				if (p_item->_type == BENCODING_ITEM_DICT) {
					p_item->_state = dict_waiting_key;
				}
			} 
			else {
				parsing_result = ERR_TORRENT_PARSER_BAD_TORRENT;
				break;
			}
		} 
		else if (p_item->_type == BENCODING_ITEM_STRING) 
		{
			bencoding_string *p_str = (bencoding_string *) p_item;
			if (p_item->_state == string_parsing_len) 
			{
				if (input_char >= '0' && input_char <= '9') 
				{
					p_item->_len = p_item->_len * 10 + input_char - '0';
				} 
				else if (input_char == ':') 
				{
					if (p_item->_len > BENCODING_STRING_MAX_LEN) 
					{
						parsing_result = ERR_TORRENT_PARSER_STRING_TOO_LONG;
						break;
					}

					p_item->_state = string_parsing_buf;
					p_str->_valid_len = 0;
					p_str->_str = (char *)malloc(p_str->_base._len + 1);
					
					if (p_str->_str == NULL) 
					{
						parsing_result = ERR_ETM_OUT_OF_MEMORY;
						break;
					}
				} 
				else 
				{
					parsing_result = ERR_TORRENT_PARSER_BAD_TORRENT;
					break;
				}
			} 
			else 
			{
				long long copy_len = p_str->_base._len - p_str->_valid_len;

				if (len - parse_cur >= copy_len) {
					memcpy(p_str->_str + p_str->_valid_len, src + parse_cur,
							copy_len);
					p_str->_valid_len = p_str->_base._len;
					parse_cur += copy_len - 1;
					p_torrent->parsing_related._bytes_parsed += copy_len
							- 1;
				} else {
					memcpy(p_str->_str + p_str->_valid_len, src + parse_cur,
							len - parse_cur);
					p_str->_valid_len += len - parse_cur;
					parse_cur = len - 1;
					p_torrent->parsing_related._bytes_parsed += len
							- parse_cur - 1;
				}
			}

			if (p_str->_valid_len == p_str->_base._len) 
			{
				p_str->_base._state = string_parsing_buf;
				p_str->_str[p_str->_valid_len] = '\0';
				p_item = p_item->_p_parent;

				if (p_item->_type == BENCODING_ITEM_DICT) {
					p_item->_state =
							(p_item->_state == dict_key_parsing) ?
									dict_waiting_val : dict_waiting_key;
				}
				/***
				 * Check info hash location,the BENCODING dictionary referred
				 *  by key "info" in the root dictionary of the torrent
				 */
				if (p_str->_valid_len == 4
						&& p_str->_base._p_parent->_type == BENCODING_ITEM_DICT
						&& p_str->_base._p_parent->_p_parent == NULL
						&& strncmp("info", p_str->_str, 4) == 0) {
					p_torrent->parsing_related._update_info_sha1 = 1;
					sha1_start = parse_cur + 1;
				}
			}
		} 
		else if (p_item->_type == BENCODING_ITEM_DICT) 
		{
			if (p_item->_state == dict_waiting_key) 
			{
				bencoding_dict *p_dict = (bencoding_dict *) p_item;
				if (p_dict->_key != NULL) 
				{
					tp_store_dict(p_torrent, p_dict);
				}
				
				if (input_char >= '0' && input_char <= '9') 
				{
					bencoding_string *p_str = bencoding_string_create(p_item);
					if (p_str == NULL) {
						parsing_result = ERR_ETM_OUT_OF_MEMORY;
						break;
					}
					p_str->_base._len = input_char - '0';
					p_dict->_base._state = dict_key_parsing;
					p_dict->_key = p_str;
					p_item = (bencoding_item_base *) p_str;
				} 
				else if (input_char == 'e') 
				{
					p_item = p_item->_p_parent;
					if (p_item == NULL) 
					{
						++parse_cur;
						++p_torrent->parsing_related._bytes_parsed;
						break;
					} 
					else if (p_item->_type == BENCODING_ITEM_DICT) 
					{
						p_item->_state = dict_waiting_key;
					}
					
					if (p_item != NULL && p_item->_p_parent == NULL 
						&& p_item->_type == BENCODING_ITEM_DICT
						&& ((bencoding_dict *)p_item)->_item->_type == BENCODING_ITEM_DICT
						&& strncmp("info", ((bencoding_dict *)p_item)->_key->_str, 4) == 0) 
					{
						sha1_end = parse_cur + 1;
					}
				} 
				else 
				{
					parsing_result = ERR_TORRENT_PARSER_BAD_TORRENT;
					break;
				}
			} 
			else if (p_item->_state == dict_waiting_val) 
			{
				bencoding_dict *p_dict = (bencoding_dict *) p_item;
				if (input_char >= '0' && input_char <= '9') 
				{
					bencoding_string *p_str = bencoding_string_create(p_item);
					if (p_str == NULL) {
						parsing_result = ERR_ETM_OUT_OF_MEMORY;
						break;
					}
					p_str->_base._len = input_char - '0';
					p_item->_state = dict_val_parsing;
					p_dict->_item = (bencoding_item_base *) p_str;
					p_item = (bencoding_item_base *) p_str;
				} 
				else if (input_char == 'd') 
				{
					bencoding_dict *p_new_dict = bencoding_dict_create(p_item);
					if (p_new_dict == NULL) {
						parsing_result = ERR_ETM_OUT_OF_MEMORY;
						break;
					}
					p_item->_state = dict_val_parsing;
					((bencoding_dict *) p_item)->_item =
							(bencoding_item_base *) p_new_dict;
					p_item = (bencoding_item_base *) p_new_dict;
				} 
				else if (input_char == 'l') 
				{
					bencoding_list *p_list = bencoding_list_create(p_item);
					if (p_list == NULL) {
						parsing_result = ERR_ETM_OUT_OF_MEMORY;
						break;
					}
					p_item->_state = dict_val_parsing;
					((bencoding_dict *) p_item)->_item =
							(bencoding_item_base *) p_list;
					p_item = (bencoding_item_base *) p_list;
				} 
				else if (input_char == 'i') 
				{
					bencoding_int *p_int = bencoding_integer_create(p_item);
					if (p_int == NULL) {
						parsing_result = ERR_ETM_OUT_OF_MEMORY;
						break;
					}
					p_item->_state = dict_val_parsing;
					((bencoding_dict *) p_item)->_item =
							(bencoding_item_base *) p_int;
					p_item = (bencoding_item_base *) p_int;
				} 
				else 
				{
					parsing_result = ERR_TORRENT_PARSER_BAD_TORRENT;
					break;
				}
			}
			else 
			{
				//p_item->_state is HPTP_STATE_WAIT_DICT_FEATURE
				if (input_char == 'd') {
					p_item->_state = dict_waiting_key;
				} else {
					parsing_result = ERR_TORRENT_PARSER_BAD_TORRENT;
					break;
				}
			}
		} 
		else if (p_item->_type == BENCODING_ITEM_LIST) 
		{
			bencoding_list *p_list = (bencoding_list *) p_item;
			if (p_list->_item != NULL
					&& p_list->_item->_type != BENCODING_ITEM_STRING) 
			{
				tp_store_list(p_torrent, p_list);
			}
			if (input_char >= '0' && input_char <= '9') 
			{
				bencoding_string *p_str = bencoding_string_create(p_item);
				if (p_str == NULL) {
					parsing_result = ERR_ETM_OUT_OF_MEMORY;
					break;
				}

				p_str->_base._len = input_char - '0';

				bencoding_list_add_item(p_list, (bencoding_item_base *) p_str);
				p_item = (bencoding_item_base *) p_str;
			} 
			else if (input_char == 'd') 
			{
				bencoding_dict *p_dict = bencoding_dict_create(p_item);
				if (p_dict == NULL) {
					parsing_result = ERR_ETM_OUT_OF_MEMORY;
					break;
				}
				p_list->_item = (bencoding_item_base *) p_dict;
				p_item = (bencoding_item_base *) p_dict;
			} 
			else if (input_char == 'i') 
			{
				bencoding_int *p_new = bencoding_integer_create(p_item);
				if (p_new == NULL) {
					parsing_result = ERR_ETM_OUT_OF_MEMORY;
					break;
				}
				bencoding_list_add_item(p_list, (bencoding_item_base *) p_new);
				p_item = (bencoding_item_base *) p_new;
			}
			else if (input_char == 'l') 
			{
				bencoding_list *p_new = bencoding_list_create(p_item);
				if (p_new == NULL) {
					parsing_result = ERR_ETM_OUT_OF_MEMORY;
					break;
				}
				p_list->_item = (bencoding_item_base *) p_new;
				p_item = (bencoding_item_base *) p_new;
			} 
			else if (input_char == 'e') 
			{
				if (p_list->_item != NULL) 
				{
					tp_store_list(p_torrent, p_list);
				}

				p_item = p_item->_p_parent;
				if (p_item->_type == BENCODING_ITEM_DICT) 
				{
					p_item->_state = dict_waiting_key;
				}
			} 
			else 
			{
				parsing_result = ERR_TORRENT_PARSER_BAD_TORRENT;
				break;
			}
		}
		++p_torrent->parsing_related._bytes_parsed;
	}

	p_torrent->parsing_related._p_item = p_item;


	if (parsing_result == 0
			&& p_torrent->parsing_related._update_info_sha1 == 1) 
	{
		if (sha1_start >= 0 && sha1_end >= 0) {
			sha1_update(&p_torrent->parsing_related._sha1_ctx,
					(unsigned char *) src + sha1_start, sha1_end - sha1_start);
			sha1_finish(&p_torrent->parsing_related._sha1_ctx,
					p_torrent->_info_hash);
			p_torrent->_info_hash_valid = 1;
		} else if (sha1_start >= 0) {
			sha1_update(&p_torrent->parsing_related._sha1_ctx,
					(unsigned char *) src + sha1_start, len - sha1_start);
		} else if (sha1_end >= 0) {
			sha1_update(&p_torrent->parsing_related._sha1_ctx,
					(unsigned char *) src, sha1_end);
			sha1_finish(&p_torrent->parsing_related._sha1_ctx,
					p_torrent->_info_hash);
			p_torrent->_info_hash_valid = 1;
		} else {
			sha1_update(&p_torrent->parsing_related._sha1_ctx,
					(unsigned char *) src, len);
		}
	} 

	return parse_cur;
}

/**
 * return : null stands for error occurred
 */
TorrentSubFileInfo* tp_find_incompleted_file(TorrentParser* torrent) {
	TorrentSubFileInfo *file = NULL;
	if (torrent->_file_list == NULL) {
		file = tp_torrent_referred_file_create(0);

		if (file != NULL){
			torrent->_list_tail = file;
			torrent->_file_list = file;
			++torrent->_file_num;
		}
	} 
	else if (torrent->_list_tail->_file_name == NULL
				|| torrent->_list_tail->_file_path == NULL
				|| torrent->_list_tail->_file_size == TORRENT_PARSER_INVALID_FILE_SIZE) {
			file = torrent->_list_tail;
	}
	else 
	{
		file = tp_torrent_referred_file_create(
				torrent->_list_tail->_file_index + 1);
		if (file != NULL) {
			file->_file_offset = torrent->_list_tail->_file_offset
					+ torrent->_list_tail->_file_size;
			torrent->_list_tail->_p_next = file;
			torrent->_list_tail = file;
			++torrent->_file_num;
		}
	}

	return file;
}


bencoding_int* bencoding_integer_create(bencoding_item_base *p_parent) 
{
	bencoding_int* p_int = (bencoding_int *)malloc(sizeof(bencoding_int));
	
	if (p_int != NULL) {
		p_int->_base._type = BENCODING_ITEM_INTEGER;
		p_int->_base._p_parent = p_parent;
		p_int->_value = 0;
		p_int->_base._len = 0;
		p_int->_base._p_next = NULL;
	}

	return p_int;
}

bencoding_string* bencoding_string_create(bencoding_item_base *p_parent)
{
	bencoding_string* p_str = (bencoding_string *)malloc(sizeof(bencoding_string));

	if (p_str != NULL) {
		p_str->_base._type = BENCODING_ITEM_STRING;
		p_str->_base._p_parent = p_parent;
		p_str->_base._state = string_parsing_len;
		p_str->_base._len = 0;
		p_str->_str = NULL;
		p_str->_valid_len = 0;
		p_str->_base._p_next = NULL;
	}

	return p_str;
}

bencoding_list* bencoding_list_create(bencoding_item_base *p_parent) 
{
	bencoding_list* p_list = (bencoding_list *)malloc(sizeof(bencoding_list));

	if (p_list != NULL) {
		p_list->_base._type = BENCODING_ITEM_LIST;
		p_list->_base._p_parent = p_parent;
		p_list->_base._len = 0;
		p_list->_item = NULL;
		p_list->_base._p_next = NULL;
	}

	return p_list;
}

bencoding_dict* bencoding_dict_create(bencoding_item_base *p_parent) 
{
	bencoding_dict* p_dict = (bencoding_dict *)malloc(sizeof(bencoding_dict));

	if (p_dict != NULL) {
		p_dict->_base._type = BENCODING_ITEM_DICT;
		p_dict->_base._p_parent = p_parent;
		p_dict->_base._state = dict_waiting_key;
		p_dict->_base._len = 0;
		p_dict->_key = NULL;
		p_dict->_item = NULL;
		p_dict->_base._p_next = NULL;
	}

	return p_dict;
}
void bencoding_string_destroy(bencoding_string *p_str) {
	if (p_str->_str != NULL) {
		free(p_str->_str);
	}
	free(p_str);
}

void bencoding_list_destroy(bencoding_list *p_list) 
{
	bencoding_item_base *p_node = p_list->_item;
	while (p_node != NULL) {
		bencoding_item_base *p_del = p_node;
		p_node = p_node->_p_next;
		if (p_del->_type == BENCODING_ITEM_NEGINT
				|| p_del->_type == BENCODING_ITEM_INTEGER) {
			free(p_del);
		} else if (p_del->_type == BENCODING_ITEM_STRING) {
			bencoding_string_destroy((bencoding_string *)p_del);
		} else if (p_del->_type == BENCODING_ITEM_LIST) {
			bencoding_list_destroy((bencoding_list *)p_del);
			free(p_del);
		} else {
			bencoding_dict_destroy((bencoding_dict *)p_del);
			free(p_del);
		}
	}
}

void bencoding_dict_destroy(bencoding_dict *p_dict) 
{
	if (p_dict->_key != NULL) {
		bencoding_string_destroy(p_dict->_key);
	}

	if (p_dict->_item != NULL) 
	{
		if (p_dict->_item->_type == BENCODING_ITEM_INTEGER
				|| p_dict->_item->_type == BENCODING_ITEM_NEGINT) {
			free(p_dict->_item);
		} else if (p_dict->_item->_type == BENCODING_ITEM_STRING) {
			bencoding_string_destroy((bencoding_string *)p_dict->_item);
		} else if (p_dict->_item->_type == BENCODING_ITEM_LIST) {
			bencoding_list_destroy((bencoding_list *)p_dict->_item);
			free(p_dict->_item);
		} else {
			bencoding_dict_destroy((bencoding_dict *)p_dict->_item);
			free(p_dict->_item);
		}
	}
}

int tp_parsing_related_add_file(TorrentParser *p_torrent, TorrentSubFileInfo *p_file) 
{
	p_file->_p_next = NULL;

	if (p_torrent->_file_list == NULL) 
	{
		p_torrent->_file_list = p_file;
		p_torrent->_list_tail = p_file;
	} 
	else {
		p_torrent->_list_tail->_p_next = p_file;
		p_torrent->_list_tail = p_file;
	}

	++p_torrent->_file_num;
	return 0;
}

TorrentSubFileInfo* tp_torrent_referred_file_create(uint32_t index) 
{
	TorrentSubFileInfo* p_file = (TorrentSubFileInfo *)malloc(sizeof(TorrentSubFileInfo));
	if (p_file != NULL) {
		p_file->_file_index = index;

		p_file->_p_next = NULL;

		p_file->_file_offset = 0;
		p_file->_file_size = TORRENT_PARSER_INVALID_FILE_SIZE;

		p_file->_file_name = NULL;
		p_file->_file_path = NULL;

		p_file->_file_name_len = 0;
		p_file->_file_path_len = 0;
	}
	return p_file;
}

int bencoding_list_add_item(bencoding_list *p_list, bencoding_item_base *p_item) 
{
	if (p_list->_item == NULL) {
		p_list->_item = p_item;
	} else {
		bencoding_item_base *p_node = p_list->_item;
		while (p_node->_p_next)
			p_node = p_node->_p_next;
		p_node->_p_next = p_item;
	}
	return 1;
}

int tp_add_announce(TorrentParser *p_torrent, bencoding_string *p_ann) 
{
	p_ann->_base._p_next = NULL;
	if (p_torrent->_ann_list == NULL) {
		p_torrent->_ann_list = p_ann;
	} else {
		bencoding_string *p_node = p_torrent->_ann_list;
		while (p_node->_base._p_next)
			p_node = (bencoding_string *) p_node->_base._p_next;
		p_node->_base._p_next = (bencoding_item_base *) p_ann;
	}
	return 1;
}

void tp_torrent_init(TorrentParser *torrent) {
	torrent->parsing_related._torrent_dict._key = NULL;
	torrent->parsing_related._torrent_dict._item = NULL;
	torrent->parsing_related._torrent_dict._base._len = 0;
	torrent->parsing_related._torrent_dict._base._p_parent = NULL;
	torrent->parsing_related._torrent_dict._base._state =
				dict_waiting_prefix;
	torrent->parsing_related._torrent_dict._base._type =
				BENCODING_ITEM_DICT;
	torrent->parsing_related._p_item =
				(bencoding_item_base *) &torrent->parsing_related._torrent_dict;
	torrent->parsing_related._torrent_size = 0;
	torrent->parsing_related._bytes_parsed = 0;

	torrent->_title_name_len = 0;
	torrent->_title_name = NULL;
	torrent->parsing_related._update_info_sha1 = 0;

	torrent->src_encoding = NULL;
	torrent->dst_encoding = NULL;

	torrent->_ann_list = NULL;
	torrent->_file_list = NULL;
	torrent->_list_tail = NULL;

	torrent->_info_hash_valid = 0;
	torrent->_piece_hash_len = 0;
	torrent->_piece_hash_ptr = NULL;
	torrent->_piece_length = 0;
	torrent->_file_num = 0;

	sha1_initialize(&torrent->parsing_related._sha1_ctx);
}

void tp_torrent_destroy(TorrentParser *p_torrent) {
	if (p_torrent->_title_name != NULL) {
		free(p_torrent->_title_name);
		p_torrent->_title_name = NULL;
		p_torrent->_title_name_len = 0;
	}
	
	if (p_torrent->_piece_hash_ptr != NULL) {
		free(p_torrent->_piece_hash_ptr);
		p_torrent->_piece_hash_ptr = NULL;
		p_torrent->_piece_hash_len = 0;
	}

	p_torrent->_file_num = 0;
	p_torrent->_file_total_size = TORRENT_PARSER_INVALID_FILE_SIZE;
	p_torrent->_info_hash_valid = 0;
	p_torrent->_is_dir_organized = 0;
	p_torrent->_piece_length = 0;

	if(p_torrent->src_encoding!=NULL){
		free(p_torrent->src_encoding);
		p_torrent->src_encoding = NULL;
	}
	
	if(p_torrent->dst_encoding!=NULL){
		free(p_torrent->dst_encoding);
		p_torrent->dst_encoding= NULL;
	}

	bencoding_string *p_str_del;

	while (p_torrent->_ann_list != NULL) {
		p_str_del = p_torrent->_ann_list;
		p_torrent->_ann_list = (bencoding_string *)
			p_torrent->_ann_list->_base._p_next;

		free(p_str_del->_str);
		free(p_str_del);
	}
	p_torrent->_ann_list = NULL;


	TorrentSubFileInfo *p_subfile_del;
	while (p_torrent->_file_list != NULL) {
		p_subfile_del = p_torrent->_file_list;
		p_torrent->_file_list = 
			p_torrent->_file_list->_p_next;
		
		if(p_subfile_del->_file_name!=NULL){
			free(p_subfile_del->_file_name);
		}
		if(p_subfile_del->_file_path!=NULL){
			free(p_subfile_del->_file_path);
		}
		free(p_subfile_del);
	}

	p_torrent->_list_tail = NULL;
	p_torrent->_file_list = NULL;

	p_torrent->parsing_related._update_info_sha1 = 0;
	p_torrent->parsing_related._torrent_size = 0;
	p_torrent->parsing_related._bytes_parsed = 0;
	p_torrent->parsing_related._p_item = NULL;
	
	bencoding_dict_destroy(&p_torrent->parsing_related._torrent_dict);
}

