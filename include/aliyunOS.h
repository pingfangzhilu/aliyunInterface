#ifndef _aliyunOS_H
#define _aliyunOS_H

#ifdef __cplusplus
extern "C" {
#endif
#include "ComsHead.h"
#include "base/queWorkCond.h"



#define URL_VOICES_EVENT	1		//下载微信发送过来对讲语音事件
#define LOCAL_MP3_EVENT 	2		//没有使用	
#define URL_CLOCK_EVENT 	3		//下载微信发送过来闹钟语音事件
#define URL_DOWN_MP3_EVENT	4		//下载微信端发送过来的mp3歌曲事件
#define CHECK_VERSION_EVENT	5		//检查版本号
typedef struct{	
	unsigned char quit;
	unsigned short upPort;
	int sockfd;
	FILE *downfp;
	WorkQueue *DownEvent;
	struct sockaddr_in voicePeer;//语音服务地址		
	char recvMnsfile[128];		//接收到mns 发送过来的语音文件
	char uploadhost[64];
}AliOsMns_t;

typedef struct{
	char url[128];
	char path[12];
}ClockEvent_t;
extern void GetAliyunMns(const char *requestUrl,const char *SECRET,const char *AccessKeyID,const char *queueName,void GetMNS(const char *JsonData));
extern void CleanAliyunMns(void);
extern const char *GetqueueName(void);

#ifdef __cplusplus
};
#endif

#endif

