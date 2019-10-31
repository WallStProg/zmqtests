int startMonitor(void* context);
int startMonitor(void* context, void* (*function)(void*));
int stopMonitor();
int monitorEvent(void* socket, const char* name);
