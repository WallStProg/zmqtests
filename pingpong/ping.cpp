#include <unistd.h>

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
   sprintf(pubEndpoint, "tcp://127.0.0.1:%d", pingPort);
   CALL_INT_FUNC(zmq_bind(pub, pubEndpoint));

   // connect sub
   void* sub = zmq_socket(theContext, ZMQ_SUB);
   CALL_INT_FUNC(zmq_setsockopt(sub, ZMQ_SUBSCRIBE, "", 0));
   char subEndpoint[ZMQ_MAX_ENDPOINT_LENGTH +1];
   sprintf(subEndpoint, "tcp://127.0.0.1:%d", pongPort);
   CALL_INT_FUNC(zmq_connect(sub, subEndpoint));

   // wait for connects
   sleep(1);
   CALL_INT_FUNC(kickSocket(pub));

   int requestNum = 0;
   long start = getMicros();

   while(++requestNum < numRequests) {
      requestMsg requestMsg;
      strcpy(requestMsg.topic, "TOPIC");
      strcpy(requestMsg.type, "request");
      CALL_INT_FUNC(zmq_send(pub, &requestMsg, sizeof(requestMsg), 0));

      CALL_INT_FUNC(zmq_recv(sub, &requestMsg, sizeof(requestMsg), 0));
      assert(strcmp(requestMsg.type, "ack") == 0);

      CALL_INT_FUNC(zmq_recv(sub, &requestMsg, sizeof(requestMsg), 0));
      assert(strcmp(requestMsg.type, "response") == 0);
   }
   long end = getMicros();
   int latency = (end - start) / numRequests;
   printf ("Avg. latency=%d micros\n", latency);


   CALL_INT_FUNC(zmq_close(pub));
   CALL_INT_FUNC(zmq_close(sub));


   CALL_INT_FUNC(zmq_ctx_shutdown(theContext));
   CALL_INT_FUNC(zmq_ctx_term(theContext));

   return 0;
}


