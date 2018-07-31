//
// This code was cribbed from "The ZeroMQ Guide - for C Developers" -- Example 2.7 Weather Update proxy
//

// #define _GNU_SOURCE

//#include <unistd.h>
// #include <stdlib.h>
#include <signal.h>
#include <uuid/uuid.h>
#include <string.h>

#include <zmq.h>
#include "common.h"

void sighandler(int unused)
{
   unused;
}

int main (int argc, char** argv)
{
   // create uuid
   uuid_t temp;
   uuid_generate(temp);
   char theUuid[UUID_STRING_SIZE+ 1];
   uuid_unparse(temp, theUuid);

   void* theContext = zmq_ctx_new ();

   // bind sub and get endpoint
   void* frontend = zmq_socket(theContext, ZMQ_XSUB);
   zmq_bind (frontend, "tcp://127.0.0.1:*");
   char subEndpoint[ZMQ_MAX_ENDPOINT_LENGTH];
   size_t nameSize = sizeof(subEndpoint);
   zmq_getsockopt(frontend, ZMQ_LAST_ENDPOINT, subEndpoint, &nameSize);

   // bind pub to well-known address
   void* backend = zmq_socket(theContext, ZMQ_XPUB);

   // set welcome msg to be delivered when sub connects
   connectMsg welcomeMsg;
   welcomeMsg.command = 'W';
   strcpy(welcomeMsg.uuid, theUuid);
   strcpy(welcomeMsg.endpoint, subEndpoint);
   zmq_setsockopt(backend, ZMQ_XPUB_WELCOME_MSG, &welcomeMsg, sizeof(welcomeMsg));

   // bind the backend socket to pub endpoint
   zmq_bind(backend, "tcp://127.0.0.1:5555");

   //  Run the proxy until the user interrupts us
   signal(SIGINT, &sighandler);
   zmq_proxy(frontend, backend, NULL);

   zmq_close(frontend);
   zmq_close(backend);
   zmq_ctx_destroy(theContext);

   return 0;
}
