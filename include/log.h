#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>
 
//#define SYS_LOG(fmt, args...)   		printf("%s: "fmt,__func__, ## args)
#define SYS_LOG(fmt, args...)   		{}

#define SYS_WARN_LOG(fmt, args...)  	printf("%s: "fmt,__func__, ## args)
#define SYS_ERR_LOG(fmt, args...)  		printf("%s: "fmt,__func__, ## args)

#define CLOCK_LOG_MESSAGE(fmt, args...)	printf("%s: "fmt,__func__, ## args)
#endif
