#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "openssl/hmac.h"
#include "common.h"
#include "systools.h"
#include "log.h"

#define MNS_VERSION	"2015-06-06"
#if 0
#define ACCOUNT_ID	"1985610938783461"
#define SECRET		"Ug1veajm0OvJ40xx7XfAkyT7PvK57x"
#define AccessKeyID	"LTAIrlAqr5PJxkp5"
#else

//#define ACCOUNT_ID	"1080160006225952"
//#define SECRET		"FqYfA21KuKWh6hHgeXKOO9Cv71fPfC"
//#define AccessKeyID	"bu1ocTf36YI6zFkP"
#endif

//KEY:aJutw3MNm5QTlfYs
//SECRET:sona0wPx8TdblecuNnyxDKtQSnMPuc
//消息推送地址：  http://1183877247676334.mns.cn-shenzhen.aliyuncs.com
//AccountId: 1183877247676334

//Host：1183877247676334.mns.cn-shenzhen.aliyuncs.com
//【链接】公共参数
//https://help.aliyun.com/document_detail/27485.html?spm=5176.doc27487.6.237.IoFQEr

static void GetDate(char *date){
    const char* wday[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    const char* mon[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    time_t timep;
    struct tm* p;
    time(&timep);	
    //p=localtime(&timep);  /*取得当地时间*/
    p = gmtime(&timep);
    snprintf(date,128,"%s%s%02d %s %d %02d:%02d:%02d%s",wday[p->tm_wday],", ",p->tm_mday,mon[(p->tm_mon)],(1900+p->tm_year),p->tm_hour, p->tm_min, p->tm_sec," GMT");
	//printf("%s\n", date);
}
static void CreateDate(char *request,char *date)
{
    GetDate(date);
    strcat(request,"Date:");
    strcat(request,date);
    strcat(request,"\n");
}    
static void AppendAccountId(char *request,const char *account_id)
{
    strcat(request,"http://");
    strcat(request,account_id);
    strcat(request,"/");
    //strcat(request,".mns.cn-shenzhen.aliyuncs.com/");	
}
static void CreatePut(char *request,char *CanonicalizedResource,const char *queueName,const char *account_id){
    strcat(request,"PUT ");
    AppendAccountId(request,account_id);
    strcat(request,"queues/");
    strcat(request,queueName);
    strcat(request,"?metaOverride=true HTTP/1.1\n");
    snprintf(CanonicalizedResource,128,"%s%s%s","/queues/",queueName,"?metaOverride=true\n");	
}
static void CreateGet(char *request,char *CanonicalizedResource,const char *queueName,const char *account_id)
{
    strcat(request,"GET ");
    AppendAccountId(request,account_id);
    strcat(request,"queues/");
    strcat(request,queueName);
    strcat(request,"/messages HTTP/1.1\n");
    snprintf(CanonicalizedResource,128,"%s%s%s","/queues/",queueName,"/messages");	
}
static void CreateDelete(char *request,char *CanonicalizedResource,const char *queueName,const char *ReceiptHandle,const char *account_id)
{ 
    strcat(request,"DELETE ");
    //strcat(request,"http://1183877247676334.mns.cn-shenzhen.aliyuncs.com/");
    AppendAccountId(request,account_id);
    strcat(request,"queues/");
    strcat(request,queueName);
    strcat(request,"/messages?");
    strcat(request,"ReceiptHandle=");
    strcat(request,ReceiptHandle);
    strcat(request," HTTP/1.1\n");
    snprintf(CanonicalizedResource,128,"%s%s%s%s%s","/queues/",queueName,"/messages?","ReceiptHandle=",ReceiptHandle);	
}
static void CreatePost(char *request,char *CanonicalizedResource,const char *queueName,const char *account_id)
{ 
    strcat(request,"POST ");
    AppendAccountId(request,account_id);
    strcat(request,"queues/");
    strcat(request,queueName);
    strcat(request,"/messages HTTP/1.1\n");
    snprintf(CanonicalizedResource,128,"%s%s%s","/queues/",queueName,"/messages");	
}
static void CreateHost(char *request,const char *AccountId)
{
    strcat(request,"Host:");
    strcat(request,AccountId);
    //strcat(request,".mns.cn-hangzhou.aliyuncs.com\n");
    strcat(request,".mns.cn-shenzhen.aliyuncs.com\n");
}
static void createVersion(char *request,const char *version)
{
    strcat(request,"x-mns-version:");
    strcat(request,version);
    strcat(request,"\n");
}
static void pt(char *md, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++)
    {   
        printf( "%d\t", md[i]);
    }   
    printf("\n");
}

//char * Signature = base64(hmac-sha1(VERB + "\n"
//                                    + CONTENT-MD5 + "\n"
//                                    + CONTENT-TYPE + "\n"
//                                    + DATE + "\n"
//                                    + CanonicalizedMNSHeaders
//                                    + CanonicalizedResource));

static void test_Signature(void)
{
    char hmac_str1[1024]={0};
    int n = snprintf(hmac_str1,1024,"%s%s%s%s%s","GET\n\n","text/xml\n","Tue, 04 Oct 2016 09:20:17 GMT\n","x-mns-version:2015-06-06\n","/queues/1104653919/messages");
    int hmac_len = n;
    unsigned int digest_len=0;
    char *SECRET	=	"FqYfA21KuKWh6hHgeXKOO9Cv71fPfC";
    int key_length=strlen(SECRET);
    char digest[EVP_MAX_MD_SIZE]={0};
    //char digest[EVP_MAX_MD_SIZE]={-77,101,-84,53,-51,-76,8,43,-37,74,86,-97,12,-54,-16,-89,-51,-71,-73,-11};
    digest_len=20;
    HMAC(EVP_sha1(), (const void *)SECRET, key_length, (const unsigned char *)hmac_str1,hmac_len,(unsigned char *)digest, &digest_len);
    pt(digest, digest_len);
    char * Signature = (char *)base64_encode((unsigned char *)digest,digest_len);
    printf("test Signature = %s ok Signature =%s\n",Signature,"s2WsNc20CCvbSlafDMrwp825t/U=");
}

static void createSignature(char *request,const char *date,const char *method,const char *CanonicalizedResource,const char *SECRET,const char *AccessKeyID)
{
//    test_Signature();
    char *CanonicalizedMNSHeaders = "x-mns-version:2015-06-06\n";
    char hmac_str1[1024]={0};
    snprintf(hmac_str1,1024,"%s%s%s%s%s%s%s",method,"\n\n","text/xml\n",date,"\n",CanonicalizedMNSHeaders,CanonicalizedResource);
    int hmac_len = strlen(hmac_str1);
    unsigned int digest_len=0;
    int key_length=strlen(SECRET);
    char digest[EVP_MAX_MD_SIZE]={0};
    HMAC(EVP_sha1(), (const void *)SECRET, key_length,(const unsigned char *)hmac_str1,hmac_len,(unsigned char *)digest, &digest_len);
//    pt(digest, digest_len);
    char * Signature = (char *)base64_encode((unsigned char *)digest,digest_len);
    SYS_LOG("Signature = %s\n",Signature); 	  
    strcat(request,"Authorization:MNS ");
    strcat(request,AccessKeyID);
    strcat(request,":");
    strcat(request,Signature);
    strcat(request,"\n");
    free(Signature);
}

static void CreateCommonHead(char *request,const char *queueName,int method,const char *ReceiptHandle,const char *ACCOUNT_ID,const char *SECRET,const char *AccessKeyID,int Content_Length){
    char date[128]={0};
    char CanonicalizedResource[128]={0};
    char methodStr[10]={0};
    char Content[64]={0};
    switch(method){
        case PUT:
	      	break;
	case GET:
    		CreateGet(request, CanonicalizedResource,queueName,(const char *)ACCOUNT_ID);
		snprintf(methodStr,10,"%s","GET");
		break;
	case DELETE:
    		CreateDelete(request,CanonicalizedResource,queueName,ReceiptHandle,(const char *)ACCOUNT_ID);
		snprintf(methodStr,10,"%s","DELETE");
		break;
	case POST:
		
		CreatePost(request, CanonicalizedResource,queueName,(const char *)ACCOUNT_ID);
		snprintf(Content,64,"Content-Length:%d\n",Content_Length);
		strcat(request,Content);	
		snprintf(methodStr,10,"%s","POST");
		break;
    }
    CreateDate(request,date);
    CreateHost(request, (const char *)ACCOUNT_ID);
    createVersion(request,(const char *)MNS_VERSION);
    
    createSignature(request,(const char *)date,(const char *)methodStr,(const char *)CanonicalizedResource,SECRET,AccessKeyID);
    strcat(request,"Content-Type:text/xml\r\n");
    //strcat(request,"Connection: keep-alive\r\n");
    strcat(request,"Connection:close\n");	
    strcat(request,"\n");
}
/*
@:构造获取服务器上消息请求
@: request :构造的消息结果  queueName: 设备mac 账号
*/
void GetMnsRequest(char *request,const char *queueName,const char *ACCOUNT_ID,const char *SECRET,const char *AccessKeyID){
	CreateCommonHead(request,queueName,GET,NULL,ACCOUNT_ID,SECRET,AccessKeyID,0);	
}
/*
@:构造删除服务器上消息请求
@: request :构造的消息结果  queueName: 设备mac 账号
*/
void delteMnsReq(char *request,const char *queueName,const char *ReceiptHandle,const char *ACCOUNT_ID,const char *SECRET,const char *AccessKeyID){
    CreateCommonHead(request,queueName,DELETE,ReceiptHandle,ACCOUNT_ID,SECRET,AccessKeyID,0);
}

void SendMnsMessage(char *request,const char *queueName,const char *ACCOUNT_ID,const char *SECRET,const char *AccessKeyID,int Content_Length){
	CreateCommonHead(request,queueName,POST,NULL,ACCOUNT_ID,SECRET,AccessKeyID,Content_Length);	
}

#if 0
int main(void){
    //char * p = GetDate();
    char request[2048]={0};
    char *queueName="1104653919";
    CreateCommonHead(request,queueName,ACCOUNT_ID);
    printf("request:\n");
    printf("%s\n",request);
    return 0;
}
#endif
