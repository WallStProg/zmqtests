#ifndef COMMON_H_3186
#define COMMON_H_3186

#include <assert.h>

#define ZMQ_BUILD_DRAFT_API
#include <zmq.h>

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
extern "C" {
#endif

   void log_msg(const char *format, ...);
   int kickSocket(void* socket);
   int kickSocket2(void* socket);

#ifdef __cplusplus
}
#endif


#define CALL_INT_FUNC(x)                                                             \
   do {                                                                              \
      int rc = (x);                                                                  \
      if (rc < 0) {                                                                  \
         log_msg("Error %d(%s)", errno, zmq_strerror(errno));                        \
      }                                                                              \
   } while(0)


#endif
