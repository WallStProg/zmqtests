#include <string.h>

int seconds = 0;
int sleepAtExit = 0;
int port = 0;
int disableReconnect = 0;
int fix1256 = 0;
int sendAfterDisconnect = 0;
int linger = 0;
int immediateExit = 0;
int duplicateDisconnect = 0;
int pollAfterDisconnect = 0;
int longLived = 0;
int earlyLinger = 0;

void parseParams(int argc, char** argv)
{
   for (int i = 1; i < argc; i++) {
      if (strncasecmp("-seconds", argv[i], strlen(argv[i])) == 0) {
         seconds = atoi(argv[++i]);
         log_msg("Param:seconds=%d", seconds);
      }
      if (strncasecmp("-sleep-at-exit", argv[i], strlen(argv[i])) == 0) {
         sleepAtExit = atoi(argv[++i]);
         log_msg("Param:sleepAtExit=%d", sleepAtExit);
      }
      if (strncasecmp("-port", argv[i], strlen(argv[i])) == 0) {
         port = atoi(argv[++i]);
         log_msg("Param:port=%d", port);
      }
      if (strncasecmp("-disable-reconnect", argv[i], strlen(argv[i])) == 0) {
         disableReconnect = 1;
         log_msg("Param:disableReconnect=%d", disableReconnect);
      }
      if (strncasecmp("-fix1256", argv[i], strlen(argv[i])) == 0) {
         fix1256 = 1;
         log_msg("Param:fix1256=%d", fix1256);
      }
      if (strncasecmp("-send-after-disconnect", argv[i], strlen(argv[i])) == 0) {
         sendAfterDisconnect = 1;
         log_msg("Param:sendAfterDisconnect=%d", sendAfterDisconnect);
      }
      if (strncasecmp("-immediate-exit", argv[i], strlen(argv[i])) == 0) {
         immediateExit = 1;
         log_msg("Param:immediateExit=%d", immediateExit);
      }
      if (strncasecmp("-duplicate-disconnect", argv[i], strlen(argv[i])) == 0) {
         duplicateDisconnect = 1;
         log_msg("Param:duplicateDisconnect=%d", duplicateDisconnect);
      }
      if (strncasecmp("-linger", argv[i], strlen(argv[i])) == 0) {
         linger = atoi(argv[++i]);
         log_msg("Param:linger=%d", linger);
      }
      if (strncasecmp("-poll-after-disconnect", argv[i], strlen(argv[i])) == 0) {
         pollAfterDisconnect = 1;
         log_msg("Param:pollAfterDisconnect=%d", pollAfterDisconnect);
      }
      if (strncasecmp("-long-lived", argv[i], strlen(argv[i])) == 0) {
         longLived = 1;
         log_msg("Param:longLived=%d", longLived);
      }
      if (strncasecmp("-early-linger", argv[i], strlen(argv[i])) == 0) {
         earlyLinger = 1;
         log_msg("Param:earlyLinger=%d", earlyLinger);
      }
   }
}
