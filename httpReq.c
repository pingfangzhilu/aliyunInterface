/*
help doc :https://help.aliyun.com/document_detail/35134.html?spm=a2c4g.11186623.6.709.caqKXP
*/
#include "ComsHead.h"
#include "common.h"

#include "base/demo_tcp.h"
#include "base/pool.h"
#include "config.h"

#define LOCK_WEIXIN	1
#define UNLOCK_WEIXIN	0
typedef struct{
	unsigned char lockWeixin;
	unsigned char quit;
	char Devquename[32];
	char requestUrl[128];
	char SECRET[32];
	char AccessKeyID[20];
	void (*CallGetMNS)(const char *JsonData);
}AliMns;

static AliMns *alios =NULL;

void lock_weixin(void){
	alios->lockWeixin =LOCK_WEIXIN;
}
void unlock_weixin(void){
	alios->lockWeixin = UNLOCK_WEIXIN;
}
void SetSockRecvtimeOut(int sock,int time){
    struct timeval tv;
    tv.tv_sec = time;
    tv.tv_usec=0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
}

void parse_url(const char *url, char *domain, int *port, char *file_name){
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
static int GetAliOsMsg(int method,const char *queueName,const char *ReceiptHandle){
	int sockfd = 0;
    char buffer[1] = "";
    struct hostent   *host=NULL;
    int portnumber = 0;
    int nbytes = 0;
    char host_addr[256] = {0};
    char host_file[256] = {0};
    char request[1024] = "";
    int send = 0;
    int totalsend = 0;
    int i = 0;
    //GetHost(aliUrl, host_addr, host_file, &portnumber);
	parse_url((const char *)alios->requestUrl, host_addr, &portnumber, host_file);
	
    SYS_LOG("webhost:%s\n ", host_addr);
    SYS_LOG("hostfile:%s\n ", host_file);
    SYS_LOG("portnumber:%d\n ", portnumber);
#ifdef LOG_FILE
	FILE *logfp = fopen("log.txt","a+");
	if(logfp==NULL){
		goto exit1;
	}
#endif
	FILE *xmlfp=NULL;
	if(method==GET){
		GetMnsRequest(request,queueName,host_addr,(const char *)alios->SECRET,(const char *)alios->AccessKeyID);
	    xmlfp = fopen(RECV_XML,"w+");
		if(xmlfp==NULL){
			SYS_ERR_LOG("open xml failed \n");
			goto exit1;
		} 	
	}
	else if(method==DELETE){
		delteMnsReq(request,queueName,ReceiptHandle,host_addr,(const char *)alios->SECRET,(const char *)alios->AccessKeyID);
	}
	
    if((host=gethostbyname(host_addr)) == NULL)
    {
        fprintf(stderr, "Gethostname   error,   %s\n ",   strerror(errno));
        return -1;
    }
	sockfd = create_client(inet_ntoa(*((struct in_addr *)host->h_addr)),portnumber);
	if(sockfd<0){
		goto exit0;
	}
	SetSockRecvtimeOut(sockfd,30);
	
    SYS_LOG("%s\n", request);
    //printf("%s\n", request);
    send = 0;
    totalsend = 0;
    nbytes=strlen(request);
    while(totalsend < nbytes){
        send = write(sockfd, request+totalsend, nbytes-totalsend);
        if(send == -1){
            SYS_ERR_LOG( "send error!%s\n ", strerror(errno));
           	goto exit2;
        }
        totalsend += send;
        //SYS_LOG("%d bytes send OK!\n ", totalsend);
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
            //printf( "%c ", buffer[0]);
#endif
        }else{
#ifdef DBG_XML_DATA	        
			XMLdata[xmlCount++] = buffer[0];
#endif
			if(method==GET){
				fwrite(buffer,1,1,xmlfp);	
				#ifdef LOG_FILE
					fwrite(buffer,1,1,logfp);	
				#endif
				//printf( "%c ", buffer[0]);
			}
		}
    }
#ifdef DBG_XML_DATA	
    printf("\nXMLdata : \n%s\n",XMLdata);
#endif
#ifdef LOG_FILE	
	fclose(logfp);
#endif
exit2:
	if(method==GET)
		fclose(xmlfp);
exit1:
	 close(sockfd); 
exit0:
	printf( "\n");
	return 0;	
}

static int paseraliMsg(const char *queueName){
	char ReceiptHandle[128]={0};
	struct stat buf;
	//memset(&buf,0,sizeof(struct stat));
	GetAliOsMsg(GET,queueName,"");
	stat((const char *)RECV_XML , &buf);
	if(buf.st_size==0){
		SYS_WARN_LOG("is empty file \n");
		return -1;
	}
	//printf("buf.st_size =%d\n",buf.st_size);
	GetXmlData((const char *)RECV_XML  ,ReceiptHandle,alios->CallGetMNS);
	if(!strcmp(ReceiptHandle,"")){
		SYS_WARN_LOG("not message \n");
		return -1;
	}
	SYS_WARN_LOG("start delete ReceiptHandle =%s\n",ReceiptHandle);
	GetAliOsMsg(DELETE,queueName,(const char *)ReceiptHandle);
	return 0;
}

const char *GetqueueName(void){
	return (const char *)alios->Devquename;
}

static void *runAliyunMns(void *arg){
	while(alios->quit){
		sleep(1);
		if(alios->lockWeixin ==LOCK_WEIXIN){
			continue;
		}
		paseraliMsg(alios->Devquename);
	}
	return NULL;
}

void GetAliyunMns(const char *requestUrl,const char *SECRET,const char *AccessKeyID,const char * queueName,void GetMNS(const char *JsonData)){
	alios =(AliMns *)calloc(1,sizeof(AliMns));
	if(alios==NULL){
		SYS_ERR_LOG("calloc  aliyun  failed  \n");
		return ;
	}
	alios->CallGetMNS=GetMNS;
	alios->quit =1;
	alios->lockWeixin = UNLOCK_WEIXIN;
	snprintf(alios->Devquename,32,"%s",queueName);
	snprintf(alios->requestUrl,128,"%s",requestUrl);
	snprintf(alios->SECRET,32,"%s",SECRET);
	snprintf(alios->AccessKeyID,20,"%s",AccessKeyID);
	if(pthread_create_attr(runAliyunMns,alios)){
		SYS_ERR_LOG("\n create aliyun pthread failed  \n");
		goto exit0;
	}
	return ;
exit0:
	free(alios);
}
void CleanAliyunMns(void){	
	alios->quit=0;
	free(alios);
}
#define TEST_MNS
#ifdef TEST_MNS

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
int websDecode64(char *outbuf, char *string, int outlen)
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


static void GetMNS(const char *msgtype){
	printf("%s: msgtype =%s\n",__func__,msgtype);
	char msg[2048]={0};
	websDecode64(msg,(unsigned char *) msgtype, 2048);
	printf("%s: msg =%s\n",__func__,msg);
	if(!strcmp(msgtype,"text")){
		
	}else if(!strcmp(msgtype,"music")){

	}else if(!strcmp(msgtype,"music")){

	}
}
int main(int   argc,   char   *argv[]){
	const char *requestUrl="http://1226525498732712.mns.cn-hangzhou.aliyuncs.com";
	const char *SECRET="U87P0Vy0H9IXdxgb1DdBVoya89aK4r";
	const char *AccessKeyID="LTAI0V1E2e1MAHdV";
	
	const char *queueName= "linkeweici00001";
	GetAliyunMns(requestUrl,SECRET,AccessKeyID,queueName,GetMNS);
	while(1){
		sleep(1);
	}
	return 0;
}
#endif
