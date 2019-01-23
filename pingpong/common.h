#ifndef COMMON_H_3186
#define COMMON_H_3186

#include <assert.h>

#include <zmq.h>

#define     ONE_MILLION                      1000000

#define     ZMQ_MAX_ENDPOINT_LENGTH          256
#define     UUID_STRING_SIZE                 36

typedef struct requestMsg {
   char     topic[256];
   char     type[256];
} requestMsg;




#ifdef __cplusplus
extern "C" {
#endif

   void log_msg(const char *format, ...);
   int kickSocket(void* socket);
   int kickSocket2(void* socket);
   long getMicros();

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
