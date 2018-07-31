#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <uuid/uuid.h>
#include <zmq.h>
#include <string>
#include <map>
using namespace std;
#include "common.h"

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
   // get params
   int seconds = 0;
   for (int i = 1; i < argc; i++) {
      if (strcasecmp("-s", argv[i]) == 0) {
         seconds = atoi(argv[++i]);
      }
   }

   // create uuid
   uuid_t temp;
   uuid_generate(temp);
   char theUuid[UUID_STRING_SIZE+ 1];
   uuid_unparse(temp, theUuid);

   void* theContext = zmq_ctx_new();

   // bind data pub
   void* dataPub = zmq_socket(theContext, ZMQ_PUB);
   zmq_bind(dataPub, "tcp://127.0.0.1:*");
   char pubEndpoint[ZMQ_MAX_ENDPOINT_LENGTH +1];
   size_t nameSize = sizeof(pubEndpoint);
   zmq_getsockopt(dataPub, ZMQ_LAST_ENDPOINT, pubEndpoint, &nameSize);

   // create proxy pub
   void* proxyPub = zmq_socket(theContext, ZMQ_PUB);

   // connect to proxy sub
   void* proxySub = zmq_socket(theContext, ZMQ_SUB);
   zmq_setsockopt(proxySub, ZMQ_SUBSCRIBE, "", 0);
   zmq_connect(proxySub, "tcp://127.0.0.1:5555");

   // create data sub
   void* dataSub = zmq_socket(theContext, ZMQ_SUB);
   zmq_setsockopt(dataSub, ZMQ_SUBSCRIBE, "", 0);

   typedef map<string, string> _peers;
   _peers peers;

   time_t endTime = 0;
   if (seconds > 0) {
      endTime = time(NULL) + seconds;
   }

   while(1) {

      if ((endTime > 0) && (time(NULL) > endTime)) {
         sendConnectMsg(proxyPub, 'D', theUuid, pubEndpoint);
      }

      zmq_pollitem_t pollitems[] = {
         { proxySub, 0, ZMQ_POLLIN , 0},
         { dataSub,  0, ZMQ_POLLIN , 0}
      };

      int rc = zmq_poll(pollitems, 2, 1000);

      // keep sending a connect msg until we connect to our own pub
      if (peers.find(theUuid) == peers.end()) {
         sendConnectMsg(proxyPub, 'C', theUuid, pubEndpoint);
      }

      if (pollitems[0].revents & ZMQ_POLLIN) {
         connectMsg msg;
         zmq_recv(proxySub, &msg, sizeof(msg), 0);

         fprintf(stderr, "Got %c from %s with endpoint %s\n", msg.command, msg.uuid, msg.endpoint);

         _peers::iterator it = peers.find(msg.uuid);

         if (msg.command == 'W') {
            // welcome msg from proxy
            fprintf(stderr, "Connecting proxy pub to: %s\n", msg.endpoint);
            zmq_connect(proxyPub, msg.endpoint);
         }
         else if (msg.command == 'C') {
            // connect msg from peer (possibly us)
            if (it == peers.end()) {
               fprintf(stderr, "Connecting data sub to %s\n", msg.endpoint);
               peers[msg.uuid] = msg.endpoint;
               zmq_connect(dataSub, msg.endpoint);
               if (strcmp(msg.uuid, theUuid) != 0) {
                  sendConnectMsg(proxyPub, 'C', theUuid, pubEndpoint);
               }
            }
         }
         else if (msg.command == 'D') {
            if (it != peers.end()) {
               fprintf(stderr, "Disconnecting data sub from %s\n", msg.endpoint);
               peers.erase(it);
               zmq_disconnect(dataSub, msg.endpoint);
               // if we got our own disconnect msg, quit
               if (strcmp(msg.uuid, theUuid) == 0) {
                  break;
               }
            }
         }
      }

      if (pollitems[1].revents & ZMQ_POLLIN) {
         dummyMsg dummyMsg;
         zmq_recv(dataSub, &dummyMsg, sizeof(dummyMsg), 0);
      }
   }

   zmq_close(dataSub);
   zmq_close(proxySub);
   zmq_close(dataPub);
   zmq_close(proxyPub);

   zmq_ctx_term(theContext);

   return 0;
}


