#ifndef _aliyunOS_H
#define _aliyunOS_H

#ifdef __cplusplus
extern "C" {
#endif
#include "ComsHead.h"
#include "base/queWorkCond.h"



#define URL_VOICES_EVENT	1		//����΢�ŷ��͹����Խ������¼�
#define LOCAL_MP3_EVENT 	2		//û��ʹ��	
#define URL_CLOCK_EVENT 	3		//����΢�ŷ��͹������������¼�
#define URL_DOWN_MP3_EVENT	4		//����΢�Ŷ˷��͹�����mp3�����¼�
#define CHECK_VERSION_EVENT	5		//���汾��
typedef struct{	
	unsigned char quit;
	unsigned short upPort;
	int sockfd;
	FILE *downfp;
	WorkQueue *DownEvent;
	struct sockaddr_in voicePeer;//���������ַ		
	char recvMnsfile[128];		//���յ�mns ���͹����������ļ�
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

