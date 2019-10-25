#include <string.h>

int port = 0;
int stopReconnectOnError = 0;

void parseParams(int argc, char** argv)
{
   for (int i = 1; i < argc; i++) {
      if (strncasecmp("-port", argv[i], strlen(argv[i])) == 0) {
         port = atoi(argv[++i]);
         log_msg("Param:port=%d", port);
      }
      if (strncasecmp("-stop-reconnect-on", argv[i], strlen(argv[i])) == 0) {
         stopReconnectOnError = atoi(argv[++i]);
         log_msg("Param:stopReconnectOnError=%d", stopReconnectOnError);
      }
   }
}
