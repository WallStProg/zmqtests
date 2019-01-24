#include <stdlib.h>
#include <string.h>

int pingPort = 0;
int pongPort = 0;
int numRequests = 100000;
bool pollFlag = false;

void parseParams(int argc, char** argv)
{
   for (int i = 1; i < argc; i++) {
      if (strncasecmp("-ping", argv[i], strlen(argv[i])) == 0) {
         pingPort = atoi(argv[++i]);
         log_msg("Param:ping=%d", pingPort);
      }
      if (strncasecmp("-pong", argv[i], strlen(argv[i])) == 0) {
         pongPort = atoi(argv[++i]);
         log_msg("Param:pong=%d", pongPort);
      }
      if (strncasecmp("-n", argv[i], strlen(argv[i])) == 0) {
         numRequests = atoi(argv[++i]);
         log_msg("Param:n=%d", numRequests);
      }
      if (strncasecmp("-poll", argv[i], strlen(argv[i])) == 0) {
         pollFlag = true;
         log_msg("Param:poll=%d", pollFlag);
      }
   }
}
