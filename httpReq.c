/*
help doc :https://help.aliyun.com/document_detail/35134.html?spm=a2c4g.11186623.6.709.caqKXP
*/
#include "ComsHead.h"
#include "common.h"

#include "base/demo_tcp.h"
#include "base/pool.h"
#include "config.h"

#define LOCK_MSG 		1
#define UNLOCK_MSG		0
typedef struct{
	unsigned char lock;
	unsigned char quit;
	char Devquename[32];
	char AppQuename[32];
	char requestUrl[128];
	char SECRET[32];
	char AccessKeyID[20];
	void (*CallGetMNS)(const char *JsonData);
}AliMns_t;

static AliMns_t *alios =NULL;

static char    map64[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};
static char    alphabet64[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/',
};
/*********************************** Code *************************************/
/*
 *    Decode a buffer from "string" and into "outbuf"
 */
static int websDecode64(char *outbuf, char *string, int outlen)
{
    unsigned long    shiftbuf;
    char            *cp, *op;
    int                c, i, j, shift;
    op = outbuf;
    *op = '\0';
    cp = string;
    while (*cp && *cp != '=') {
        /*
         *        Map 4 (6bit) input bytes and store in a single long (shiftbuf)
         */
        shiftbuf = 0;
        shift = 18;
        for (i = 0; i < 4 && *cp && *cp != '='; i++, cp++) {
            c = map64[*cp & 0xff];
            if (c == -1) {
                printf("Bad string: %s at %c index %d \n", string,c, i);
                return -1;
            }
            shiftbuf = shiftbuf | (c << shift);
            shift -= 6;
        }
        /*
         *        Interpret as 3 normal 8 bit bytes (fill in reverse order).
         *        Check for potential buffer overflow before filling.
         */
        --i;
        if ((op + i) >= &outbuf[outlen]) {
            strcpy(outbuf, "String too big");
            return -1;
        }
        for (j = 0; j < i; j++) {
            *op++ = (char) ((shiftbuf >> (8 * (2 - j))) & 0xff);
        }
        *op = '\0';
    }
    return 0;
}
void lockMsg(void){
	alios->lock =LOCK_MSG;
}
void unlockMsg(void){
	alios->lock = UNLOCK_MSG;
}
static void SetSockRecvtimeOut(int sock,int time){
    struct timeval tv;
    tv.tv_sec = time;
    tv.tv_usec=0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
}

static void parse_url(const char *url, char *domain, int *port, char *file_name){
        int j = 0;
        int start = 0;
        *port = 80;
        char *patterns[] = {"http://", "https://", NULL};
        int i;
        for(i = 0; patterns[i]; i++)
                if (strncmp(url, patterns[i], strlen(patterns[i])) == 0)
                        start = strlen(patterns[i]);
        for( i = start; url[i] != '/' && url[i] != '\0'; i++, j++)
                domain[j] = url[i];
        domain[j] = '\0';
        char *pos = strstr(domain, ":");
        if (pos)
                sscanf(pos, ":%d", port);
        for( i = 0; i < (int)strlen(domain); i++)
        {
                if (domain[i] == ':')
                {
                        domain[i] = '\0';
                        break;
                }
        }
        j = 0;
        for ( i = start; url[i] != '\0'; i++)
        {
                if (url[i] == '/')
                {
                        if (i !=  strlen(url) - 1)
                                j = 0;
                                continue;
                }
                else
                        file_name[j++] = url[i];
        }
        file_name[j] = '\0';
}

//#define LOG_FILE	
static int httpRequest(const char *host_addr,int portnumber ,const char *head,const char *Content){
	int sockfd = 0;
    	char buffer[1] = "";
    	struct hostent   *host=NULL;
    	int nbytes = 0;
    	int send = 0;
    	int totalsend = 0;
    	int i = 0;

#ifdef LOG_FILE
	FILE *logfp = fopen("log.txt","a+");
	if(logfp==NULL){
		goto exit1;
	}
#endif
	FILE *xmlfp=NULL;
	xmlfp = fopen(RECV_XML,"w+");
   	if((host=gethostbyname(host_addr)) == NULL){
       	fprintf(stderr, "Gethostname   error,   %s\n ",   strerror(errno));
        	return -1;
    	}
	sockfd = create_client(inet_ntoa(*((struct in_addr *)host->h_addr)),portnumber);
	if(sockfd<0){
		goto exit0;
	}
	SetSockRecvtimeOut(sockfd,30);
	
    	//printf("%s\n", head);
    	send = 0;
    	totalsend = 0;
    	nbytes=strlen(head);
    	while(totalsend < nbytes){
       	send = write(sockfd, head+totalsend, nbytes-totalsend);
        	if(send == -1){
            		SYS_ERR_LOG( "send error!%s\n ", strerror(errno));
           		goto exit1;
        	}
        	totalsend += send;
        	//SYS_LOG("%d bytes send OK!\n ", totalsend);
    	}
	if(Content!=NULL||!strcmp(Content,"")){
		send = 0;
		totalsend = 0;
		nbytes=strlen(Content);
		while(totalsend < nbytes){
			send = write(sockfd, Content, nbytes-totalsend);
			if(send == -1){
				SYS_ERR_LOG( "send error!%s\n ", strerror(errno));
				goto exit1;
			}
			totalsend += send;
		 }
	}
    	SYS_LOG( "\nThe   following   is   the   response   header:\n ");
    	i=0;
//#define DBG_XML_DATA
#ifdef DBG_XML_DATA
	char XMLdata[4096];
    	memset(XMLdata,0,4096);
    	int xmlCount=0;
#endif
    	while((nbytes=read(sockfd,buffer,1))==1){/*  recv http server response   */
      	if(i < 4){
            if(buffer[0] == '\r' || buffer[0] == '\n'){
                i++;
            }else{
                i = 0;
	}
#ifdef DBG_XML_DATA
            printf( "%c ", buffer[0]);
#endif
        }else{
#ifdef DBG_XML_DATA	        
			XMLdata[xmlCount++] = buffer[0];
			printf( "%c ", buffer[0]);
#endif
			fwrite(buffer,1,1,xmlfp);	
#ifdef LOG_FILE
			fwrite(buffer,1,1,logfp);	
#endif
		}
    }
#ifdef DBG_XML_DATA	
    printf("\nXMLdata : \n%s\n",XMLdata);
#endif
#ifdef LOG_FILE	
	fclose(logfp);
#endif
	fclose(xmlfp);
exit1:
	 close(sockfd); 
exit0:
	return 0;	
}

static int ConsumerMessage(const char *queueName){
 	char host_addr[256] = {0};
    	char host_file[256] = {0};
    	char request[1024] = "";
	int portnumber = 0;	
	parse_url((const char *)alios->requestUrl, host_addr, &portnumber, host_file);
	GetMnsRequest(request,queueName,host_addr,(const char *)alios->SECRET,(const char *)alios->AccessKeyID);
	return httpRequest((const char *)host_addr,portnumber,(const char *)request,(const char *)"");
}

static int delMessage(const char *queueName,const char *ReceiptHandle){
 	char host_addr[256] = {0};
    	char host_file[256] = {0};
    	char request[1024] = "";
	int portnumber = 0;	
	parse_url((const char *)alios->requestUrl, host_addr, &portnumber, host_file);
	delteMnsReq(request,queueName,ReceiptHandle,host_addr,(const char *)alios->SECRET,(const char *)alios->AccessKeyID);
	return httpRequest((const char *)host_addr,portnumber,(const char *)request,(const char *)"");
}
static int __ProducerMessage(const char *queueName,const void *msg){
 	char host_addr[256] = {0};
    	char host_file[256] = {0};
    	char request[1024] = "";
	int portnumber = 0;	
	char *postMessage=NULL;
	char get_md5Val[64]={0};
	char recv_md5Val[64]={0};
	
	parse_url((const char *)alios->requestUrl, host_addr, &portnumber, host_file);
	char post[]={"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Message xmlns=\"http://mns.aliyuncs.com/doc/v1/\">\n\t<MessageBody>%s</MessageBody>\n\t<DelaySeconds>0</DelaySeconds>\n\t<Priority>1</Priority>\n</Message>\n"};	
	
	unsigned char * msg_64= base64_encode((unsigned char *) msg, strlen(msg));
	CreateMsg_Md5Sum((const char *)msg_64,get_md5Val);
	postMessage = (char *)calloc(1,strlen(msg_64)+strlen(post)+8);
	if(postMessage==NULL){
		perror("calloc failed ");
		return -1;
	}
	sprintf(postMessage,post,msg_64);
	SendMnsMessage(request,queueName,host_addr,(const char *)alios->SECRET,(const char *)alios->AccessKeyID,strlen(postMessage));
	//printf("%s\n",request);
	//printf("%s\n",postMessage);
	int ret =httpRequest((const char *)host_addr,portnumber,(const char *)request,(const char *)postMessage);
	free(msg_64);
	free(postMessage);
	GetXmlMd5((const char *)RECV_XML  ,recv_md5Val);
	if(strcasecmp(recv_md5Val,get_md5Val)){
		printf("send message:[%s]  failed md5:[%s]  recv_md5Val:[%s]\n",msg,get_md5Val,recv_md5Val);
		return -1;
	}
	printf("send message:[%s]  ok md5:[%s]  recv_md5Val:[%s]\n",msg,get_md5Val,recv_md5Val);
	return ret;
}
static int ProducerMessage(const void *msg){
	return __ProducerMessage(alios->AppQuename,msg);
}

static void PaserMns(const char *base64_msg){
	//printf("%s: base64_msg =%s\n",__func__,base64_msg);
	int len = strlen(base64_msg);
	char *msg = (char *)calloc(1,len);
	if(msg==NULL){
		return ;
	}
	websDecode64(msg,(unsigned char *) base64_msg, len);
	//printf("%s: msg =%s\n",__func__,msg);
	alios->CallGetMNS(msg);
}
static void *mnsPthread(void *arg){
	char ReceiptHandle[128]={0};
	struct stat buf;
	while(alios->quit){
		sleep(1);
		if(alios->lock==LOCK_MSG){
			continue;
		}
		ConsumerMessage(alios->Devquename);
		stat((const char *)RECV_XML , &buf);
		if(buf.st_size==0){
			SYS_WARN_LOG("is empty file \n");
			continue;
		}
		GetXmlData((const char *)RECV_XML  ,ReceiptHandle,PaserMns);
		if(!strcmp(ReceiptHandle,"")){
			SYS_WARN_LOG("not message \n");
			continue;
		}
		SYS_LOG("start delete ReceiptHandle =%s\n",ReceiptHandle);
		delMessage(alios->Devquename,(const char *)ReceiptHandle);
	}
	return NULL;
}	
static int __initAliyunMns(const char *requestUrl,const char *SECRET,const char *AccessKeyID,const char * queueName,void GetMNS(const char *JsonData)){
	alios =(AliMns_t *)calloc(1,sizeof(AliMns_t));
	if(alios==NULL){
		SYS_ERR_LOG("calloc  aliyun  failed  \n");
		return -1;
	}
	alios->CallGetMNS=GetMNS;
	alios->quit =1;
	alios->lock = UNLOCK_MSG;
	
	//snprintf(alios->Devquename,32,"%s",queueName);
	snprintf(alios->Devquename,32,"%s_d",queueName);
	snprintf(alios->AppQuename,32,"%s_a",queueName);
	snprintf(alios->requestUrl,128,"%s",requestUrl);
	snprintf(alios->SECRET,32,"%s",SECRET);
	snprintf(alios->AccessKeyID,20,"%s",AccessKeyID);
	printf("alios->AppQuename = %s\n",alios->AppQuename);
	printf("alios->Devquename = %s\n",alios->Devquename);
#if 1	
	if(pthread_create_attr(mnsPthread,alios)){
		SYS_ERR_LOG("\n create aliyun pthread failed  \n");
		goto exit0;
	}
#else
	char *msg="linux message test send";
	ProducerMessage(msg);
#endif
	return 0;
exit0:
	free(alios);
	return -1;
}
int initAliyunMns(const char * queueName,void GetMNS(const char *JsonData)){
	const char *requestUrl="http://1226525498732712.mns.cn-hangzhou.aliyuncs.com";
	const char *SECRET="U87P0Vy0H9IXdxgb1DdBVoya89aK4r";
	const char *AccessKeyID="LTAI0V1E2e1MAHdV";
	return __initAliyunMns(requestUrl,SECRET,AccessKeyID,queueName,GetMNS);
}
void cleanAliyunMns(void){	
	alios->quit=0;
	free(alios);
}

#ifdef TEST_MAIN
static void testPaserMns(const char *msg){
	printf("%s: msg =%s\n",__func__,msg);
}
int main(int   argc,char *argv[]){	
	const char *queueName= "linkeweici00001";
	initAliyunMns(queueName,testPaserMns);
	while(1){
		sleep(1);
	}
	return 0;
}
#endif
