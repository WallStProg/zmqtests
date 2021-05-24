#include <unistd.h>

#define ZMQ_BUILD_DRAFT_API
#include <zmq.h>

#include "common.h"
#include "monitor.h"

#include "params.c"

int main(int argc, char** argv)
{
   parseParams(argc, argv);

   void* theContext = zmq_ctx_new();

   int rc;
   rc = startMonitor(theContext, monitorFunc);
   assert(rc == 0);

   // bind pub
   void* pub = zmq_socket(theContext, ZMQ_PUB);
   assert(pub);

   CALL_INT_FUNC(zmq_socket_monitor_versioned(pub, "inproc://dataSub", ZMQ_EVENT_ALL, 2, ZMQ_PAIR));

   char pubEndpoint[ZMQ_MAX_ENDPOINT_LENGTH +1];
   sprintf(pubEndpoint, "tcp://%s:%d", tcpAddr, port);
   CALL_INT_FUNC(zmq_bind(pub, pubEndpoint));

   sleep(99999);

   rc = zmq_socket_monitor(pub, NULL, ZMQ_EVENT_ALL);
   assert(rc == 0);

   CALL_INT_FUNC(zmq_close(pub));

   CALL_INT_FUNC(zmq_ctx_shutdown(theContext));
   CALL_INT_FUNC(zmq_ctx_term(theContext));

   return 0;
}


