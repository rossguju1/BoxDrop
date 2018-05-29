#ifndef PEER_H
#define PEER_H

typedef struct filetemp{		//PLACEHOLDER
	char data[50];
}file_t;



void peer_stop();

#define MAX_LEN 1024 /*Path length for a directory*/
#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 16 /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/

void *monitor(void *arg);
void add_watches(int fd, char *root);


#endif //PEER_H