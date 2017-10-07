#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <zmq.h>

#define ZMQ_CONTROL_ENDPOINT  "inproc://control"

typedef struct zmqControlMsg {
   char     command;
   void*    threadAddr;
   long     seqNum;
} zmqControlMsg;

void* theContext = NULL;
bool  keepGoing = true;
pthread_t thread1;
pthread_t thread2;
long maxControlMsgReceived = 0;
long maxControlMsgSent = 0;
bool debugFlag = false;
long sleepDuration = 0;
int controlReceiver = ZMQ_PAIR;
int controlSender = ZMQ_PAIR;

extern "C" void onSignal(int sig)
{
   fprintf(stderr, "\nMax control msg sent %ld\n", maxControlMsgSent);
   fprintf(stderr, "\nMax control msg received %ld\n", maxControlMsgReceived);
   // abort & create core file
   assert(false);
}


void checkVoid(void* x)
{
   if (x == 0) {
      fprintf(stderr, "\nError %d(%s)\n", errno, zmq_strerror(errno));
      assert(false);
   }
}


void checkInt(int x)
{
   if (x < 0) {
      fprintf(stderr, "\nError %d(%s)\n", errno, zmq_strerror(errno));
      assert(false);
   }
}


// send a command to the control socket
void sendCommand(void* context, zmqControlMsg* msg, int msgSize)
{
   void* temp = zmq_socket(theContext, controlSender);
   checkVoid(temp);

   int rc = zmq_connect(temp, ZMQ_CONTROL_ENDPOINT);
   checkInt(rc);

   int i = zmq_send(temp, msg, msgSize, 0);
   checkInt(i);
   assert(i == msgSize);

   rc = zmq_close(temp);
   checkInt(rc);
}


// generate commands to control socket
void* commandLoop(void* threadAddr)
{
   while(keepGoing) {
      zmqControlMsg msg;
      msg.command = 'P';
      msg.threadAddr = threadAddr;
      msg.seqNum = ++maxControlMsgSent;
      if (debugFlag) {
         fprintf(stderr, "Sending command # %ld\n", msg.seqNum);
      }
      else {
         fprintf(stderr, ".");
      }
      sendCommand(theContext, &msg, sizeof(msg));

      usleep(sleepDuration);
   }

   return NULL;
}


void processControlMsg(zmq_msg_t* zmsg)
{
   zmqControlMsg* pMsg = (zmqControlMsg*) zmq_msg_data(zmsg);
   if (pMsg->seqNum != maxControlMsgReceived +1) {
      fprintf(stderr, "\nGot control msg from thread %x with seqNum %ld, previous seqNum was %ld\n", pMsg->threadAddr, pMsg->seqNum, maxControlMsgReceived);
   }
   if (pMsg->seqNum > maxControlMsgReceived) {
      maxControlMsgReceived = pMsg->seqNum;
   }
   if (debugFlag) {
      fprintf(stderr, "Got control msg from thread %x with seqNum %ld\n", pMsg->threadAddr, pMsg->seqNum);
   }
}

void processDataMsg(zmq_msg_t* zmsg)
{
}

void* mainLoop(void*)
{
   // control socket
   void* controlSub = zmq_socket(theContext, controlReceiver);
   if (controlReceiver == ZMQ_SUB) {
      zmq_setsockopt (controlSub, ZMQ_SUBSCRIBE, "", 0);
   }
   zmq_bind(controlSub, ZMQ_CONTROL_ENDPOINT);

   // data socket
   void* dataSub = zmq_socket(theContext, ZMQ_SUB);
   zmq_bind(dataSub, "tcp://127.0.0.1:5101");

   zmq_msg_t zmsg;
   zmq_msg_init(&zmsg);

   while (keepGoing) {

      zmq_pollitem_t items[] = {
         { controlSub, 0, ZMQ_POLLIN , 0},
         { dataSub, 0, ZMQ_POLLIN , 0}
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

   return NULL;
}


int main(int argc, char** argv)
{
   // get command line params
   for (int i = 0; i < argc; ++i) {
      if (strcasecmp("-sleep", argv[i]) == 0) {
         sleepDuration = atoi(argv[++i]);
      }
      else if (!strcmp("-debug", argv[i])) {
         debugFlag = true;
      }
      else if (!strcmp("-pubsub", argv[i])) {
         controlReceiver = ZMQ_SUB;
         controlSender = ZMQ_PUB;
      }
   }

   int major, minor, patch;
   zmq_version(&major, &minor, &patch);
   fprintf(stderr, "Using libzmq version %d.%d.%d\n", major, minor, patch);

   if (controlReceiver == ZMQ_SUB) {
      fprintf(stderr, "Using pub/sub sockets\n");
   }
   else {
      fprintf(stderr, "Using pair sockets\n");
   }
   fprintf(stderr, "Sleeping for %ld us between sends\n", sleepDuration);

   // setup signal handler
   signal(SIGINT, &onSignal);

   // initialize zmq
   theContext = zmq_ctx_new();

   // start main loop
   pthread_create(&thread1, NULL, mainLoop, NULL);

   // start command loop
   pthread_create(&thread2, NULL, commandLoop, &thread2);

   // wait for signal
   pause();

   return 0;
}