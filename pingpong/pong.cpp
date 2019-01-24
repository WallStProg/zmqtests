#define ZMQ_BUILD_DRAFT_API
#include <zmq.h>

#include "common.h"

#include "params.c"

int main(int argc, char** argv)
{
   parseParams(argc, argv);

   void* theContext = zmq_ctx_new();

    // setup monitor sockets
   void* controlSub = zmq_socket(theContext, ZMQ_SERVER);
   assert(controlSub);
   CALL_INT_FUNC(zmq_bind(controlSub, "inproc://monitor"));

   void* controlPub = zmq_socket(theContext, ZMQ_CLIENT);
   assert(controlPub);
   CALL_INT_FUNC(zmq_connect(controlPub, "inproc://monitor"));


   // bind pub
   void* pub = zmq_socket(theContext, ZMQ_PUB);
   char pubEndpoint[ZMQ_MAX_ENDPOINT_LENGTH +1];
   sprintf(pubEndpoint, "tcp://127.0.0.1:%d", pongPort);
   CALL_INT_FUNC(zmq_bind(pub, pubEndpoint));

   // connect sub
   void* sub = zmq_socket(theContext, ZMQ_SUB);
   CALL_INT_FUNC(zmq_setsockopt(sub, ZMQ_SUBSCRIBE, "", 0));
   char subEndpoint[ZMQ_MAX_ENDPOINT_LENGTH +1];
   sprintf(subEndpoint, "tcp://127.0.0.1:%d", pingPort);
   CALL_INT_FUNC(zmq_connect(sub, subEndpoint));

   while(1) {
      int waitFlag = 0;
      if (pollFlag) {
         zmq_pollitem_t pollitems[] = { { sub, 0, ZMQ_POLLIN , 0}, { controlSub, 0, ZMQ_POLLIN , 0} };
         CALL_INT_FUNC(zmq_poll(pollitems, controlFlag ? 2 : 1, -1));
         waitFlag = ZMQ_DONTWAIT;
      }
      requestMsg requestMsg;
      CALL_INT_FUNC(zmq_recv(sub, &requestMsg, sizeof(requestMsg), waitFlag));
      assert(strcmp(requestMsg.type, "request") == 0);

      strcpy(requestMsg.type, "ack");
      CALL_INT_FUNC(zmq_send(pub, &requestMsg, sizeof(requestMsg), ZMQ_DONTWAIT));

      strcpy(requestMsg.type, "response");
      CALL_INT_FUNC(zmq_send(pub, &requestMsg, sizeof(requestMsg), ZMQ_DONTWAIT));
   }

   CALL_INT_FUNC(zmq_close(pub));
   CALL_INT_FUNC(zmq_close(sub));

   CALL_INT_FUNC(zmq_close(controlSub));
   CALL_INT_FUNC(zmq_close(controlPub));

   CALL_INT_FUNC(zmq_ctx_shutdown(theContext));
   CALL_INT_FUNC(zmq_ctx_term(theContext));

   return 0;
}


