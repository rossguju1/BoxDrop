
#ifndef CS60_PROJ_MONITOR_H
#define CS60_PROJ_MONITOR_H



#define MAX_LEN 1024 /*Path length for a directory*/
#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 50 /*Assuming that the length of the filename won't exceed 50 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/

void *monitor(void *arg);
void add_watches(int fd, char *root);





#endif //CS60_PROJ_MONITOR_H
