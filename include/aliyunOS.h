#ifndef _aliyunOS_H
#define _aliyunOS_H

#ifdef __cplusplus
extern "C" {
#endif
#include "ComsHead.h"
#include "base/queWorkCond.h"

extern int ProducerMessage(const void *msg);
extern int initAliyunMns(const char * queueName,void GetMNS(const char *JsonData));
extern void cleanAliyunMns(void);
extern void lockMsg(void);
extern void unlockMsg(void);

#ifdef __cplusplus
};
#endif

#endif

