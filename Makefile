
OPENSRC_DIR=/home/workone/kaiyue/aliyunserver/opensrc

XML_INC = -I $(OPENSRC_DIR)/libxml/mxml-2.10/output/x86/include
XML_LIB_X86= -L $(OPENSRC_DIR)/libxml/mxml-2.10/output/x86/lib/ -lmxml
OPEN_SSL_INC=-I $(OPENSRC_DIR)/openssl-1.0.1t/output/x86/include/
CURL_INC=-I $(OPENSRC_DIR)/libcurl/curl-7.50.1/output/x86/include/

TAR = aliyun
TAR_FACTORY=huashang


CFLAGS = -Wall -I ./include/ $(OPEN_SSL_INC) $(CURL_INC)  $(XML_INC) $(SDK_INC) -I $(KERNEL_PATH)/include/


MY_LIB_X86 =-L/home/workone/kaiyue/aliyunserver/lib/ -lbase -lsystools463 -DX86
LDFLAGS_X86= -lrt  -L $(OPENSRC_DIR)/libcurl/curl-7.50.1/output/x86/lib -lcurl  -L $(OPENSRC_DIR)/openssl-1.0.1t/output/x86/lib/ -lssl -lcrypto $(XML_LIB_X86) $(MY_LIB_X86)     
LDFLAGS=$(LDFLAGS_X86)
CFLAGS = -Wall -I ./include/ $(OPEN_SSL_INC) $(CURL_INC)  $(XML_INC)  

CC=$(CROSS_COMPILE)gcc

all += httpReq.o
all += common.o
all += libxml.o

export CC
$(TAR): $(all)
	$(CC) $(CFLAGS) -o $(TAR) $(all) $(LDFLAGS)
	$(RM) -f *.gch *.bak $(all) 
	
%.o:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $< 

.PHONY: clean
clean:
	rm -f $(TAR) $(all) 
