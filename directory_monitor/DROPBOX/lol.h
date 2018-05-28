
#ifndef CS60_PROJ_MONITOR_H
#define CS60_PROJ_MONITOR_H



#define MAX_LEN 1024 /*Path length for a directory*/
#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 16 /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN  asdasdasdasas   ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/

void *monitor(void *arg);
void add_watches(int fd, char *root);
vghgjhbhj




#endif //CS60_PROJ_MONITOR_H
