#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <uuid/uuid.h>

#include <string>
#include <map>
using namespace std;

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

   // create data sub
   void* dataSub = zmq_socket(theContext, ZMQ_SUB);
   CALL_INT_FUNC(zmq_socket_monitor_versioned(dataSub, "inproc://dataSub", ZMQ_EVENT_ALL, 2, ZMQ_PAIR));
   CALL_INT_FUNC (zmq_setsockopt (dataSub, ZMQ_RECONNECT_IVL, &interval, sizeof (interval)));
   CALL_INT_FUNC (zmq_setsockopt (dataSub, ZMQ_HANDSHAKE_IVL, &interval, sizeof (interval)));
   CALL_INT_FUNC(zmq_setsockopt(dataSub, ZMQ_RECONNECT_STOP, &stopReconnectOnError, sizeof(stopReconnectOnError)));
   CALL_INT_FUNC(zmq_setsockopt(dataSub, ZMQ_SUBSCRIBE, "", 0));

   char pubEndpoint[ZMQ_MAX_ENDPOINT_LENGTH +1];
   sprintf(pubEndpoint, "tcp://%s:%d", tcpAddr, port);
   CALL_INT_FUNC(zmq_connect(dataSub, pubEndpoint));

   sleep(120);

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
