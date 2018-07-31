#ifndef COMMON_H_3186
#define COMMON_H_3186

#include <assert.h>

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

void checkVoid(void* x)
{
   if (x == 0) {
      fprintf(stderr, "\nError %d (%s)\n", errno, zmq_strerror(errno));
      assert(0);
   }
}


void checkInt(int x)
{
   if (x < 0) {
      fprintf(stderr, "\nError %d (%s)\n", errno, zmq_strerror(errno));
      assert(0);
   }
}
#endif
