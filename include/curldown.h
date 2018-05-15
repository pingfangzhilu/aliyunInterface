#ifndef __CURLDOWN_H
#define __CURLDOWN_H 

#ifdef __cplusplus
extern "C" {
#endif

#define DOWN_QUIT	0
#define DOWN_WAIT	1
#define DOWN_ING	2

typedef struct{
	void *user_data;
	char url[128];
	int timeout;
	void (*startDownFile)(void *user_data,const char *filename,int streamLen);
	void (*getStreamData)(void *user_data,const char *data,int size);
	void (*endDownFile)(void *user_data,int endSize);
}DownLoad_t;

extern void progressBar(long cur_size, long total_size,float *getPercent);

extern void quitDownFile(void);

extern void setDowning(void);

extern int getDownState(void);

extern void curlDownloadFile(DownLoad_t *download);

extern void initCurl(void);

extern void cleanCurl(void);

#ifdef __cplusplus
};
#endif

#endif
