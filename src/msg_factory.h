#ifndef _MSG_FACTORY_20130817
#define _MSG_FACTORY_20130817

#include "define.h"

typedef int32_t (*msg_decode_t)(const char *msg_in, uint32_t msg_in_len, char **p_msg_out);
typedef int32_t (*msg_encode_t)(const char *msg_in, uint32_t msg_in_len, char **p_msg_out);
typedef int32_t (*msg_validator_t)(const char *msg, uint32_t msg_len);

typedef struct msg_factory {
	msg_decode_t 		decode;
	msg_encode_t 		encode;
	msg_validator_t		validator;
} msg_factory_t;

int32_t msg_factory_initialize(msg_factory_t* self);
int32_t msg_factory_finalize(msg_factory_t* self);
int32_t msg_validator_http(const char *msg, uint32_t msg_len);

#endif //_MSG_FACTORY_20130817
