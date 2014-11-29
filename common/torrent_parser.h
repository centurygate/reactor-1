/*******************************************************************************
 * File Name:   torrent_parser.h
 * Author:      WANG CHANGQING
 * Date:        2011-07-11
 * Notes:       Used to parse torrent.
 ******************************************************************************/
#ifndef	___TORRENT_PARSER__H___
#define	___TORRENT_PARSER__H___

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "sha1.h"

typedef struct tag_torrent_sub_file_info {
	unsigned _file_index;
	char *_file_name;
	unsigned _file_name_len;
	char *_file_path;
	unsigned _file_path_len;
	int64_t _file_offset;
	int64_t _file_size;
	struct tag_torrent_sub_file_info *_p_next;
} TorrentSubFileInfo;

typedef enum __tag_bencoding_item_type {
	BENCODING_ITINTEGER,
	BENCODING_ITNEGINT,
	BENCODING_ITSTRING,
	BENCODING_ITLIST,
	BENCODING_ITDICT,
} bencoding_item_type;

typedef enum __tag_tp_parsing_state {
	dict_waiting_prefix,
	dict_waiting_key,
	dict_waiting_val,
	dict_key_parsing,
	dict_val_parsing,
	string_parsing_len,
	string_parsing_buf,
} tp_parsing_state;

typedef struct __tag_bencoding_item_base {
	bencoding_item_type _type;
	tp_parsing_state _state;
	unsigned _len;
	struct __tag_bencoding_item_base *_p_next;
	struct __tag_bencoding_item_base *_p_parent;
} bencoding_item_base;

typedef struct __tag_bencoding_string {
	bencoding_item_base _base;
	char *_str;
	unsigned _valid_len;
} bencoding_string;

typedef struct __tag_bencoding_dict {
	bencoding_item_base _base;
	bencoding_string *_key;
	bencoding_item_base *_item;
} bencoding_dict;

typedef struct __tag_bencoding_list {
	bencoding_item_base _base;
	bencoding_item_base *_item;
} bencoding_list;

typedef struct __tag_bencoding_integer {
	bencoding_item_base _base;
	long long _value;
} bencoding_int;

typedef struct __tag_tp_parsing_related {
	bencoding_dict _torrent_dict;
	bencoding_item_base *_p_item;
	ctx_sha1 _sha1_ctx;
	uint64_t _torrent_size;
	uint64_t _bytes_parsed;
	uint8_t _update_info_sha1;
} tp_parsing_related;

typedef struct tag_torrent_parser {
	tp_parsing_related parsing_related; //It's discouraged to access parsing_related.

	bencoding_string *_ann_list;

	TorrentSubFileInfo* _file_list;
	TorrentSubFileInfo* _list_tail;
	uint32_t _file_num;
	uint64_t _file_total_size;
	uint8_t _info_hash[SHA1_SIZE];
	uint8_t _info_hash_valid;

	char *_title_name;
	int _title_name_len;

	char *src_encoding;
	char *dst_encoding;
	
	char _is_dir_organized;
	long long _piece_length;
	uint8_t* _piece_hash_ptr;
	uint32_t _piece_hash_len;
} TorrentParser;


/*******************************************************************************
 * User interface of torrent_parser_module.
 */

int32_t parse_torrent_from_memory(TorrentParser *torrent,const char *dst_encoding, const char *data,uint32_t length);
int32_t parse_torrent(TorrentParser *torrent, const char *path,const char *dst_encoding);

void tp_torrent_init(TorrentParser *p_torrent);
int parse_torrent_part(TorrentParser *p_torrent, const char * src, size_t len);
int tp_finish(TorrentParser *p_torrent);
void tp_torrent_destroy(TorrentParser *p_torrent);

#endif	//___TORRENT_PARSER__H___
