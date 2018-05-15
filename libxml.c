#include "ComsHead.h"

#ifdef XML_LIB
#include "libxml/parser.h"
#include "libxml/tree.h"
//#define DEBUG_XML_TIME

#ifdef DEBUG_XML_TIME
static FILE *log_fp=NULL;
static int logNum=0;
static void Getaliyun_Date(const char *log_str,time_t *timep){
	if(++logNum>20){
		log_fp =fopen("/log/xml.log","w+");
		logNum=0;
	}else{
		log_fp =fopen("/log/xml.log","a+");
	}
    	const char* wday[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    	const char* mon[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    	struct tm* p;
    	p = gmtime(timep);
    	printf("%s%s%02d %s %d %02d:%02d:%02d%s\n",wday[p->tm_wday],", ",p->tm_mday,mon[(p->tm_mon)],(1900+p->tm_year),p->tm_hour, p->tm_min, p->tm_sec," GMT");
	if(log_fp){
		fprintf(log_fp,"%s",log_str);
		fprintf(log_fp,"%s%s%02d %s %d %02d:%02d:%02d%s\n",wday[p->tm_wday],", ",p->tm_mday,mon[(p->tm_mon)],(1900+p->tm_year),p->tm_hour, p->tm_min, p->tm_sec," GMT");
		fclose(log_fp);
	}
}
void getxmlTime(const char *xmlTime,unsigned int *sec_t,unsigned int *msec_t){
	int len = strlen(xmlTime);
	char min[12]={0};
	char sec[4]={0};
	memcpy(min,xmlTime,len-3);
	memcpy(sec,xmlTime+len-3,3);
	*sec_t = atoi(min);
	*msec_t = atoi(sec);
}

static void debug_xmlTime(const char *log_str,const char *szKey){
	unsigned int sec=0;
	unsigned int msec=0;
	getxmlTime((const char *)szKey,&sec,&msec);
	//printf(" sec=%d msec=%d\n",sec,msec);
	printf("%s",log_str);
	Getaliyun_Date((const char *)log_str,(time_t *)&sec);
}
void write_aliyun_log(const char *data){
	log_fp =fopen("/log/xml.log","a+");
	if(log_fp){
		fprintf(log_fp,"%s\n","recv:");
		fprintf(log_fp,"%s","text:");
		fprintf(log_fp,"%s\n",data);
		fclose(log_fp);
	}
}
#endif

int GetXmlData(const char *szDocName ,char *ReceiptHandle,void weixinMessage(const char *JsonData)){
	int ret=0;
    xmlDocPtr doc; 		//定义解析文档指针
    xmlNodePtr curNode; //定义结点指针(你需要它为了在各个结点间移动)
    xmlChar *szKey; 	//临时字符串变量

    //doc = xmlReadFile(szDocName,"GB2312",XML_PARSE_RECOVER); //解析文件
	doc = xmlReadFile(szDocName,"UTF-8",XML_PARSE_RECOVER);
    if (NULL == doc){ 
       printf("Document not parsed successfully\n"); 
       return -1;
    }
    curNode = xmlDocGetRootElement(doc); //确定文档根元素
    if (NULL == curNode){
       	printf("empty document\n");
	   	ret=-1;
		goto exit0;
    }

    curNode = curNode->children;		
    while(curNode != NULL){
      	 //取出节点中的内容
		if ((!xmlStrcmp(curNode->name,(const xmlChar *)"MessageBody"))) {
			szKey = xmlNodeGetContent(curNode);
			//"title":"music","content":"http://fdfs.xmcdn.com/group17/M0B/50/52/wKgJKVfyQXqjNiRmABxlodbcjhY095.mp3"
			//printf("Content szKey ============ %s ============\n", szKey);
			weixinMessage(szKey);
			//sscanf(szKey+1, "%*[^:]:\"%[^\"]\",\"%*[^:]:\"%[^\"]", msgtype, msgdata);
#ifdef DEBUG_XML_TIME			
			write_aliyun_log((const char *)szKey);
#endif
			xmlFree(szKey);
		}
		else if ((!xmlStrcmp(curNode->name,"ReceiptHandle"))) {
			szKey = xmlNodeGetContent(curNode);
			memcpy(ReceiptHandle,szKey,strlen((char *)szKey));
			//printf("Content ReceiptHandle ============ %s ============\n", ReceiptHandle);
			xmlFree(szKey);
		}
#ifdef DEBUG_XML_TIME
		else if ((!xmlStrcmp(curNode->name,"EnqueueTime"))){
			time_t tt ;
			time(&tt);
			printf("get system time:");
			Getaliyun_Date((const char *)"get system time:",(time_t *)&tt);
			szKey = xmlNodeGetContent(curNode);
			debug_xmlTime((const char *)"EnqueueTime:",szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(curNode->name,"FirstDequeueTime"))){
			szKey = xmlNodeGetContent(curNode);
			debug_xmlTime((const char *)"FirstDequeueTime:",szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(curNode->name,"NextVisibleTime"))){
			szKey = xmlNodeGetContent(curNode);
			debug_xmlTime((const char *)"NextVisibleTime:",szKey);
			xmlFree(szKey);
		}else if((!xmlStrcmp(curNode->name,"DequeueCount"))){
			szKey = xmlNodeGetContent(curNode);
			printf("DequeueCount:%s\n",szKey);
			xmlFree(szKey);
		}
#endif	
		curNode = curNode->next;
     }
exit0:
	xmlFreeDoc(doc);
    return ret; 
}
#else
#include<mxml.h>
#if 0
void test_read(const char *xmlfile){
	FILE *fp = fopen(xmlfile,"r");
	printf("\n");
	char buf[128]={0};
	while(1){
		memset(buf,0,128);
		int ret = fread(buf,1,128,fp);
		if(ret==0){
			break;
		}
		printf("%s",buf);
	}
	fclose(fp);
	printf("\n");
}
#endif
/*
mxmlLoadFile()和mxmlLoadString().这些函数的最后一个参数是一个回调函数，决定了在一个XML文档中每个数据节点的值的类型。
Mini-XML为简单XML数据文件定义了几个标准的回调函数：
MXML_INTEGER_CALLBACK-所有的数据节点包含以空格分割的整数。
MXML_OPAQUE_CALLBACK-所有的数据节点包含"不透明"字符串（CDATA）。
MXML_REAL_CALLBACK-所有的数据节点包含以空格分割的浮点数。
MXML_TEXT_CALLBACK-所有的数据节点包含以空格分割的文本字符串。


MXML_DESCEND含义是一直向下直到树的根部
MXML_DESCEND_FIRST含义是向下搜索到一个节点的第一个匹配子节点，但不再继续向下搜索
MXML_NO_DESCEND含义是不查看任何的子节点在XML元素层次中，仅查看同级的伙伴节点或者父节点直到到达顶级节点或者给出的树的顶级节点.
*/
int GetXmlData(const char *xmlfile,char *ReceiptHandle,void weixinMessage(const char *JsonData)){
        FILE *fp;
        mxml_node_t *tree,*node;
        int ret=-1;
	 //test_read(xmlfile);		
        fp = fopen(xmlfile, "r");	
        tree = mxmlLoadFile(NULL, fp,MXML_OPAQUE_CALLBACK);
        fclose(fp);

        mxml_node_t *val;
        node = mxmlGetFirstChild(tree);
        val = mxmlFindElement(node, tree, "MessageBody",NULL, NULL,MXML_DESCEND);
        if(val){
                //printf("MessageBody: %s \n",val->child->value.opaque);
                weixinMessage(val->child->value.opaque);
        }

        val = mxmlFindElement(node, tree, "ReceiptHandle",NULL, NULL,MXML_DESCEND);
        if(val){
                //printf("ReceiptHandle: %s \n",val->child->value.opaque);
                memcpy(ReceiptHandle,val->child->value.opaque,strlen(val->child->value.opaque));
                ret=0;
        }
        mxmlDelete(tree);
        return ret;
}

int GetXmlMd5(const char *xmlfile,char *md5){
        FILE *fp;
        mxml_node_t *tree,*node;
        int ret=-1;
	 //test_read(xmlfile);		
        fp = fopen(xmlfile, "r");	
        tree = mxmlLoadFile(NULL, fp,MXML_OPAQUE_CALLBACK);
        fclose(fp);

        mxml_node_t *val;
        node = mxmlGetFirstChild(tree);
        val = mxmlFindElement(node, tree, "MessageBodyMD5",NULL, NULL,MXML_DESCEND);
        if(val){
		//printf("MessageBodyMD5: %s \n",val->child->value.opaque);
        	sprintf(md5,"%s",val->child->value.opaque);		
        }
        mxmlDelete(tree);
        return ret;
}
#endif
#if 0
/********************************************************************
*********************************************************************/
void CreateXml(void){
    //定义文档和节点指针
    xmlDocPtr doc = xmlNewDoc(BAD_CAST"1.0");
    xmlNodePtr root_node = xmlNewNode(NULL,BAD_CAST"root");

    //设置根节点
    xmlDocSetRootElement(doc,root_node);
    //在根节点中直接创建节点
    xmlNewTextChild(root_node, NULL, BAD_CAST "newNode1", BAD_CAST "newNode1 content\n");
    xmlNewTextChild(root_node, NULL, BAD_CAST "newNode2", BAD_CAST "newNode2 content\n");
    xmlNewTextChild(root_node, NULL, BAD_CAST "newNode3", BAD_CAST "newNode3 content\n");
    //创建一个节点，设置其内容和属性，然后加入根结点
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"node2");
    xmlNodePtr content = xmlNewText(BAD_CAST"NODE CONTENT");
    xmlAddChild(root_node,node);
    xmlAddChild(node,content);
    xmlNewProp(node,BAD_CAST"attribute",BAD_CAST "yes\n");
    //创建一个儿子和孙子节点
    node = xmlNewNode(NULL, BAD_CAST "son");
    xmlAddChild(root_node,node);
    xmlNodePtr grandson = xmlNewNode(NULL, BAD_CAST "grandson");
    xmlAddChild(node,grandson);
    xmlAddChild(grandson, xmlNewText(BAD_CAST "This is a grandson node"));
    //存储xml文档
    int nRel = xmlSaveFile("CreatedXml.xml",doc);

    if (nRel != -1){
       printf("一个xml文档被创建,写入 \n");
    }

    //释放文档内节点动态申请的内存
    xmlFreeDoc(doc);
    return 0;
}
#endif
#if 0
void testweixinMessage(const char *JsonData){
	char msgtype[64]={0};
	char msgdata[512]={0};
	sscanf(JsonData+1, "%*[^:]:\"%[^\"]\",\"%*[^:]:\"%[^\"]", msgtype, msgdata);
} 
int main(void){
	char msgdata[512]={0},delsign[512]={0};
	char msgtype[64]={0};
	GetXmlData((const char *)"text.xml",msgtype,msgdata,delsign);
	printf("==========================================================================\n");
	printf("%s\n%s\n%s\n",filetype,url,delsign);
	printf("==========================================================================\n");

}
#endif
