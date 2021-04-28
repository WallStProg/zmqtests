#include <stdlib.h>
#include <string.h>

int port = 0;
int stopReconnectOnError = 0;
char* tcpAddr = NULL;
int interval = 1000;

void parseParams(int argc, char** argv)
{
   for (int i = 1; i < argc; i++) {
      if (strncasecmp("-addr", argv[i], strlen(argv[i])) == 0) {
         tcpAddr = argv[++i];
         log_msg("Param:-addr=%s", tcpAddr);
      }
      if (strncasecmp("-port", argv[i], strlen(argv[i])) == 0) {
         port = atoi(argv[++i]);
         log_msg("Param:port=%d", port);
      }
      if (strncasecmp("-interval", argv[i], strlen(argv[i])) == 0) {
         interval = atoi(argv[++i]);
         log_msg("Param:interval=%d", interval);
      }
      if (strncasecmp("-stop-reconnect-on", argv[i], strlen(argv[i])) == 0) {
         stopReconnectOnError = atoi(argv[++i]);
         log_msg("Param:stopReconnectOnError=%d", stopReconnectOnError);
      }
   }

   if (tcpAddr == NULL)
      tcpAddr = strdup("127.0.0.1");
}
