#ifndef MSG_PROCESSOR_20130820
#define MSG_PROCESSOR_20130820

#include <stdint.h>

struct socket_wrapper;
typedef void (*connect_handler_t)(struct socket_wrapper* p_conn);
typedef void (*close_handler_t)(struct socket_wrapper* p_conn);
typedef void (*error_handler_t)(struct socket_wrapper* p_conn, int32_t errcode);
typedef void (*msg_handler_t)(struct socket_wrapper* p_conn, char *msg, uint32_t msg_len);

typedef struct msg_processor {
		close_handler_t		close_handler;
		error_handler_t		error_handler;
		msg_handler_t		msg_handler;
		connect_handler_t	connect_handler;
} msg_processor_t;

int32_t msg_processor_initialize(msg_processor_t* self);
int32_t msg_processor_finalize(msg_processor_t* self);
#endif //MSG_PROCESSOR_20130820
