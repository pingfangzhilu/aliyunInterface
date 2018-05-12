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
@:�����ȡ����������Ϣ����
@: request :�������Ϣ���  queueName: �豸mac �˺�
*/
extern void GetMnsRequest(char *request,const char *queueName,const char *ACCOUNT_ID,const char *SECRET,const char *AccessKeyID);
/*
@:����ɾ������������Ϣ����
@: request :�������Ϣ���  queueName: �豸mac �˺�
*/
extern void delteMnsReq(char *request,const char *queueName,const char *ReceiptHandle,const char *ACCOUNT_ID,const char *SECRET,const char *AccessKeyID);

#ifdef __cplusplus
};
#endif

#endif
