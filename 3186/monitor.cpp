#define ZMQ_BUILD_DRAFT_API
#include <zmq.h>
#include <pthread.h>
#include <assert.h>
#include <memory.h>

#include "monitor.h"
#include "common.h"

pthread_t   monitorThread;
void*       monitorSub;
void*       monitorPub;


/*  DRAFT 0MQ socket events and monitoring                                    */
/*  Unspecified system errors during handshake. Event value is an errno.      */
#define ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL   0x0800
/*  Handshake complete successfully with successful authentication (if        *
 *  enabled). Event value is unused.                                          */
#define ZMQ_EVENT_HANDSHAKE_SUCCEEDED          0x1000
/*  Protocol errors between ZMTP peers or between server and ZAP handler.     *
 *  Event value is one of ZMQ_PROTOCOL_ERROR_*                                */
#define ZMQ_EVENT_HANDSHAKE_FAILED_PROTOCOL    0x2000
/*  Failed authentication requests. Event value is the numeric ZAP status     *
 *  code, i.e. 300, 400 or 500.                                               */
#define ZMQ_EVENT_HANDSHAKE_FAILED_AUTH        0x4000

const char* get_zmqEventName(int event)
{
   switch(event) {
      case ZMQ_EVENT_CONNECTED                  : return "CONNECTED";
      case ZMQ_EVENT_CONNECT_DELAYED            : return "CONNECT_DELAYED";
      case ZMQ_EVENT_CONNECT_RETRIED            : return "CONNECT_RETRIED";
      case ZMQ_EVENT_LISTENING                  : return "LISTENING";
      case ZMQ_EVENT_BIND_FAILED                : return "BIND_FAILED";
      case ZMQ_EVENT_ACCEPTED                   : return "ACCEPTED";
      case ZMQ_EVENT_ACCEPT_FAILED              : return "ACCEPT_FAILED";
      case ZMQ_EVENT_CLOSED                     : return "CLOSED";
      case ZMQ_EVENT_CLOSE_FAILED               : return "CLOSE_FAILED";
      case ZMQ_EVENT_DISCONNECTED               : return "DISCONNECTED";
      case ZMQ_EVENT_MONITOR_STOPPED            : return "MONITOR_STOPPED";
      case ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL : return "HANDSHAKE_FAILED_NO_DETAIL";
      case ZMQ_EVENT_HANDSHAKE_SUCCEEDED        : return "HANDSHAKE_SUCCEEDED";
      case ZMQ_EVENT_HANDSHAKE_FAILED_PROTOCOL  : return "HANDSHAKE_FAILED_PROTOCOL";
      case ZMQ_EVENT_HANDSHAKE_FAILED_AUTH      : return "HANDSHAKE_FAILED_AUTH";
      default                                   : return "UNKNOWN";
   }
}


typedef struct __attribute__ ((packed)) zmq_monitor_frame1 {
   uint16_t    event;
   uint32_t    value;
} zmq_monitor_frame1;


#define     ZMQ_MAX_ENDPOINT_LENGTH          256

int monitorEvent(void* socket, const char* name)
{
   // First frame in message contains event number and value
   zmq_msg_t msg;
   zmq_msg_init (&msg);
   if (zmq_msg_recv (&msg, socket, 0) == -1)
      return -1; // Interrupted, presumably
   assert (zmq_msg_more (&msg));

   zmq_monitor_frame1* pFrame1 = (zmq_monitor_frame1*) zmq_msg_data (&msg);
   int event = pFrame1->event;
   int value = pFrame1->value;
   const char* eventName = get_zmqEventName(event);

   // Second frame in message contains event address
   zmq_msg_init (&msg);
   if (zmq_msg_recv (&msg, socket, 0) == -1)
      return -1; // Interrupted, presumably
   assert (!zmq_msg_more (&msg));

   uint8_t* data = (uint8_t *) zmq_msg_data (&msg);
   size_t size = zmq_msg_size(&msg);
   char endpoint[ZMQ_MAX_ENDPOINT_LENGTH +1];
   memset(endpoint, '\0', sizeof(endpoint));
   memcpy(endpoint, data, size);

   log_msg("socket:%p name:%s value:%d event:%d desc:%s endpoint:%s", socket, name, value, event, eventName, endpoint);

   return 0;
}

void* monitorFunc(void* context)
{
   int rc;

   void* dataPub = zmq_socket(context, ZMQ_PAIR);
   assert(dataPub);
   rc = zmq_connect(dataPub, "inproc://dataPub");
   assert(rc == 0);

   void* dataSub = zmq_socket(context, ZMQ_PAIR);
   assert(dataSub);
   rc = zmq_connect(dataSub, "inproc://dataSub");
   assert(rc == 0);

   void* proxyPub = zmq_socket(context, ZMQ_PAIR);
   assert(proxyPub);
   rc = zmq_connect(proxyPub, "inproc://proxyPub");
   assert(rc == 0);

   void* proxySub = zmq_socket(context, ZMQ_PAIR);
   assert(proxySub);
   rc = zmq_connect(proxySub, "inproc://proxySub");
   assert(rc == 0);

   while(1) {
      zmq_pollitem_t items[] = {
         { dataPub,    0, ZMQ_POLLIN , 0},
         { dataSub,    0, ZMQ_POLLIN , 0},
         { proxyPub,   0, ZMQ_POLLIN , 0},
         { proxySub,   0, ZMQ_POLLIN , 0},
         { monitorSub, 0, ZMQ_POLLIN , 0},
      };
      rc = zmq_poll(items, 5, -1);

      if (items[0].revents & ZMQ_POLLIN) {
         rc = monitorEvent(dataPub, "dataPub");
         assert(rc == 0);
      }

      if (items[1].revents & ZMQ_POLLIN) {
         rc = monitorEvent(dataSub, "dataSub");
         assert(rc == 0);
      }

      if (items[2].revents & ZMQ_POLLIN) {
         rc = monitorEvent(proxyPub, "proxyPub");
         assert(rc == 0);
      }

      if (items[3].revents & ZMQ_POLLIN) {
         rc = monitorEvent(proxySub, "proxySub");
         assert(rc == 0);
      }

      if (items[4].revents & ZMQ_POLLIN) {
         break;
      }
   }


   rc = zmq_close(dataPub);
   assert(rc == 0);

   rc = zmq_close(dataSub);
   assert(rc == 0);

   rc = zmq_close(proxyPub);
   assert(rc == 0);

   rc = zmq_close(proxySub);
   assert(rc == 0);

   return NULL;
}


int startMonitor(void* context)
{
   int rc;

   monitorSub = zmq_socket(context, ZMQ_SERVER);
   assert(monitorSub);
   rc = zmq_bind(monitorSub, "inproc://monitor");
   assert(rc == 0);

   monitorPub = zmq_socket(context, ZMQ_CLIENT);
   assert(monitorPub);
   rc = zmq_connect(monitorPub, "inproc://monitor");
   assert(rc == 0);

   rc = pthread_create(&monitorThread, NULL, monitorFunc, context);
   assert(rc == 0);

   return 0;
}


int stopMonitor()
{
   int rc;

   int bytes = zmq_send(monitorPub, "X", 1, 0);
   assert(bytes == 1);

   rc = pthread_join(monitorThread, NULL);
   assert(rc == 0);

   rc = zmq_close(monitorPub);
   assert(rc == 0);

   rc = zmq_close(monitorSub);
   assert(rc == 0);

   return 0;
}

