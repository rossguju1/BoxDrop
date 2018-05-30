#ifndef PEER_H
#define PEER_H


typedef struct current_downloads{		//curent downloads
    char files[MAX_CONCURRENT_DOWNLOADS * FILE_NAME_LEN];
    int num_downloads;
}file_t;

typedef struct file_name{
    char[MAX_LEN] name;
    int name_lenght;

};


void peer_stop();

#define MAX_LEN 1024 /*Path length for a directory*/
#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 16 /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/

void *monitor(void *arg);
void add_watches(int fd, char *root);
void *modify(void *arg);
#endif //PEER_H