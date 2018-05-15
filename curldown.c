//采用CURLOPT_RESUME_FROM_LARGE实现文件断点续传功能
#include<stdlib.h>
#include<stdio.h>
#include<sys/stat.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include<curl/curl.h>
#include "curldown.h"

typedef struct{
	int quit;
	long streamSize;
	long downSize;
	long timeout;
	void *user_data;
	char filename[128];	
	void (*startDownFile)(void *user_data,const char *filename,int streamLen);
	void (*getStreamData)(void *user_data,const char *data,int size);
}FtpFile;
static FtpFile *ftp=NULL;


void progressBar(long cur_size, long total_size,float *getPercent){    
	/*用于显示下载进度条*/    
	float percent = (float) 
	cur_size / total_size;    
	const int numTotal = 50;    
	int numShow = (int)(numTotal * percent);    
	if (numShow == 0)        
		numShow = 1;    
	if (numShow > numTotal)        
		numShow = numTotal;    
	char sign[51] = {0};    
	memset(sign, '=', numTotal);    
	printf("\r%.2f%%\t[%-*.*s] %.2f/%.2fMB", percent * 100, numTotal, numShow, sign, cur_size / 1024.0 / 1024.0, total_size / 1024.0 / 1024.0);    fflush(stdout);    
	if (numShow == numTotal)        
		printf("\n");
	//printf("progressBar ...%2.f\n",percent*100);
	*getPercent = percent*100;
	//return (percent*100);
}

void quitDownFile(void){
	if(ftp->quit==DOWN_ING){
		ftp->quit=DOWN_WAIT;
		do{
			usleep(10000);
		}while(ftp->quit==DOWN_WAIT);
	}
}
void setDowning(void){
	ftp->quit=DOWN_ING;
}
int getDownState(void){
	return ftp->quit;
}
static size_t getcontentlengthfunc(void*data,size_t size,size_t nmemb,void *stream){
	int r;
	long len=0;		
	FtpFile *ftp = (FtpFile *)stream;
	r=sscanf(data,"Content-Length:%ld\n",&len);
	if(r){
		if(ftp->streamSize==0){
			ftp->streamSize =len;
			if(len!=0){
				ftp->startDownFile(ftp->user_data,ftp->filename,ftp->streamSize);
			}else{
				printf("error down file len =%d\n",len);
			}
		}
	}
	return	size*nmemb;
}

static size_t wirtefunc(void*data,size_t size,size_t nmemb,void*stream){
	//printf("nmemb = %d\n",nmemb);
	FtpFile *ftp = (FtpFile *)stream;
	if(ftp->quit==DOWN_WAIT){
		return 0;
	}

	ftp->downSize +=size*nmemb;
	ftp->getStreamData(ftp->user_data,data,size*nmemb);

	//int ret = fwrite(data,size,nmemb,ftp->fp);
	return size*nmemb;
}
static int __curlDownloadFile(CURL *curlhandle,const char *remotepath,FtpFile *ftp){ 
	CURLcode r=CURLE_GOT_NOTHING;
	curl_off_t local_file_len=-1;
	int use_resume=0;	
	if(ftp->downSize!=0)
	{ 
		local_file_len =ftp->downSize;
		use_resume=1;
	} 
	curl_easy_setopt(curlhandle, CURLOPT_SSL_VERIFYPEER, false);	//设置下载https的接口
	//curl_easy_setopt(curlhandle,CURLOPT_UPLOAD,1L);	//上传接口
	curl_easy_setopt(curlhandle,CURLOPT_URL,remotepath); 
	curl_easy_setopt(curlhandle,CURLOPT_CONNECTTIMEOUT, 10);	//设置连接超时，单位秒 
	curl_easy_setopt(curlhandle,CURLOPT_TIMEOUT,ftp->timeout);	//设置连接和下载的超时时间

	//设置http头部处理函数 
	curl_easy_setopt(curlhandle,CURLOPT_HEADERFUNCTION, getcontentlengthfunc);
	curl_easy_setopt(curlhandle,CURLOPT_HEADERDATA,ftp);

	//设置文件续传的位置给
	curl_easy_setopt(curlhandle,CURLOPT_RESUME_FROM_LARGE, use_resume?local_file_len:0); 

	curl_easy_setopt(curlhandle,CURLOPT_WRITEDATA,ftp); 
	curl_easy_setopt(curlhandle,CURLOPT_WRITEFUNCTION,wirtefunc);
	//curl_easy_setopt(curlhandle,CURLOPT_READFUNCTION,readfunc);//读取上传接口数据
	//curl_easy_setopt(curlhandle,CURLOPT_READDATA,f);			//读取上传接口参数
	curl_easy_setopt(curlhandle,CURLOPT_NOPROGRESS,1L);
	curl_easy_setopt(curlhandle,CURLOPT_VERBOSE,1L); 
	curl_easy_setopt(curlhandle,CURLOPT_FOLLOWLOCATION,1L); 
	r=curl_easy_perform(curlhandle); 
	if(r!=CURLE_OK) 
	{
		const char *error = curl_easy_strerror(r);
		fprintf(stderr,"%s: %s\n",__func__,error);
		//用户主动退出下载
		if(!strcmp(error,"Failed writing received data to disk/application")){
			//printf("-------------user quit downfile ---------\n");
			return 0;
		}else if(!strcmp(error,"Couldn't resolve host name")){
			return 0;
		}
		return -1;
	}
	
	return 0; 
}

void curlDownloadFile(DownLoad_t *download){
	int ret=0;
	CURL *curlhandle=NULL; 
	char domain[64] = {0};		
	int port = 80;
#ifdef MIN_TIMEOUT
	int MinTimeout=0;
#endif	
	//ftp->quit=DOWN_ING;	
	curlhandle=curl_easy_init(); 	
	parse_url(download->url, domain, &port, ftp->filename);	
	ftp->downSize =0;
	ftp->streamSize=0;
	ftp->startDownFile=download->startDownFile;
	ftp->getStreamData=download->getStreamData;
	ftp->user_data=download->user_data;
	while(ftp->quit==DOWN_ING)
	{	
#ifdef MIN_TIMEOUT
		if(++MinTimeout>6){
			printf("MinTimeout exit ...... \n");
			break;
		}
#endif
		ret = __curlDownloadFile(curlhandle,(const char *)download->url,ftp);
		if(!ret){
			printf("down ok \n");
			break;
		}
		printf("start download again ...... \n");
		usleep(100);
	}
	download->endDownFile(download->user_data,ftp->downSize);
	curl_easy_cleanup(curlhandle);
	ftp->quit=DOWN_QUIT;
}

void initCurl(void){
	ftp = (FtpFile *)calloc(1,sizeof(FtpFile));
	if(ftp==NULL){
		return ;
	}
	curl_global_init(CURL_GLOBAL_DEFAULT);
}
void cleanCurl(void){	
	if(ftp){
		free(ftp);
		ftp=NULL;
	}
	curl_global_cleanup();
}

//#define MAIN_TEST

#ifdef MAIN_TEST

static FILE *fp=NULL;
static void StartDownFile(void *user_data,const char *filename,int streamLen){
	fp= fopen(filename, "a+");    
	if (fp ==NULL)    {        
		printf("Create file failed\n");   
	}  
}
static void GetStreamData(void *user_data,const char *data,int size){
	//printf("GetStreamData size = %d \n",size);
	if(fp)
		fwrite(data, 1,size,fp);
}
static void EndDownFile(void *user_data,int endSize){
	printf("endSize = %d \n",endSize);
	if(fp)
		fclose(fp);
}
void testdownload(void){
	char *url = "http://fdfs.xmcdn.com/group9/M08/A1/4A/wKgDZldzNWzRoXyzACZofeFKKKc093.mp3";
	initCurl();
	setDowning();
	DownLoad_t download;
	sprintf(download.url,"%s",url);
	download.timeout=100;
	download.startDownFile=StartDownFile;
	download.getStreamData=GetStreamData;
	download.endDownFile=EndDownFile;
	curlDownloadFile(&download); 
	cleanCurl();
}
/*
int main(int argc,char **argv){ 
	printf("...............\n");
	testdownload();
	return 0;
}
*/
#endif

