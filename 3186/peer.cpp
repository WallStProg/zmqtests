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

   // create uuid
   uuid_t temp;
   uuid_generate(temp);
   char theUuid[UUID_STRING_SIZE+ 1];
   uuid_unparse(temp, theUuid);

   void* theContext = zmq_ctx_new();

   // bind data pub
   void* dataPub = zmq_socket(theContext, ZMQ_PUB);
   char pubEndpoint[ZMQ_MAX_ENDPOINT_LENGTH +1];
   sprintf(pubEndpoint, "tcp://127.0.0.1:%d", port);
   CALL_INT_FUNC(zmq_bind(dataPub, pubEndpoint));
   size_t nameSize = sizeof(pubEndpoint);
   CALL_INT_FUNC(zmq_getsockopt(dataPub, ZMQ_LAST_ENDPOINT, pubEndpoint, &nameSize));

   // create proxy pub
   void* proxyPub = zmq_socket(theContext, ZMQ_PUB);

   // connect to proxy sub
   void* proxySub = zmq_socket(theContext, ZMQ_SUB);
   CALL_INT_FUNC(zmq_setsockopt(proxySub, ZMQ_SUBSCRIBE, "", 0));
   CALL_INT_FUNC(zmq_connect(proxySub, "tcp://127.0.0.1:5555"));

   // create data sub
   void* dataSub = zmq_socket(theContext, ZMQ_SUB);
   if (disableReconnect == 1)  {
      log_msg("Disabling reconnect on dataSub");
      int reconnectInterval = -1;
      CALL_INT_FUNC(zmq_setsockopt(dataSub, ZMQ_RECONNECT_IVL, &reconnectInterval, sizeof(reconnectInterval)));
   }
   CALL_INT_FUNC(zmq_setsockopt(dataSub, ZMQ_SUBSCRIBE, "", 0));

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

      CALL_INT_FUNC(zmq_poll(pollitems, 2, 1000));

      // doesn't work -- see https://github.com/zeromq/libzmq/issues/1256#issuecomment-64212673
      if (fix1256 == 1) {
         log_msg("Calling zmq_getsockopt w/ZMQ_EVENTS");
         size_t fd_size = sizeof(uint32_t);
         uint32_t fd;
         zmq_getsockopt (pollitems[1].socket, ZMQ_EVENTS, &fd, &fd_size);
      }

      // keep sending a connect msg until we connect to our own pub
      if (peers.find(theUuid) == peers.end()) {
         sendConnectMsg(proxyPub, 'C', theUuid, pubEndpoint);
      }

      if (pollitems[0].revents & ZMQ_POLLIN) {
         connectMsg msg;
         CALL_INT_FUNC(zmq_recv(proxySub, &msg, sizeof(msg), 0));

         log_msg("Got %c from %s with endpoint %s\n", msg.command, msg.uuid, msg.endpoint);

         _peers::iterator it = peers.find(msg.uuid);

         if (msg.command == 'W') {
            // welcome msg from proxy
            log_msg("Connecting proxy pub to: %s\n", msg.endpoint);
            CALL_INT_FUNC(zmq_connect(proxyPub, msg.endpoint));
         }
         else if (msg.command == 'C') {
            // connect msg from peer (possibly us)
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
            if (it != peers.end()) {
               log_msg("Disconnecting data sub from %s", msg.endpoint);
               peers.erase(it);
               CALL_INT_FUNC(zmq_disconnect(dataSub, msg.endpoint));
               CALL_INT_FUNC(kickSocket(dataPub));

               // if we got our own disconnect msg, quit
               if (strcmp(msg.uuid, theUuid) == 0) {
                  break;
               }

               if (sendAfterDisconnect == 1) {
                  // send dummy message to PUB socket to force cleanup
                  log_msg("Sending dummy msg on dataPub");
                  CALL_INT_FUNC(zmq_send(dataPub, NULL, 0, ZMQ_DONTWAIT));
               }
            }
         }
      }

      if (pollitems[1].revents & ZMQ_POLLIN) {
         dummyMsg dummyMsg;
         CALL_INT_FUNC(zmq_recv(dataSub, &dummyMsg, sizeof(dummyMsg), 0));
      }
   }

   if (sleepAtExit > 0) {
      log_msg("Sleeping for %d seconds", sleepAtExit);
      usleep(sleepAtExit*ONE_MILLION);
   }



   CALL_INT_FUNC(zmq_setsockopt(dataSub, ZMQ_LINGER, &linger, sizeof(linger)));
   //zmq_pollitem_t pollDataSub [] = { { dataSub, 0, ZMQ_POLLIN , 0} };
   //CALL_INT_FUNC(zmq_poll(pollDataSub, 1, 1));
   CALL_INT_FUNC(zmq_close(dataSub));

   CALL_INT_FUNC(zmq_setsockopt(proxySub, ZMQ_LINGER, &linger, sizeof(linger)));
   //zmq_pollitem_t pollProxySub [] = { { proxySub, 0, ZMQ_POLLIN , 0} };
   //CALL_INT_FUNC(zmq_poll(pollProxySub, 1, 1));
   CALL_INT_FUNC(zmq_close(proxySub));

   CALL_INT_FUNC(zmq_setsockopt(dataPub, ZMQ_LINGER, &linger, sizeof(linger)));
   //zmq_pollitem_t pollDataPub [] = { { dataPub, 0, ZMQ_POLLIN , 0} };
   //CALL_INT_FUNC(zmq_poll(pollDataPub, 1, 1));
   CALL_INT_FUNC(zmq_close(dataPub));

   CALL_INT_FUNC(zmq_setsockopt(proxyPub, ZMQ_LINGER, &linger, sizeof(linger)));
   //zmq_pollitem_t pollProxyPub [] = { { proxyPub, 0, ZMQ_POLLIN , 0} };
   //CALL_INT_FUNC(zmq_poll(pollProxyPub, 1, 1));
   CALL_INT_FUNC(zmq_close(proxyPub));

   if (sleepAtExit > 0) {
      log_msg("Sleeping for %d seconds", sleepAtExit);
      usleep(sleepAtExit*ONE_MILLION);
   }

   CALL_INT_FUNC(zmq_ctx_term(theContext));

   if (sleepAtExit > 0) {
      log_msg("Sleeping for %d seconds", sleepAtExit);
      usleep(sleepAtExit*ONE_MILLION);
   }

   log_msg("Exiting normally\n");

   return 0;
}


