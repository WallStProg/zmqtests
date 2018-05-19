// includes
#include <assert.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <map>

// globals
void* theContext = NULL;
bool  keepGoing = true;
bool  debugFlag = false;
bool  quietFlag = false;
bool  pollFlag = false;
long  sleepDuration = 1;
int   numMsgs = 1000;
int   numThreads = 10;
bool  failed = false;
bool  dontWaitFlag = false;
int   linger = 30000;
int   msgSleep = 0;
bool  mutexFlag = false;

// max sent/received
std::map<void*, long> maxControlMsgSent;
std::map<void*, long> maxControlMsgReceived;


#define CONTROL_ENDPOINT   "inproc://control"
#define DATA_ENDPOINT      "tcp://127.0.0.1:5101"

#define PING_COMMAND       'P'
#define EXIT_COMMAND       'X'

typedef struct zmqControlMsg {
   char     command;
   void*    voidArg;
   long     longArg;
} zmqControlMsg;


void checkVoid(void* x)
{
   if (x == 0) {
      fprintf(stderr, "\nError %d (%s)\n", errno, zmq_strerror(errno));
      assert(false);
   }
}


void checkInt(int x)
{
   if (x < 0) {
      fprintf(stderr, "\nError %d (%s)\n", errno, zmq_strerror(errno));
      assert(false);
   }
}


void* createSocket(void* context, int type)
{
   void* sock = zmq_socket(context, type);
   checkVoid(sock);
   if (debugFlag)
      fprintf(stderr, "\nCreated socket %x\n", sock);

   int rc = zmq_setsockopt(sock, ZMQ_LINGER, &linger, sizeof(linger));
   return sock;
}


void closeSocket(void* sock)
{
   int rc = zmq_close(sock);
   checkInt(rc);
   if (debugFlag)
      fprintf(stderr, "\nClosed socket %x\n", sock);
}

void sendCommand(void* context, zmqControlMsg* msg, int msgSize);
void sendControlMsg(void* context, char command, void* voidArg, long longArg)
{
   zmqControlMsg msg;
   msg.command = command;
   msg.voidArg = voidArg;
   msg.longArg = longArg;
   sendCommand(context, &msg, sizeof(msg));
}


// generate commands to control socket
void* commandLoop(void* threadAddr)
{
   while (keepGoing) {
      long nextMsgNum = maxControlMsgSent[threadAddr];
      nextMsgNum++;
      maxControlMsgSent[threadAddr] = nextMsgNum;
      sendControlMsg(theContext, PING_COMMAND, threadAddr, nextMsgNum);
      if (debugFlag) {
         fprintf(stderr, "Sending command # %ld\n", nextMsgNum);
      }
      else if (!quietFlag) {
         fprintf(stderr, ".");
      }

      usleep(msgSleep);

      if (nextMsgNum >= numMsgs) {
         break;
      }
   }

   return NULL;
}


void processControlMsg(zmq_msg_t* zmsg)
{
   zmqControlMsg* pMsg = (zmqControlMsg*) zmq_msg_data(zmsg);

   if (pMsg->command == EXIT_COMMAND) {
      if (debugFlag)
         fprintf(stderr, "\nGot exit msg\n");
      keepGoing = false;
   }
   else if (pMsg->command == PING_COMMAND) {
      void* threadAddr = pMsg->voidArg;
      long msgNum = pMsg->longArg;

      long maxMsgNum = maxControlMsgReceived[threadAddr];
      if (msgNum != maxMsgNum+1) {
         failed = true;
         fprintf(stderr, "Thread %x received # %ld, expected # %ld\n", threadAddr, msgNum, maxMsgNum+1);
      }
      else if (debugFlag) {
         fprintf(stderr, "Thread %x received # %ld\n", threadAddr, msgNum);
      }
      if (msgNum > maxMsgNum) {
         maxControlMsgReceived[threadAddr] = msgNum;
      }
   }
   else {
      fprintf(stderr, "\nGot unknown control msg: %c \n", pMsg->command);
   }
}


void processDataMsg(zmq_msg_t* zmsg)
{
}

void checkResults()
{
   for (std::map<void*, long>::iterator i = maxControlMsgSent.begin(); i != maxControlMsgSent.end(); ++i) {
      if (i->second != numMsgs) {
         failed = true;
      }
   }
   for (std::map<void*, long>::iterator i = maxControlMsgReceived.begin(); i != maxControlMsgReceived.end(); ++i) {
      if (i->second != numMsgs) {
         failed = true;
      }
   }
}


void printResults()
{
   // print results
   fprintf(stderr, "\n");
   for (std::map<void*, long>::iterator i = maxControlMsgSent.begin(); i != maxControlMsgSent.end(); ++i) {
      if (!quietFlag || (maxControlMsgReceived[i->first] != i->second)) {
         fprintf(stderr, "Thread %x sent # %ld, received # %ld\n", i->first, i->second, maxControlMsgReceived[i->first]);
      }
   }
}


extern "C" void onSignal(int sig)
{
   quietFlag = false;
   printResults();
   exit(1);
}


void cleanupSocket(void* socket)
{
   closeSocket(socket);
}


void* shutdownFunc(void*)
{
   // send shutdown command
   sendControlMsg(theContext, EXIT_COMMAND, NULL, 0);
}

void parseParams(int argc, char** argv)
{
   // get command line params
   for (int i = 0; i < argc; ++i) {
      if (strcasecmp("-sleep", argv[i]) == 0) {
         sleepDuration = atoi(argv[++i]);
      }
      else if (strcasecmp("-msgs", argv[i]) == 0) {
         numMsgs = atoi(argv[++i]);
      }
      else if (strcasecmp("-linger", argv[i]) == 0) {
         linger = atoi(argv[++i]);
      }
      else if (strcasecmp("-rate", argv[i]) == 0) {
         int rate = atoi(argv[++i]);
         msgSleep = (1.0 / rate) * 1000000;            // sleep in usec
      }
      else if (strcasecmp("-threads", argv[i]) == 0) {
         numThreads = atoi(argv[++i]);
      }
      else if (!strcmp("-debug", argv[i])) {
         debugFlag = true;
      }
      else if (!strcmp("-mutex", argv[i])) {
         mutexFlag = true;
      }
      else if (!strcmp("-poll", argv[i])) {
         pollFlag = true;
      }
      else if (!strcmp("-nowait", argv[i])) {
         dontWaitFlag = true;
      }
      else if (!strcmp("-quiet", argv[i])) {
         quietFlag = true;
      }
   }

   fprintf(stderr, "Inter-message sleep = %d us\n", msgSleep);
   if (pollFlag) {
      fprintf(stderr, "Polling after connect\n");
   }
   if (mutexFlag) {
      fprintf(stderr, "Serializing send w/mutex\n");
   }
   fprintf(stderr, "Sleeping for %ld seconds at shutdown\n", sleepDuration);
}

void printVersion()
{
   int major, minor, patch;
   zmq_version(&major, &minor, &patch);
   fprintf(stderr, "Using libzmq version %d.%d.%d\n", major, minor, patch);
}
