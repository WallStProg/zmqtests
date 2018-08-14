#ifndef COMMON_H_3186
#define COMMON_H_3186

#include <assert.h>

#define     ONE_MILLION                      1000000

#define     ZMQ_MAX_ENDPOINT_LENGTH          256
#define     UUID_STRING_SIZE                 36

typedef struct connectMsg {
   char     command;
   char     uuid[UUID_STRING_SIZE+1];
   char     endpoint[ZMQ_MAX_ENDPOINT_LENGTH+1];
} connectMsg;

typedef struct dummyMsg {
   char     topic[256];
   char     body[256];
} dummyMsg;


#ifdef __cplusplus
extern "C" void log_msg(const char *format, ...);
#else
void log_msg(const char *format, ...);
#endif


#define CALL_INT_FUNC(x)                                                             \
   do {                                                                              \
      int rc = (x);                                                                  \
      if (rc < 0) {                                                                  \
         log_msg("Error %d(%s)", errno, zmq_strerror(errno));                        \
      }                                                                              \
   } while(0)


void checkVoid(void* x)
{
   if (x == 0) {
      log_msg("Error %d(%s)", errno, zmq_strerror(errno));                        \
   }
}


#endif
