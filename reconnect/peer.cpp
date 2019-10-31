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

void sendConnectMsg(void* socket, const char command, const char* uuid, const char* endpoint)
{
   connectMsg msg;
   msg.command = command;
   strcpy(msg.uuid, uuid);
   strcpy(msg.endpoint, endpoint);
   zmq_send(socket, &msg, sizeof(msg), 0);
}


int main(int argc, char** argv)
{
   parseParams(argc, argv);

   pid_t pid = getpid();
   log_msg("Process id=%jd\n", pid);

   // create uuid
   uuid_t temp;
   uuid_generate(temp);
   char theUuid[UUID_STRING_SIZE+ 1];
   uuid_unparse(temp, theUuid);

   void* theContext = zmq_ctx_new();

   int rc;
   rc = startMonitor(theContext);
   assert(rc == 0);


   // bind data pub
   void* dataPub = zmq_socket(theContext, ZMQ_PUB);
   CALL_INT_FUNC(zmq_socket_monitor(dataPub, "inproc://dataPub", ZMQ_EVENT_ALL));

   char pubEndpoint[ZMQ_MAX_ENDPOINT_LENGTH +1];
   sprintf(pubEndpoint, "tcp://%s:%d", tcpAddr, port);
   CALL_INT_FUNC(zmq_bind(dataPub, pubEndpoint));
   size_t nameSize = sizeof(pubEndpoint);
   CALL_INT_FUNC(zmq_getsockopt(dataPub, ZMQ_LAST_ENDPOINT, pubEndpoint, &nameSize));

   // create proxy pub
   void* proxyPub = zmq_socket(theContext, ZMQ_PUB);
   CALL_INT_FUNC(zmq_socket_monitor(proxyPub, "inproc://proxyPub", ZMQ_EVENT_ALL));

   // connect to proxy sub at "well-known" port
   void* proxySub = zmq_socket(theContext, ZMQ_SUB);
   CALL_INT_FUNC(zmq_socket_monitor(proxySub, "inproc://proxySub", ZMQ_EVENT_ALL));
   CALL_INT_FUNC(zmq_setsockopt(proxySub, ZMQ_SUBSCRIBE, "", 0));
   char proxyEndpoint[ZMQ_MAX_ENDPOINT_LENGTH +1];
   sprintf(proxyEndpoint, "tcp://%s:5555", tcpAddr);
   CALL_INT_FUNC(zmq_connect(proxySub, proxyEndpoint));

   // create data sub
   void* dataSub = zmq_socket(theContext, ZMQ_SUB);
   CALL_INT_FUNC(zmq_socket_monitor(dataSub, "inproc://dataSub", ZMQ_EVENT_ALL));
   int heartbeatInterval = 1000;
   CALL_INT_FUNC(zmq_setsockopt(dataSub, ZMQ_HEARTBEAT_IVL, &heartbeatInterval, sizeof(heartbeatInterval)));
   CALL_INT_FUNC(zmq_setsockopt(dataSub, ZMQ_RECONNECT_STOP, &stopReconnectOnError, sizeof(stopReconnectOnError)));
   CALL_INT_FUNC(zmq_setsockopt(dataSub, ZMQ_SUBSCRIBE, "", 0));

   typedef map<string, string> _peers;
   _peers peers;
   while(1) {

      zmq_pollitem_t pollitems[] = {
         { proxySub, 0, ZMQ_POLLIN , 0},
         { dataSub,  0, ZMQ_POLLIN , 0}
      };

      CALL_INT_FUNC(zmq_poll(pollitems, 2, 1000));

      // keep sending a connect msg until we connect to our own pub
      if (peers.find(theUuid) == peers.end()) {
         sendConnectMsg(proxyPub, 'C', theUuid, pubEndpoint);
      }

      if (pollitems[0].revents & ZMQ_POLLIN) {
         connectMsg msg;
         CALL_INT_FUNC(zmq_recv(proxySub, &msg, sizeof(msg), 0));

         log_msg("Got %c from %s with endpoint %s\n", msg.command, msg.uuid, msg.endpoint);

         if (msg.command == 'W') {
            // welcome msg from proxy
            log_msg("Connecting proxy pub to: %s\n", msg.endpoint);
            CALL_INT_FUNC(zmq_setsockopt(proxyPub, ZMQ_RECONNECT_STOP, &stopReconnectOnError, sizeof(stopReconnectOnError)));
            CALL_INT_FUNC(zmq_connect(proxyPub, msg.endpoint));
         }
         else if (msg.command == 'C') {
            // connect msg from peer (possibly us)
            _peers::iterator it = peers.find(msg.uuid);
            if (it == peers.end()) {
               log_msg("Connecting data sub to %s\n", msg.endpoint);
               peers[msg.uuid] = msg.endpoint;
               CALL_INT_FUNC(zmq_connect(dataSub, msg.endpoint));
               // only publish endpoint if msg is not from us
               if (strcmp(msg.uuid, theUuid) != 0) {
                  sendConnectMsg(proxyPub, 'C', theUuid, pubEndpoint);
               }
            }
         }
         else if (msg.command == 'D') {
            // disconnect msg from peer (possibly us)
            _peers::iterator it = peers.find(msg.uuid);
            if (it != peers.end()) {
               log_msg("Disconnecting data sub from %s", msg.endpoint);
               peers.erase(it);
               CALL_INT_FUNC(zmq_disconnect(dataSub, msg.endpoint));
               // if we got our own disconnect msg, quit
               if (strcmp(msg.uuid, theUuid) == 0) {
                  log_msg("Exiting - got disconnect msg");
                  break;
               }

            }
         }
      }

      if (pollitems[1].revents & ZMQ_POLLIN) {
         dummyMsg dummyMsg;
         CALL_INT_FUNC(zmq_recv(dataSub, &dummyMsg, sizeof(dummyMsg), 0));
      }
   }

   rc = zmq_socket_monitor(dataPub, NULL, ZMQ_EVENT_ALL);
   assert(rc == 0);
   CALL_INT_FUNC(zmq_close(dataPub));

   rc = zmq_socket_monitor(dataSub, NULL, ZMQ_EVENT_ALL);
   assert(rc == 0);
   CALL_INT_FUNC(zmq_close(dataSub));

   rc = zmq_socket_monitor(proxyPub, NULL, ZMQ_EVENT_ALL);
   assert(rc == 0);
   CALL_INT_FUNC(zmq_close(proxyPub));

   rc = zmq_socket_monitor(proxySub, NULL, ZMQ_EVENT_ALL);
   assert(rc == 0);
   CALL_INT_FUNC(zmq_close(proxySub));

   rc = stopMonitor();
   assert(rc == 0);

   CALL_INT_FUNC(zmq_ctx_shutdown(theContext));
   CALL_INT_FUNC(zmq_ctx_term(theContext));

   log_msg("Exiting normally\n");

   return 0;
}
