#include <stdlib.h>
#include "msg_processor.h"

int32_t msg_processor_initialize(msg_processor_t* self)
{
	self->close_handler = NULL;
	self->error_handler = NULL;
	self->msg_handler = NULL;
	self->connect_handler = NULL;
	return 0;
}

int32_t msg_processor_finalize(msg_processor_t* self)
{
	return 0;
}
