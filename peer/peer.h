#ifndef PEER_H
#define PEER_H

#define MAX_LEN 1024 /*Path length for a directory*/
#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 16 /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/


typedef struct current_downloads{		//curent downloads
    char file_name[LEN_NAME];
    int name_length;
}file_t;

typedef struct file_name{
    char name[MAX_LEN];
    int name_length;

}file_detail;


void peer_stop();


void *monitor(void *arg);
void add_watches(int fd, char *root);
void *modify(void *arg);
void downloadFromPeer(struct in_addr peerIP, char *file_to_download, int filename_size);
#endif //PEER_H