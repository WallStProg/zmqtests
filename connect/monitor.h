int startMonitor(void* context);
int startMonitor(void* context, void* (*function)(void*));
int stopMonitor();
int monitorEvent(void* socket, const char* name);
uint64_t monitorEvent_v2(void* socket, const char* name);
void* monitorFunc(void* context);
