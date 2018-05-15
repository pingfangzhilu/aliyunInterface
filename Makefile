
OPENSRC_DIR=/home/workone/kaiyue/aliyunserver/opensrc

XML_INC = -I $(OPENSRC_DIR)/libxml/mxml-2.10/output/x86/include
OPEN_SSL_INC=-I $(OPENSRC_DIR)/openssl-1.0.1t/output/x86/include/
CURL_INC=-I $(OPENSRC_DIR)/libcurl/curl-7.50.1/output/x86/include/

TAR = aliyun

LDFLAGS_X86=-lrt -lssl  -lcrypto -lpthread -lcurl   -lmxml -L/home/workone/kaiyue/aliyunserver/lib/  -lbase -lsystools -DX86
LDFLAGS=$(LDFLAGS_X86)
CFLAGS = -Wall -I ./include/ $(OPEN_SSL_INC) $(CURL_INC)  $(XML_INC)  

CC=$(CROSS_COMPILE)gcc

all += httpReq.o
all += common.o
all += libxml.o
all += md5.o
all += curldown.o
all +=aliyunInterface.o

export CC
$(TAR): $(all)
	$(CC) $(CFLAGS) -o $(TAR) $(all) $(LDFLAGS)
	$(RM) -f *.gch *.bak $(all) 
	
%.o:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $< 

.PHONY: clean
clean:
	rm -f $(TAR) $(all) 
