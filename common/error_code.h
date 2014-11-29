/**
# -*- coding:UTF-8 -*-
*/

#ifndef _ERROR_CODE_H_
#define _ERROR_CODE_H_

#include <errno.h>

#define SUCCESS								0

/**
 * 0表示成功
 * linux系统错误码：0~255
 * windows系统错误码:0~20000
 * et错误码：1024~100000
*/

#define	INVALID_FILE_INDEX				15364 /* 非法的bt子文件id */

/************************** http error code	**********************************/
#define ERR_HTTP_BASE						9300	//与2.0保持一致。1024*9 + 84 = 9300

/************************** license error code **********************************/
#define ERR_LICENSE_FROZEN					21000 /* license已被冻结 */
#define ERR_LICENSE_EXPIRED					21001 /* license已过期 */
#define ERR_LICENSE_CANNOT_USE				21002 /* license尚不能被使用 */
#define ERR_LICENSE_REUSED					21003 /* license可能是被盗用 */
#define ERR_LICENSE_FALSE					21004 /* license非法 */
#define ERR_LICENSE_LIMITED					21006 /* license关联的peerid数量超过限制 */
#define ERR_LICENSE_PACKET					21007 /* license解包错误 */
#define ERR_LICENSE_PEERID_FROZEN			21008 /* peerid已被冻结 */

#define ERR_LICENSE_PEERID_FALSE			21105 /* peerid错误 */

#define ERR_LICENSE_RSA_KEY_VERSION			21031 /* ras密钥版本错误 */

#define ERR_LICENSE_SERVER_BUSY				21400 /* 服务器繁忙，请稍后重试 */

#define ERR_LICENSE_UNKNOWN					21500 /* license服务器未知错误 */

/************************** gaosu error code **********************************/
#define GAOSU_ERRCODE_BASE					23000

#define ERR_GAOSU_BEG							(GAOSU_ERRCODE_BASE + 552)	//(1024 * 23) //24552
#define ERR_GAOSU_NOT_AVAIABLE					(GAOSU_ERRCODE_BASE + 553)
#define ERR_GAOSU_UNDERFED						(GAOSU_ERRCODE_BASE + 554)
#define ERR_GAOSU_BOUND_ACCOUNT_NOT_FOUND		(GAOSU_ERRCODE_BASE + 555)
#define ERR_GAOSU_PQ_PUT_DATA_TASK_NOT_FOUND	(GAOSU_ERRCODE_BASE + 556)
#define ERR_GAOSU_IN_USE						(GAOSU_ERRCODE_BASE + 557)
#define ERR_GAOSU_NOT_IN_USE					(GAOSU_ERRCODE_BASE + 558)//23558
#define ERR_GAOSU_BT_NOT_AVAILABLE				(GAOSU_ERRCODE_BASE + 559)
#define ERR_GAOSU_IS_ENTERING					(GAOSU_ERRCODE_BASE + 560)
#define ERR_GAOSU_PQ_PUT_DATA_ERR				(GAOSU_ERRCODE_BASE + 561)
#define ERR_GAOSU_PQ_PIPE_FAILURE				(GAOSU_ERRCODE_BASE + 562)
#define ERR_GAOSU_WRONG_PROTOCOL				(GAOSU_ERRCODE_BASE + 563)
#define ERR_GAOSU_INVALID_USER					(GAOSU_ERRCODE_BASE + 564)	//没有获取到有效用户信息
#define ERR_GAOSU_INVALID_JUMPKEY				(GAOSU_ERRCODE_BASE + 565)	//没有获取到有效jumpkey信息
#define ERR_GAOSU_NO_CID_OR_SIZE				(GAOSU_ERRCODE_BASE + 566)	//没有获取到文件cid/gcid/size
#define ERR_GAOSU_RES_TIMEOUT					(GAOSU_ERRCODE_BASE + 567)	//lru中的资源过期未用导致丢失
#define ERR_GAOSU_END							(GAOSU_ERRCODE_BASE + 568)

#define ERR_GAOSU_CMD_RESULT_BASE				(GAOSU_ERRCODE_BASE + 652) //23652

#define ERR_GAOSU_ERR_CMD_STATUS_BASE			(GAOSU_ERRCODE_BASE + 852) //23852

/************************** lixian error code **********************************/
#define LIXIAN_ERRCODE_BASE					24000

#define ERR_LIXIAN_BEG						(LIXIAN_ERRCODE_BASE + 576)	/* 24576 */
#define ERR_LIXIAN_WRONG_PROTOCOL			(LIXIAN_ERRCODE_BASE + 577) /*  */
#define ERR_LIXIAN_GET_ORIGINAL_URL			(LIXIAN_ERRCODE_BASE + 578) /*  */
#define ERR_LIXIAN_USER_INFO_INVALID		(LIXIAN_ERRCODE_BASE + 579) /*  */
#define ERR_LIXIAN_IS_WORKING				(LIXIAN_ERRCODE_BASE + 580) /*  */
#define ERR_LIXIAN_TASK_NOT_RUNNING			(LIXIAN_ERRCODE_BASE + 581) /*  */
#define ERR_LIXIAN_ENCODE_CONVERTION_ERR	(LIXIAN_ERRCODE_BASE + 582) /*  */
#define ERR_LIXIAN_ACC_URL_FORMAT			(LIXIAN_ERRCODE_BASE + 583) /*  */
#define ERR_LIXIAN_INIT_ERROR				(LIXIAN_ERRCODE_BASE + 584)
#define ERR_LIXIAN_SRV_DOWNLOAD_FAILED		(LIXIAN_ERRCODE_BASE + 585) /* 24585 */
#define ERR_LIXIAN_NOT_AVAILABLE            (LIXIAN_ERRCODE_BASE + 586) /* 24586 */
#define ERR_LIXIAN_JUMPKEY_INVALID			(LIXIAN_ERRCODE_BASE + 587) /* 未取到jumpkey */
#define ERR_LIXIAN_END						(LIXIAN_ERRCODE_BASE + 588)

#define ERR_LIXIAN_CMD_RESULT_BASE			(LIXIAN_ERRCODE_BASE + 676) //24676

#define ERR_LIXIAN_CMD_STATUS_BASE			(LIXIAN_ERRCODE_BASE + 876) //24876

/************************** etm error code **********************************/
#define	ETM_ERRCODE_BASE					100000

#define ERR_NOT_FOUND						(ETM_ERRCODE_BASE+2301)
#define ERR_TOO_MANY						(ETM_ERRCODE_BASE+2302)

#define ERR_ET_WRONG_VERSION				(ETM_ERRCODE_BASE+2401) /* ET版本不兼容 */
#define ERR_ETM_OUT_OF_MEMORY				(ETM_ERRCODE_BASE+2402) /* 内存不足 */

#define ERR_INVALID_SYSTEM_PATH				(ETM_ERRCODE_BASE+2404) /* 系统目录非法(目录不存在或不可写) */
#define ERR_INVALID_LICENSE_LEN				(ETM_ERRCODE_BASE+2405) /* license长度非法 */
#define ERR_LICENSE_VERIFYING				(ETM_ERRCODE_BASE+2406) /* 正在等到license校验结果 */
#define ERR_TOO_MANY_TASKS					(ETM_ERRCODE_BASE+2407) /* 任务数超过数量限制 */
#define ERR_TOO_MANY_RUNNING_TASKS			(ETM_ERRCODE_BASE+2408) /* running任务超过数量限制 */
#define ERR_TASK_ALREADY_EXIST				202//(ETM_ERRCODE_BASE+2409) /* 任务已存在 */

#define ERR_NOT_ENOUGH_BUFFER				(ETM_ERRCODE_BASE+2411) /* 缓冲区不足 */

#define ERR_FILE_NOT_EXIST					(ETM_ERRCODE_BASE+2415) /* 文件不存在 */
#define ERR_FILE_ALREADY_EXIST				(ETM_ERRCODE_BASE+2416) /* 文件已存在 */

#define ERR_INVALID_PARAMETER				(ETM_ERRCODE_BASE+2418) /* 参数不合法 */
#define ERR_TOO_MANY_PARAMETERS				(ETM_ERRCODE_BASE+2419) /* 参数过多 */

#define ERR_INVALID_TASK					(ETM_ERRCODE_BASE+2433) /* 非法任务 */
#define ERR_INVALID_TASK_ID					(ETM_ERRCODE_BASE+2434) /* taskid不正确 */
#define ERR_INVALID_TASK_TYPE				(ETM_ERRCODE_BASE+2435) /* 任务类型不正确 */
#define ERR_INVALID_TASK_STATE				(ETM_ERRCODE_BASE+2436) /* 任务状态不正确 */
#define ERR_INVALID_TASK_INFO				(ETM_ERRCODE_BASE+2437) /* TaskInfo有误 */
#define ERR_INVALID_PATH					205//(ETM_ERRCODE_BASE+2438) /* 非法路径 */
#define ERR_INVALID_NAME					(ETM_ERRCODE_BASE+2438) /* 非法名称 */
#define ERR_INVALID_URL						201//(ETM_ERRCODE_BASE+2439) /* URL非法 */
#define ERR_INVALID_SEED_FILE				206//(ETM_ERRCODE_BASE+2440) /* 种子文件非法 */
#define ERR_INVALID_FILE_SIZE				(ETM_ERRCODE_BASE+2441) /* 文件大小非法 */
#define ERR_INVALID_CID						(ETM_ERRCODE_BASE+2442) /* cid非法 */
#define ERR_INVALID_GCID					(ETM_ERRCODE_BASE+2443) /* gcid非法 */
#define ERR_INVALID_FILE_NAME				(ETM_ERRCODE_BASE+2444) /* 文件名非法(过长或含有特殊字符) */
#define ERR_INVALID_BT_FILE_NUM				(ETM_ERRCODE_BASE+2445) /* bt子文件个数非法 */
#define ERR_INVALID_FILE_INDEX_ARRAY		(ETM_ERRCODE_BASE+2446) /* bt子文件列表非法 */
#define INVALID_USER_DATA					(ETM_ERRCODE_BASE+2447) /* 非法userdata */
#define ERR_INVALID_FILE_INDEX				(ETM_ERRCODE_BASE+2448) /* bt子文件ID非法 */

#define ERR_NOT_RUNNINT_TASK				(ETM_ERRCODE_BASE+2450) /* 只有running任务支持该操作 */
#define ERR_HIGH_SPEED_ALREADY_OPENED		(ETM_ERRCODE_BASE+2451) /* 高速已经在工作 */
#define ERR_HIGH_SPEED_OPENING				(ETM_ERRCODE_BASE+2452) /* 正在等待高速开启 */
#define ERR_NO_DISK							(ETM_ERRCODE_BASE+2453) /* 没有磁盘 */

#define ERR_TORRENT_PARSER_BAD_TORRENT		(ETM_ERRCODE_BASE+2456) /* bt种子非法 */
#define ERR_TORRENT_PARSER_STRING_TOO_LONG	(ETM_ERRCODE_BASE+2457) /* bt字符串项过长 */
#define ERR_TORRENT_PARSER_INCOMPLETE		(ETM_ERRCODE_BASE+2458) /* bt种子内容不完整 */
#define ERR_TORRENT_PARSER_BAD_READ			(ETM_ERRCODE_BASE+2459) /* 无法读取bt种子 */

//======== remote control ========
#define ERR_RC_NOT_WORKING					(ETM_ERRCODE_BASE+4454) /* 远程不在线 */

/***************************************************************************/
/*  remap sqlite, these errcodes are map from sqlite3.h                    */
/***************************************************************************/
#define RE_SQLITE_BASE						( ETM_ERRCODE_BASE+6496 )	//106496
#define RE_SQLITE_ERROR						( RE_SQLITE_BASE+1 )   /* SQL error or missing database */
#define RE_SQLITE_INTERNAL					( RE_SQLITE_BASE+2 )   /* Internal logic error in SQLite */
#define RE_SQLITE_PERM						( RE_SQLITE_BASE+3 )   /* Access permission denied */
#define RE_SQLITE_ABORT						( RE_SQLITE_BASE+4 ) //106500   /* Callback routine requested an abort */
#define RE_SQLITE_BUSY						( RE_SQLITE_BASE+5 )   /* The database file is locked */
#define RE_SQLITE_LOCKED					( RE_SQLITE_BASE+6 )   /* A table in the database is locked */
#define RE_SQLITE_NOMEM						( RE_SQLITE_BASE+7 )   /* A malloc() failed */
#define RE_SQLITE_READONLY					( RE_SQLITE_BASE+8 )   /* Attempt to write a readonly database */
#define RE_SQLITE_INTERRUPT					( RE_SQLITE_BASE+9 ) //106505  /* Operation terminated by sqlite3_interrupt()*/
#define RE_SQLITE_IOERR						( RE_SQLITE_BASE+10 )   /* Some kind of disk I/O error occurred */
#define RE_SQLITE_CORRUPT					( RE_SQLITE_BASE+11 )   /* The database disk image is malformed */
#define RE_SQLITE_NOTFOUND					( RE_SQLITE_BASE+12 )   /* Unknown opcode in sqlite3_file_control() */
#define RE_SQLITE_FULL						( RE_SQLITE_BASE+13 )   /* Insertion failed because database is full */
#define RE_SQLITE_CANTOPEN					( RE_SQLITE_BASE+14 ) //106510   /* Unable to open the database file */
#define RE_SQLITE_PROTOCOL					( RE_SQLITE_BASE+15 )   /* Database lock protocol error */
#define RE_SQLITE_EMPTY						( RE_SQLITE_BASE+16 )   /* Database is empty */
#define RE_SQLITE_SCHEMA					( RE_SQLITE_BASE+17 )   /* The database schema changed */
#define RE_SQLITE_TOOBIG					( RE_SQLITE_BASE+18 )   /* String or BLOB exceeds size limit */
#define RE_SQLITE_CONSTRAINT				( RE_SQLITE_BASE+19 ) //106515  /* Abort due to constraint violation */
#define RE_SQLITE_MISMATCH					( RE_SQLITE_BASE+20 )   /* Data type mismatch */
#define RE_SQLITE_MISUSE					( RE_SQLITE_BASE+21 )   /* Library used incorrectly */
#define RE_SQLITE_NOLFS						( RE_SQLITE_BASE+22 )   /* Uses OS features not supported on host */
#define RE_SQLITE_AUTH						( RE_SQLITE_BASE+23 )   /* Authorization denied */
#define RE_SQLITE_FORMAT					( RE_SQLITE_BASE+24 ) //106520   /* Auxiliary database format error */
#define RE_SQLITE_RANGE						( RE_SQLITE_BASE+25 )   /* 2nd parameter to sqlite3_bind out of range */
#define RE_SQLITE_NOTADB					( RE_SQLITE_BASE+26 )   /* File opened that is not a database file */
#define RE_SQLITE_ROW						( RE_SQLITE_BASE+100 )  /* sqlite3_step() has another row ready */
#define RE_SQLITE_DONE						( RE_SQLITE_BASE+101 )  /* sqlite3_step() has finished executing */

#define REMAP_SQLITE_ERR(err)				(RE_SQLITE_BASE+(err))
#define REVERT_SQLITE_ERR(err)				((err)-RE_SQLITE_BASE)

#define ERR_INVALID_REQUEST					(ETM_ERRCODE_BASE+8544) /* 非法请求 */
#define ERR_PROTOCOL_ERR					(ETM_ERRCODE_BASE+8545) /* 协议错误 */
#define ERR_PARAM_NOT_FOUND					(ETM_ERRCODE_BASE+8546) /* 找不到该参数 */

#define ERR_HUIYUAN_LOGIN_LOGING			(ETM_ERRCODE_BASE+9569) /* 正在等待会员登录 */

#define ERR_CANNOT_GET_LICENSE				(ETM_ERRCODE_BASE+10593) /* 无法获取license */


/************************** em reactor error code **********************************/
//============ COMMON ===============
#define COMMON_ERR_BASE		1*1000
#define EMEMOUT				(-(COMMON_ERR_BASE+1))
#define EQUEUEFULL			(-(COMMON_ERR_BASE+2))
#define EQUEUEEMPTY			(-(COMMON_ERR_BASE+3))

//============ HANDLE ===============
#define HANLDE_ERR_BASE		(COMMON_ERR_BASE + 500)
#define EHANDLESHOULDRETRY	(-(HANLDE_ERR_BASE+1)) // DON'T TREAT THIS AS AN ERROR, RETRY LATER

//============ SELECTOR ==============
#define SELECTOR_ERR_BASE	2*1000 
#define ESELECTORFULL		(-(SELECTOR_ERR_BASE+1))
#define ESELECTORNOENT		(-(SELECTOR_ERR_BASE+2))

//============ SOCKET ==============
#define SOCKET_ERR_BASE		3*1000
#define ESOCKINVALIDIP		(-(SOCKET_ERR_BASE+1))
#define ESOCKSENDBUFULL		(-(SOCKET_ERR_BASE+2))
#define ESOCKNOTSUPPORT		(-(SOCKET_ERR_BASE+3))
#define ESOCKTIMEOUT		(-(SOCKET_ERR_BASE+4))
#define SOCKCLOSED			(-(SOCKET_ERR_BASE+5)) // NOT an error, just notify reactor do not process this socket any more
#define ESOCKDNSTIMEOUT		(-(SOCKET_ERR_BASE+6))
#define ESOCKCONNECTTIMEOUT	(-(SOCKET_ERR_BASE+7))
#define ESOCKIDLETIMEOUT	(-(SOCKET_ERR_BASE+8))

//============ TIMER ===============
#define TIMER_ERR_BASE		4*1000
#define ETIMERFULL			(-(TIMER_ERR_BASE+1))
#define ETIMERINVALID		(-(TIMER_ERR_BASE+2))
#define ETIMERNOENT			(-(TIMER_ERR_BASE+3))

//============ REACTOR ============
#define REACTOR_ERR_BASE	6*1000
#define EREACTORFULL		(-(REACTOR_ERR_BASE+1))
#define EREACTORNOENT		(-(REACTOR_ERR_BASE+2))

//============ MSG_FACTORY ============
#define MF_ERR_BASE			7*1000
#define EMFINVALIDMSG		(-(MF_ERR_BASE+1))

#endif
