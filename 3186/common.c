#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <memory.h>
#include <sys/time.h>

#include "common.h"

int kickSocket(void* socket)
{
   zmq_pollitem_t poll_items [] = { {socket, 0, ZMQ_POLLIN | ZMQ_POLLOUT , 0} };
   return zmq_poll(poll_items, 1, 1);
}


const char* getTimestamp(char *buffer, int   bufferLength)
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   struct tm result;
   struct tm *t = localtime_r(&tv.tv_sec, &result);
   char localBuf[16];
   sprintf(localBuf, "%02d:%02d:%02d.%06d", t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec);

   memcpy(buffer, localBuf, bufferLength);
   return buffer;
}


#define MAX_LOG_MSG_SIZE 1024
#ifdef __cplusplus
extern "C" {
#endif
void log_msg(const char *format, ...)
{
   char temp[MAX_LOG_MSG_SIZE +1] = "";
   if (format) {
      va_list ap;
      va_start(ap, format);
      if (vsnprintf(temp, MAX_LOG_MSG_SIZE, format, ap) == -1)
         temp[MAX_LOG_MSG_SIZE] = '\0';
      va_end(ap);
   }

   char buf[16+1];
   fprintf(stderr, "%s\t%s\n", getTimestamp(buf, sizeof(buf)), temp);
}
#ifdef __cplusplus
}
#endif
