#include <unistd.h>

#define ZMQ_BUILD_DRAFT_API
#include <zmq.h>

#include "common.h"

#include "params.c"

int main(int argc, char** argv)
{
   parseParams(argc, argv);

   void* theContext = zmq_ctx_new();

   // bind pub
   void* pub = zmq_socket(theContext, ZMQ_PUB);
   assert(pub);
   char pubEndpoint[ZMQ_MAX_ENDPOINT_LENGTH +1];
   sprintf(pubEndpoint, "tcp://127.0.0.1:%d", port);
   CALL_INT_FUNC(zmq_bind(pub, pubEndpoint));

   sleep(99999);

   CALL_INT_FUNC(zmq_close(pub));

   CALL_INT_FUNC(zmq_ctx_shutdown(theContext));
   CALL_INT_FUNC(zmq_ctx_term(theContext));

   return 0;
}


