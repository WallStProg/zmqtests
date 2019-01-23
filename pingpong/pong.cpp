#include <zmq.h>

#include "common.h"

#include "params.c"

int main(int argc, char** argv)
{
   parseParams(argc, argv);

   void* theContext = zmq_ctx_new();

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
      requestMsg requestMsg;
      CALL_INT_FUNC(zmq_recv(sub, &requestMsg, sizeof(requestMsg), 0));
      assert(strcmp(requestMsg.type, "request") == 0);

      strcpy(requestMsg.type, "ack");
      CALL_INT_FUNC(zmq_send(pub, &requestMsg, sizeof(requestMsg), ZMQ_DONTWAIT));

      strcpy(requestMsg.type, "response");
      CALL_INT_FUNC(zmq_send(pub, &requestMsg, sizeof(requestMsg), ZMQ_DONTWAIT));
   }

   CALL_INT_FUNC(zmq_close(pub));
   CALL_INT_FUNC(zmq_close(sub));


   CALL_INT_FUNC(zmq_ctx_shutdown(theContext));
   CALL_INT_FUNC(zmq_ctx_term(theContext));

   return 0;
}


