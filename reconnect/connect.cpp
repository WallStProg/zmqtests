#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <uuid/uuid.h>
#include <zmq.h>
#include <string>
#include <map>
using namespace std;

#include "common.h"
#include "monitor.h"
#include "params.c"


void* connectMonitor(void* context)
{
   int rc;

   void* dataSub = zmq_socket(context, ZMQ_PAIR);
   assert(dataSub);
   rc = zmq_connect(dataSub, "inproc://dataSub");
   assert(rc == 0);


   while(1) {
      zmq_pollitem_t items[] = {
         { dataSub,    0, ZMQ_POLLIN , 0},
      };
      rc = zmq_poll(items, 1, -1);

      if (items[0].revents & ZMQ_POLLIN) {
         rc = monitorEvent(dataSub, "dataSub");
         assert(rc == 0);
      }
   }

   rc = zmq_close(dataSub);
   assert(rc == 0);

   return NULL;
}



int main(int argc, char** argv)
{
   parseParams(argc, argv);

   void* theContext = zmq_ctx_new();

   int rc;
   rc = startMonitor(theContext, connectMonitor);
   assert(rc == 0);

   // create data sub
   void* dataSub = zmq_socket(theContext, ZMQ_SUB);
   CALL_INT_FUNC(zmq_socket_monitor(dataSub, "inproc://dataSub", ZMQ_EVENT_ALL));
   CALL_INT_FUNC(zmq_setsockopt(dataSub, ZMQ_RECONNECT_STOP, &stopReconnectOnError, sizeof(stopReconnectOnError)));
   CALL_INT_FUNC(zmq_setsockopt(dataSub, ZMQ_SUBSCRIBE, "", 0));

   char pubEndpoint[ZMQ_MAX_ENDPOINT_LENGTH +1];
   sprintf(pubEndpoint, "tcp://127.0.0.1:%d", port);
   CALL_INT_FUNC(zmq_connect(dataSub, pubEndpoint));

   sleep(60);

   rc = zmq_socket_monitor(dataSub, NULL, ZMQ_EVENT_ALL);
   assert(rc == 0);
   CALL_INT_FUNC(zmq_close(dataSub));

   rc = stopMonitor();
   assert(rc == 0);

   CALL_INT_FUNC(zmq_ctx_shutdown(theContext));
   CALL_INT_FUNC(zmq_ctx_term(theContext));

   log_msg("Exiting normally\n");

   return 0;
}
