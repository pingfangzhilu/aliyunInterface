#ifndef _COMMON_H
#define _COMMON_H

enum{
	PUT,
	GET,
	DELETE,
	POST
};
#ifdef __cplusplus
extern "C" {
#endif

/*
@:构造获取服务器上消息请求
@: request :构造的消息结果  queueName: 设备mac 账号
*/
extern void GetMnsRequest(char *request,const char *queueName,const char *ACCOUNT_ID,const char *SECRET,const char *AccessKeyID);
/*
@:构造删除服务器上消息请求
@: request :构造的消息结果  queueName: 设备mac 账号
*/
extern void delteMnsReq(char *request,const char *queueName,const char *ReceiptHandle,const char *ACCOUNT_ID,const char *SECRET,const char *AccessKeyID);

#ifdef __cplusplus
};
#endif

#endif
