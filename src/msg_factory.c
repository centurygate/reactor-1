#include <stdlib.h>
#include <ctype.h>
#include "msg_factory.h"
#include "string_utility.h"
#include "error_code.h"

#define tbl_hexvalue \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x00\x00\x00\x00\x00\x00"\
	"\x00\x0a\x0b\x0c\x0d\x0e\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x0a\x0b\x0c\x0d\x0e\x0f\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"\
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

#define get_hexvalue(v) tbl_hexvalue[(uint8_t)(v)];

#define hex2char(p) (get_hexvalue((*p))<<4) | get_hexvalue((p)[1]);

int32_t msg_factory_initialize(msg_factory_t* self)
{
	self->decode = NULL;
	self->encode = NULL;
	self->validator = msg_validator_http;
	return 0;
}
int32_t msg_factory_finalize(msg_factory_t* self)
{
	return 0;
}
static int32_t _msg_validator_http_only_header(const char *msg, uint32_t msg_len) {
	const char* header_end = "\r\n\r\n";
	int32_t header_end_strlen = 4;
	int32_t i;
	int32_t header_len = 0;

	if(msg_len < header_end_strlen) {
		return header_end_strlen;
	}

	i = 0;
	do {
		if(memcmp(msg + i, header_end, header_end_strlen) == 0) {
			header_len = i + header_end_strlen;
			break;
		}
		i++;
	} while(i < msg_len - header_end_strlen + 1);

	if (i == msg_len - header_end_strlen + 1)
		return i + 512;
	return header_len;
}

static int32_t _msg_validator_http_chunked(const char *msg, uint32_t msg_len) {
	const char* header_end = "\r\n\r\n";
	int32_t header_end_strlen = 4;
	int32_t i;
	int32_t block_len = 0;
	int32_t parse_stat = 0; // 0-reading chunk length
							// 1-skipping \r\n
							// 3-skipping contents

	if(msg_len < header_end_strlen) {
		return header_end_strlen;
	}

	i = 0;
	do {
		if (memcmp(msg + i, header_end, header_end_strlen) == 0) {
			i += header_end_strlen;
			break;
		}
		i++;
	} while(i < msg_len - header_end_strlen + 1);

	while(i < msg_len) {
		if( (parse_stat == 3 || parse_stat == 0) 
			&& msg[i] >= '0' && msg[i] <= '9') {
			parse_stat = 0;
			block_len *= 0x10;
			block_len += msg[i] - '0';
			i++;
			continue;
		}
		if((parse_stat == 3 || parse_stat == 0) 
			&& (msg[i] | 0x20)  >= 'a' && (msg[i] | 0x20) <= 'f') {
			parse_stat = 0;
			block_len *= 0x10;
			block_len += ( (msg[i] | 0x20) - 'a' + 10);
			i++;
			continue;
		}
		if(parse_stat == 0) {
			parse_stat = 1;
			i += 2; // skipping \r\n
		}
		parse_stat = 3;
		if(block_len == 0) {
			break;
		}
		i += block_len + 2;
		block_len = 0;
	}

	return i + 2;
}

static int32_t _msg_validator_http_with_content_length(const char *msg, uint32_t msg_len) {
	const char* header_end = "\r\n\r\n";
	const char* label_content_len = "Content-Length";
	int32_t len, body_len = 0;
	int32_t i;
	const char *p_header_tail = NULL;
	
	len = strlen(header_end);

	if(msg_len < len) {
		return len;
	}

	i = 0;
	do {
		if(memcmp(msg + i, header_end, len) == 0) {
			p_header_tail = msg + i + len;
			break;
		}
		i++;
	} while(i < msg_len - len + 1);

	if(p_header_tail == NULL) {
		// try to recieve a complete http header
		return msg_len + 64;
	}
	// http header is complete
	len = strlen(label_content_len);
	i = 0;
	while(i < msg_len - len + 1) {
		if(strnicmp(msg + i, label_content_len, len) == 0) {
			// "Content-Length" found
			int32_t j = 0;
			int32_t num = 0;
			const char *t = msg + i + len;
			while(t[j] !='\r' && !isdigit(t[j]))j++;
			if(t[j] =='\r') {
				return EMFINVALIDMSG;
			}
			while(isdigit(t[j])) {
				num *= 10;
				num += t[j] - '0';
				j++;
			}
			body_len = num;
			break;
		}
		i++; 
	}
	
	if(i == msg_len - len + 1) {
		return EMFINVALIDMSG;
	}

	return body_len + (p_header_tail - msg);
}

int32_t msg_validator_http(const char *msg, uint32_t msg_len) {
	int32_t ret;
	
	if(msg_len < 4) {
		return 64;
	}
	
	if( msg[0] == 'H' &&
		msg[1] == 'T' &&
		msg[2] == 'T' &&
		msg[3] == 'P') {
		// HTTP Response
		ret = _msg_validator_http_with_content_length(msg, msg_len);
		if(ret < 0) {
			ret = _msg_validator_http_chunked(msg, msg_len);
		}
		return ret;
	}

	if( msg[0] == 'G' &&
		msg[1] == 'E' &&
		msg[2] == 'T') {
		ret = _msg_validator_http_only_header(msg, msg_len);
		return ret;
	}

	if( msg[0] == 'P' &&
		msg[1] == 'O' &&
		msg[2] == 'S' &&
		msg[3] == 'T') {
		ret = _msg_validator_http_with_content_length(msg, msg_len);
		return ret;
	}
	
	return EMFINVALIDMSG;
}

