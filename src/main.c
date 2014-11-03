#if 1
#include <stdio.h>
#include "reactor.h"
#include "msg_factory.h"
#include "msg_processor.h"
#include "logger.h"

FILE* out;
int term_flag;
static int32_t test_msg_validator(const char* msg, uint32_t msg_len)
{
	return msg_validator_http(msg, msg_len);
}

static void test_connect_handler(conn_info_t* conn)
{
	printf("==============> Connected!\n");
}

static void test_close_handler(conn_info_t* conn)
{
	printf("==============> Closed!\n");
	term_flag = 1;
}

static void test_error_handler(conn_info_t* conn, int32_t errcode)
{
	fprintf(stderr, "============> Error: %d\n", errcode);
}
static void test_msg_handler(conn_info_t* conn, char* msg, uint32_t msg_len)
{
	char* p = strstr(msg, "\r\n\r\n");
	fwrite(p+4, msg_len - (p - msg)-4, 1, out);
}

int main(int argc, char** argv)
{
	char* host;
	uint16_t port;
	char* req;
	reactor_t* reactor;
	conn_info_t* conn;
	msg_factory_t test_msg_factory;
	msg_processor_t test_msg_processor;
	char* outf = "reactor_test.out";
	/*
		if (argc != 4)
		{
		fprintf(stderr, "Usage: %s <host> <port> <request string>\n", argv[0]);
		return 1;
		}*/
	logger_initialize(default_logger);
	host = "www.gnu.org";
	//host = "dldir1.qq.com";
	port = 80;
	req = "GET /licenses/agpl.txt HTTP/1.0\r\nHost: www.gnu.org\r\nConnection: Keep-Alive\r\n\r\n";
	//req = "GET /qqfile/qq/QQ5.1/10035/QQ5.1.exe HTTP/1.0\r\nHost: dldir1.qq.com\r\nConnection: Keep-Alive\r\n\r\n";

	out = fopen(outf, "wb");

	msg_factory_initialize(&test_msg_factory);
	msg_processor_initialize(&test_msg_processor);
	reactor = reactor_create(3);
	test_msg_factory.validator = test_msg_validator;
	
	test_msg_processor.msg_handler = test_msg_handler;
	test_msg_processor.error_handler = test_error_handler;
	test_msg_processor.close_handler = test_close_handler;
	test_msg_processor.connect_handler = test_connect_handler;

	conn = reactor_add_connector(reactor, EM_SOCK_TCP, host, port, &test_msg_factory, &test_msg_processor);
	conn_send(conn, req, strlen(req), EM_SOCKET_COPY_MSG_BUFFER);

	msg_processor_finalize(&test_msg_processor);
	msg_factory_finalize(&test_msg_factory);

	for (; term_flag == 0;)
	{
		reactor_handle_events(reactor, 100);
	}
	fclose(out);
	logger_finalize(default_logger);
	reactor_destroy(reactor);
	printf("Result saved in file >>>>>>>>>>>>>> %s\n", outf);
	return 0;
}
#endif

