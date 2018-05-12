#ifndef _CONFIG_H
#define _CONFIG_H

//#define DBG_XML_DATA				//打印获取服务器上xml数据

//#define INPUT_CMD					//开启命令行输入

#define SPEEK_PORT	20011			//对讲消息监听端口
	
#define VOICES_SERVER_POART	20001	//语音服务监听端口

#define RECV_XML	"recv.xml"		//接收到的xml数据

//#define DEBUG_ALIYUN_OS			//测试 阿里云接口

//#define CLOCK_OFF					//闹钟

#define DOWN_CACHE_DIR	"/Down/"


#define UPLOAD_LOG



#define ALIYUN_ARGV_LOG_FILE		"/log/aliyun_argv.log"	
#define ACK_WEIXIN_LOG_FILE			"/log/ack_weixin.log"
#define RECV_WEIXIN_MESSAGE_FILE	"/log/recv_weixin.log"	
#define CLOCK_FILE_LOG_FILE			"/log/clock_log.log"

#define ALIYUN_FILE_LOCK			"/var/aliyunos.lock"
#define CLOSE_SYSTEM_LOCK_FILE		"/var/close_system.lock"

//#define HUASHANG_JIAOYU

#endif
