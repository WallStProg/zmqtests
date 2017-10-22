// Copyright 2017 by Bill Torpey. All Rights Reserved.
// This work is licensed under a Creative Commons Attribution-NonCommercial-NoDerivs 3.0 United States License.
// http://creativecommons.org/licenses/by-nc-nd/3.0/us/deed.en

#include <zmq.h>

#include "common.h"

// tls for control socket
pthread_key_t  key;


// send a command to the control socket
void sendCommand(void* context, zmqControlMsg* msg, int msgSize)
{
   void* controlPub = pthread_getspecific(key);
   if (controlPub == NULL) {
      controlPub = createSocket(theContext, ZMQ_PUB);
      int rc = zmq_connect(controlPub, CONTROL_ENDPOINT);
      checkInt(rc);

      if (pollFlag) {
         zmq_pollitem_t items[] = { { controlPub, 0, ZMQ_POLLIN, 0} };
         int rc = zmq_poll(items, 1, 1);
         checkInt(rc);
      }

      rc = pthread_setspecific(key, controlPub);
      checkInt(rc);
   }

   int i = zmq_send(controlPub, msg, msgSize, 0);
   checkInt(i);
   assert(i == msgSize);

   if (pollFlag) {
      zmq_pollitem_t items[] = { { controlPub, 0, ZMQ_POLLIN, 0} };
      int rc = zmq_poll(items, 1, 1);
      checkInt(rc);
   }
}


void* mainLoop(void*)
{
   int rc;

  // control socket
   void* controlSub = createSocket(theContext, ZMQ_SUB);
   rc = zmq_setsockopt (controlSub, ZMQ_SUBSCRIBE, "", 0);
   checkInt(rc);
   rc = zmq_bind(controlSub, CONTROL_ENDPOINT);
   checkInt(rc);

   // data socket
   void* dataSub = createSocket(theContext, ZMQ_SUB);
   rc = zmq_bind(dataSub, DATA_ENDPOINT);
   checkInt(rc);

   zmq_msg_t zmsg;
   zmq_msg_init(&zmsg);

   while (keepGoing) {

      zmq_pollitem_t items[] = {
         { controlSub, 0, ZMQ_POLLIN, 0},
         { dataSub,    0, ZMQ_POLLIN, 0}
      };
      int rc = zmq_poll(items, 2, -1);
      if (rc < 0) {
         break;
      }

      // got command msg?
      if (items[0].revents & ZMQ_POLLIN) {
         // controlSub
         int size = zmq_msg_recv(&zmsg, controlSub, 0);
         if (size != -1) {
            processControlMsg(&zmsg);
         }
         continue;
      }

      // got normal msg?
      if (items[1].revents & ZMQ_POLLIN) {
         int size = zmq_msg_recv(&zmsg, dataSub, 0);
         if (size != -1) {
            processDataMsg(&zmsg);
         }
         continue;
      }
   }

   closeSocket(controlSub);
   closeSocket(dataSub);

   return NULL;
}


int main(int argc, char** argv)
{
   int rc;

   parseParams(argc, argv);
   printVersion();

   fprintf(stderr, "Using pub/sub sockets\n");
   fprintf(stderr, "Sleeping for %ld seconds at shutdown\n", sleepDuration);
   if (pollFlag) {
      fprintf(stderr, "Polling after connect\n");
   }

   // setup signal handler
   signal(SIGINT, &onSignal);

   // initialize zmq
   theContext = zmq_ctx_new();
   checkVoid(theContext);
   rc = zmq_ctx_set(theContext, ZMQ_BLOCKY, false);
   checkInt(rc);

   // create the key for socket tls
   pthread_key_create(&key, cleanupSocket);

   // start main loop
   pthread_t mainThread;
   pthread_create(&mainThread, NULL, mainLoop, NULL);

   // start command loops
   pthread_t* commandThread = new pthread_t[numThreads];
   for (int i = 0; i < numThreads; ++i) {
      maxControlMsgSent[&commandThread[i]] = 0;
      maxControlMsgReceived[&commandThread[i]] = 0;
   }
   for (int i = 0; i < numThreads; ++i) {
      pthread_create(&commandThread[i], NULL, commandLoop, &commandThread[i]);
   }

   // wait for command threads to finish
   for (int i = 0; i < numThreads; ++i) {
      pthread_join(commandThread[i], NULL);
   }

   // sleep a bit
   fprintf(stderr, "Waiting...");
   usleep(sleepDuration*1000000);

   // send shutdown command
   pthread_t shutdownThread;
   pthread_create(&shutdownThread, NULL, shutdownFunc, NULL);
   pthread_join(shutdownThread, NULL);

   // wait for main thread to finish
   pthread_join(mainThread, NULL);

   rc = zmq_ctx_destroy(theContext);
   checkInt(rc);

   printResults();

   return 0;
}
