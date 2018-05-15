#include "ComsHead.h"
#include "common.h"
#include "aliyunOS.h"
#include "base/cJSON.h"
#include "curldown.h"

#define DOWN_TIMEOUT	360

static FILE *fp=NULL;
static char *filePath[128]={0};
static void StartDownFile(void *user_data,const char *filename,int streamLen){
	int len = strlen(filePath);
	sprintf(filePath+len,"%s",filename);
	if (fp ==NULL)    {        
		fp= fopen(filename, "w+");    
		if(fp ==NULL){
			printf("Create file failed\n");   
		}
	}  
}
static void GetStreamData(void *user_data,const char *data,int size){
	//printf("GetStreamData size = %d \n",size);
	if(fp){
		fwrite(data, 1,size,fp);
	}
}
static void EndDownFile(void *user_data,int endSize){
	printf("endSize = %d \n",endSize);
	if(fp){
		fclose(fp);
		fp=NULL;
	}
}
static void downloadfile(const char *url,int timeout){
	DownLoad_t download;
	sprintf(download.url,"%s",url);
	setDowning();
	download.timeout=timeout;
	download.startDownFile=StartDownFile;
	download.getStreamData=GetStreamData;
	download.endDownFile=EndDownFile;
	curlDownloadFile(&download); 
}

static void PaserAliyunMns(const char *msg){
	printf("%s: msg =%s\n",__func__,msg);
	cJSON *root = cJSON_Parse((const char *)msg);
	if(root!=NULL){
		cJSON *pj = cJSON_GetObjectItem(root,(const char *)"msgtype");
		if(pj==NULL){
			goto exit0;
		}
		if(!strcmp(pj->valuestring,"downfile")){
			pj = cJSON_GetObjectItem(root,(const char *)"url");
			if(pj==NULL){
				goto exit0;
			}
			downloadfile((const char *)pj->valuestring,DOWN_TIMEOUT);
		}
	}
exit0:
	cJSON_Delete(root);
}
static int __initAliyunOs(const char *path,const char * queueName,void GetMNS(const char *JsonData)){
	initCurl();
	sprintf(filePath,"%s",path);
	return initAliyunMns(queueName,GetMNS);
}
int initAliyunOs(const char *path,const char * queueName){
	return __initAliyunOs(path,queueName,PaserAliyunMns);
}
void cleanAliyunOs(void){
	cleanCurl();
	cleanAliyunMns();
}

int main(int   argc,char *argv[]){	
	const char *queueName= "linkeweici00001";
	char *url = "http://fdfs.xmcdn.com/group9/M08/A1/4A/wKgDZldzNWzRoXyzACZofeFKKKc093.mp3";
	initAliyunOs("./",queueName);
	
	while(1){
		sleep(1);
	}
	
	return 0;
}
