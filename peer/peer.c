
/*

 CS60 - DropBox Project

 peer.c

*/

#include <errno.h>
#include <sys/inotify.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include "../common/constants.h"
#include "peer.h"
#include "../common/filetable.h"
#include "../common/file.h"
#include <fcntl.h>

/**************** local functions ****************/
void peer_stop(); 
void* keepAlive(void* arg);
void talkto_tracker();
void *uploadThread(void *sock_desc);
void* listen_to_peer();
FILE *fp_log;

/**************** global variables ****************/
int tracker_connection = -1;         //connection to the tracker
int interval = -1;
int piece_len = -1;
bool modifying_global = true;
bool file_added = true;
file_t * peer_downloads[MAX_CONCURRENT_DOWNLOADS];
struct file_name files[MAX_FILES];

pthread_mutex_t *sendtotracker_mutex; //routing_table mutex

void add_watches(int fd, char *root)
{
    int wd;
    char *abs_dir;
    struct dirent *entry;
    DIR *dp;

    dp = opendir(root);
    if (dp == NULL)
    {
        perror("Error opening the starting directory");
        exit(0);
    }

    /* add watch to starting directory */
    wd = inotify_add_watch(fd, root, IN_CREATE | IN_DELETE | IN_CLOSE_WRITE);//);
    /*wd = inotify_add_watch(fd, files[0], IN_CREATE | IN_DELETE | IN_CLOSE_WRITE);//);
    if (wd == -1)
    {
        printf("Couldn't add watch to %s\n",files[0]);
    }
    wd = inotify_add_watch(fd, files[1], IN_CREATE | IN_DELETE | IN_CLOSE_WRITE);//);

    if (wd == -1)
    {
        printf("Couldn't add watch to %s\n",files[1]);
    }*/
    if (wd == -1)
    {
        printf("Couldn't add watch to %s\n",root);
    }
    else
    {
        printf("Watching:: %s\n",root);
    }

    /* Add watches to the Level 1 sub-dirs*/
    /*abs_dir = (char *)malloc(MAX_LEN);
    while((entry = readdir(dp)))
    {
        // if its a directory, add a watch

        if (entry->d_type == DT_DIR)
        {
            strcpy(abs_dir,root);
            strcat(abs_dir,entry->d_name);

            wd = inotify_add_watch(fd, abs_dir, IN_CREATE | IN_DELETE | IN_CLOSE_WRITE);// | IN_ONESHOT);
            if (wd == -1)
                printf("Couldn't add watch to the directory %s\n",abs_dir);
            else
                printf("Watching:: %s\n",abs_dir);
        }
    }*/

    closedir(dp);
//    free(abs_dir);
}

int main(const int argc, char *argv[])
{
	int sock_fd;
	struct sockaddr_in address;

    signal(SIGINT, peer_stop);


    sendtotracker_mutex = malloc( sizeof(pthread_mutex_t) );
    pthread_mutex_init(sendtotracker_mutex,NULL);

    // // choose a node to connect
    char hostname[50];
    printf("Enter server name to connect:");
    scanf("%s", hostname);
    struct hostent* host;
    host = gethostbyname(hostname);     //get host structure from gethostbyname
    if (host== NULL){
        printf("%s\n","Invalid Server" );
        return -1;
    }
    char* ip = inet_ntoa(*((struct in_addr *) host->h_addr_list[0]));

    // choose an IP to connect
    // char hostname[50];
    // printf("Enter server IP to connect:");
    // scanf("%s", hostname);
    // // struct hostent* host;
    // char *ip;
    // inet_pton(AF_INET, hostname, ip);

    // Create socket
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("\n\nsocket failed\n\n");
        return -1;
    }
    printf("\n\n%s\n","Peer Socket created" );


    memset(&address, '0', sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(TRACKER_PORT);




   // Convert IPv4 addresses from IP to binary form and pack it
    if(inet_pton(AF_INET, ip, &address.sin_addr)<=0) 
    {
        printf("\nInvalid Tracker address to connect \n");
        return -1;
    }

    //Connect 
    if (connect(sock_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        printf("\nConnection to Tracker Failed \n");
        return -1;
    }
    tracker_connection = sock_fd;
    printf("%s\n","Peer connected to Tracker" );

    if (argc > 1){
        //downloadFromPeer();
    }

    pthread_t monitor_thread;
    // file dummies;
    /*files[0] = "DROPBOX/overlay.c";
    files[1] = "DROPBOX/1.odt";*/
    pthread_create(&monitor_thread, NULL, monitor, (void *) 0);
    pthread_t listentopeer_thread;
    pthread_create(&listentopeer_thread, NULL, listen_to_peer, (void *) 0);
    talkto_tracker();

    while(1){
        sleep(10);
    }

    printf("%s\n","Ending peer" );
    peer_stop();

	
}

bool inDirectory(char * file_name ,int name_size)
{
	struct dirent *DIRentry;

	DIR *DIRp = opendir(DIRECTORY_NAME);

	if (DIRp == NULL)
	{
		printf("Could not open dropbox root directory" );
		return NULL;
	}
//    fileTable_t *localFileTable = create_fileTable();
//    // printf("file number %d \n", get_number_of_files());


	while ((DIRentry = readdir(DIRp)) != NULL) {
		int size = get_file_size(DIRentry->d_name);

		if (strncmp(DIRentry->d_name, file_name, name_size) == 0) {
			return true;
		} else {
			continue;
		}
	}

	closedir(DIRp);

	return false;
}

void removeFromCurrentDownloads(char * file_name, char * file_name_size)
{
    for(int i = 0 ; i < MAX_CONCURRENT_DOWNLOADS; i++)
    {
        if(peer_downloads[i]!= NULL)
        {
            if(strncmp(file_name, peer_downloads[i]->file_name, file_name_size) == 0)
            {
                free(peer_downloads[i]->file_name);
                peer_downloads[i]=NULL;
            }
        }
    }
}

bool isInCurrentDownloads(char * file_name, char * file_name_size)
{
    for(int i = 0 ; i < MAX_CONCURRENT_DOWNLOADS; i++)
    {
        if(peer_downloads[i]!= NULL)
        {
            if(strncmp(file_name, peer_downloads[i]->file_name, file_name_size) == 0)
            {
                return true;
            }
        }
    }
    return false;

}


bool CheckInFileTable(char * current_file_name,struct fileTable file_table)
{
   char temp[FILE_NAME_LEN];

	int num;

	int i;

	num = file_table.numfiles;


	for (i = 0; i < num; i++) {

		memset(temp, 0, sizeof(file_table.nodes[i].filename));

		memcpy(temp, file_table.nodes[i].filename, sizeof(file_table.nodes[i].filename));

		if (strncmp(temp, current_file_name,file_table.nodes[i].file_name_size) == 0) {
			return true;
		} else {

			continue;
		}
	}
	return false;
}

int get_number_of_files_locally()
{
    struct dirent *DIRentry;

    DIR *DIRp = opendir(DIRECTORY_NAME);
    for (int i = 0 ; i < MAX_CONCURRENT_DOWNLOADS; i++)
    {
        free(peer_downloads[i]->file_name);
        peer_downloads[i]=NULL;
    }
    if (DIRp == NULL)
    {
        printf("Could not open dropbox root directory" );
        return NULL;
    }
    int count = 0;
    while ((DIRentry = readdir(DIRp)) != NULL) {
        //Ignore files that start witn .
        if (DIRentry->d_name[0] != '.'){
            count++;
        }
        
    }

    closedir(DIRp);
    return count;
}

void get_all_files_locally()
{
    struct dirent *DIRentry;
    DIR *DIRp = opendir(DIRECTORY_NAME);
    if (DIRp == NULL)
    {
        printf("Could not open dropbox root directory\n" );
        return NULL;
    }
    int count = 0;

    //Need to free current files list first


    while ((DIRentry = readdir(DIRp)) != NULL) {
        if (DIRentry->d_name[0] != '.'){
            file_detail * temp;
            temp = (file_detail *)malloc(sizeof(file_detail));
            temp->name_length = strlen(DIRentry->d_name);
            printf("Length is %d\n", temp->name_length);
            temp->name[temp->name_length] = '\0';
            memcpy(temp->name, DIRentry->d_name, strlen(DIRentry->d_name));
            printf("Found file %s\n", temp->name);

            memcpy(&(files[count].name), temp->name, temp->name_length + 1);
            files[count].name_length = temp->name_length;
            free(temp);
            //files[count] = temp;
            count++;
        }
    }
    closedir(DIRp);
}

void remove_locally(char * file_name, int file_size){
    char root[MAX_LEN];
    strcpy(root,DIRECTORY_NAME);
    if(root[strlen(root)-1]!='/')
        strcat(root,"/");
    strcat(root, file_name);
    int ret = remove(root);
    if(ret == 0) {
        printf("File %s deleted successfully", file_name);
    } else {
        printf("Error: unable to delete the file");
    }
}

void talkto_tracker(){


    //First register
    ptp_peer_t* segtosend = malloc(sizeof(ptp_peer_t) );
    for (int i = 0 ; i < MAX_CONCURRENT_DOWNLOADS; i++)
    {
        peer_downloads[i]=NULL;
    }
    segtosend->type = REGISTER;
    //printf("%s\n","SENDING REGISTER" );
    pthread_mutex_lock(sendtotracker_mutex); 
    send(tracker_connection , segtosend , sizeof(ptp_peer_t), 0 );
    pthread_mutex_unlock(sendtotracker_mutex);
    free(segtosend);

    //Create an heartbeat thread (keepAlive)
    pthread_t heartbeat_thread;
    pthread_create(&heartbeat_thread, NULL, keepAlive, (void *) &tracker_connection);
printf("Size of ptp peer is %ld\n", sizeof(ptp_peer_t));
    printf("Size is ptp tracker is %ld\n", sizeof(ptp_tracker_t));
    ptp_tracker_t* receivedseg = malloc(sizeof(ptp_tracker_t) );

    //Keep receiving data from tracker
    while( (read( tracker_connection , receivedseg, sizeof(ptp_tracker_t))) > 0 ){
        printf("%s\n","Message received from tracker" );
        printf("Received interval is %d\n",receivedseg->interval );
        interval = receivedseg->interval;
        piece_len = receivedseg->piece_len;

        printf("Received Filetable size is %d\n",receivedseg->file_table.numfiles );
        for (int i = 0 ; i < receivedseg->file_table.numfiles; i++)
        {
            node_t * currentfile = &(receivedseg->file_table.nodes[i]);
            printf("file name : %s\n", currentfile->filename);
            printf("Number of peers is %d\n" ,currentfile->num_peers);
            for (int j=0; j<currentfile->num_peers; j++ ){
            printf ("Ip of peer %d is \n",j);
            char* iip = inet_ntoa(currentfile->IP_Peers_with_latest_file[j]);
            printf("%s\n",iip );



                printf("\nAn ip should be printed by now\n");
            }
        }


        // 2 cases
        //case 1
        // global table has more files than peer
        for (int i = 0 ; i< receivedseg->file_table.numfiles; i++)
        {
            if (inDirectory(receivedseg->file_table.nodes[i].filename, receivedseg->file_table.nodes[i].file_name_size) == false)
            {
                if (!isInCurrentDownloads(receivedseg->file_table.nodes[i].filename, receivedseg->file_table.nodes[i].file_name_size))
                {
                    for (int j = 0 ; j < MAX_CONCURRENT_DOWNLOADS ; j++)
                    {
                        if(peer_downloads[i] != NULL)
                        {
                            file_t * temp;
                            temp = (file_t *)malloc(sizeof(file_t));
                            memcpy(temp->file_name, receivedseg->file_table.nodes[i].filename, receivedseg->file_table.nodes[i].file_name_size);
                            temp->name_length = receivedseg->file_table.nodes[i].file_name_size;
                            peer_downloads[i] = temp;
                            break;
                        }
                    }
                    //TODO: DOWNLOAD FILE NOW AND REMOVE FROM PEER DOWNLOADS AFTER DONE
                    downloadFromPeer(receivedseg->file_table.nodes[i].IP_Peers_with_latest_file[0], receivedseg->file_table.nodes[i].filename, receivedseg->file_table.nodes[i].file_name_size);
                }

            }
            else{
                if (receivedseg->file_table.nodes[i].status == DELETED)
                {
                    remove_locally(receivedseg->file_table.nodes[i].filename, receivedseg->file_table.nodes[i].file_name_size);
                }
            }
        }

        //case 2 = extra fies
        get_all_files_locally();
        int num_of_files = get_number_of_files_locally();
        for(int i = 0 ; i <num_of_files; i++)
        {


            printf("Filetable content name is: %s\n",  files[i].name);
            printf("Name length is %d\n",files[i].name_length );

            char current_file_name[FILE_NAME_LEN];
            memcpy(current_file_name, files[i].name, FILE_NAME_LEN);
            if (!CheckInFileTable(current_file_name, receivedseg->file_table))
            {
                // send file update to tracker
                ptp_peer_t* segtosend = malloc(sizeof(ptp_peer_t) );
                segtosend->type = FILE_UPDATE;
                memcpy(segtosend->file_information.filename, current_file_name, FILE_NAME_LEN);
                segtosend->file_information.file_name_size = files[i].name_length;
                segtosend->file_information.latest_timestamp = time(NULL);
                segtosend->file_information.status = ADDED;
                send(tracker_connection , segtosend , sizeof(ptp_peer_t), 0 );
                memset(&current_file_name, 0, sizeof(current_file_name));
                memset(&segtosend, 0, sizeof(segtosend));
                free(segtosend);
            }
        }
//        struct dirent *DIRentry;  // Pointer for directory entry
//// 	// chdir("/Users/rossguju/Desktop/anon/CS60SP18/CS60-TeamDDOS-DropBox/common");
//
//// 	DIR *DIRp = opendir(".");
//
//// 	if (DIRp == NULL)
//// 	{
//// 		printf("Could not open dropbox root directory" );
//// 		return NULL;
//// 	}
//// 	fileTable_t *localFileTable = create_fileTable();
//// 	// printf("file number %d \n", get_number_of_files());
//
//// 	while ((DIRentry = readdir(DIRp)) != NULL) {
//
//
//
//// 		int size = get_file_size(DIRentry->d_name);
//
//// 		if (node_insert(localFileTable, DIRentry->d_name, (size - 2), timestamp, ip_address)) {
//
//// 			continue;
//// 		}
//
//// 	}
//// 	closedir(DIRp);

    }

}

void* keepAlive(void* arg) {

    ptp_peer_t* segtosend = malloc(sizeof(ptp_peer_t) );
    //Until connected to tracker
    while (tracker_connection >=0 ){
        segtosend->type = KEEP_ALIVE;
        //segtosend.peer_ip = 
        if (interval > 0){
            //printf("%s\n","SENDING KEEPALIVE" );
            pthread_mutex_lock(sendtotracker_mutex);
            send(tracker_connection , segtosend , sizeof(ptp_peer_t), 0 );
            pthread_mutex_unlock(sendtotracker_mutex); 
            sleep(interval);
        } else {
            //If sleep interval is not defined, default is 5 seconds
            sleep(5);
        }

    }
    return NULL;
    }


void peer_stop(){
    if (tracker_connection >=0){
        close(tracker_connection);
        tracker_connection = -1;
    }
    pthread_mutex_destroy(sendtotracker_mutex);
    printf("Exiting Peer\n");
    exit(1);
}


void* listen_to_peer(){

    int SERV_PORT, list_sock, comm_sock;
    struct sockaddr_in listener_peer, connecting_peer ;  // server address

    socklen_t clilen;
    
    SERV_PORT = PEERTOPEER_PORT;

    // 1. Create socket on which to listen (file descriptor)
    list_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (list_sock < 0) {
        perror("error opening socket stream");
        exit(1);
    }


    // 2. Initiate the fields of the server address
    listener_peer.sin_family = AF_INET;
    listener_peer.sin_addr.s_addr = htonl(INADDR_ANY);
    listener_peer.sin_port = htons(SERV_PORT);

    // 3. bind the socket to the given server address
    if (bind(list_sock, (struct sockaddr *) &listener_peer, sizeof(listener_peer))) {
        perror("binding socket name");
        exit(2);
    }
    printf("Listening at port %d\n", ntohs(listener_peer.sin_port));
    // printf("To find your MacOS IP address, ifconfig | grep 'inet '\n");
  
    /* Listening to incoming connections */
    if ((listen(list_sock, MAX_PEER_SLOTS)) == -1)
    {
            fprintf(stderr, "Error on listen --> %s", strerror(errno));

            exit(EXIT_FAILURE);
    }


    clilen = sizeof(connecting_peer);
    pthread_t thread_id;
  
    char client_address[INET_ADDRSTRLEN];

    int peer_sock;
    while (true) {
        // accept connection and receive communication socket (file descriptor)
        peer_sock = accept(list_sock, (struct sockaddr *)&connecting_peer, &clilen);
        if (peer_sock == -1){
          perror("error accept");
        } else {

            if( NULL == inet_ntop( AF_INET, &(connecting_peer.sin_addr), client_address,sizeof(client_address) )
            ) {
                perror("erro inet_ntop");
                return EXIT_FAILURE;
            }

            printf("Client %s is connected \n", client_address);
            if( pthread_create( &thread_id , NULL ,  uploadThread , (void*) &peer_sock) < 0)
            {
                perror("could not create thread");
                return 1;
            }
            printf("Handling with thread %lu\n", thread_id);
             
          
            // pthread_detach(thread_id);

        }
    
      // pthread_join( thread_id , NULL);     
      // printf("Connection ended\n");
      /* Note that if the server doesn't explicitly exit or close the socket,
       * the socket will be closed when the program is killed or terminates. */    
    }
    close(list_sock);
}

void *uploadThread(void *sock_desc){
  
    int fd, peer_sock;
    FILE* fp;
    struct stat file_stat;
    // int offset;
    int remain_data;
    char file_size[256];

    //get peer socker descriptor
    peer_sock = *(int*)sock_desc;

    

    //read the file
    char filename[FILE_NAME_LEN];
    int get_file_name;

    if ((get_file_name= recv(peer_sock, filename, sizeof(filename), 0))< 0){
      printf("couldn't get filename from peer\n");
      return -1;
    }

    printf("filename is %s\n",filename );


    // fd = open(filename, O_RDONLY);
    // if (fd == -1)
    // {
    //         fprintf(stderr, "Error opening file --> %s", strerror(errno));

    //         exit(EXIT_FAILURE);
    // }

    fp = fopen(filename, "r");
    if (fp == NULL){
        printf("%s\n","Error opening file :(" );
    }

    fseek(fp,0,SEEK_END);
    int fileLen = ftell(fp);
    fseek(fp,0,SEEK_SET);
    // char *buffer = (char*)malloc(fileLen);
    // fread(buffer,fileLen,1,fp);
    


    fprintf(stdout, "File Size: \n%d bytes\n", fileLen);

    /* Sending file size */
    int len = send(peer_sock, &fileLen, sizeof(fileLen), 0);
    if (len < 0)
    {
          fprintf(stderr, "Error on sending greetings --> %s", strerror(errno));

          exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Server sent %d bytes for the size\n", fileLen);

    // offset = 0;
    remain_data = file_stat.st_size;
    /* Sending file data */
    int sent_bytes = 0;
    // sent_bytes = sendfile(peer_sock, fd, &offset, FILETRANSFER_BUFSIZE);
    // if (sent_bytes <= 0)
    // {
    //     // if (sent != 0)
    //         perror("sendfile");
    // }
    // printf("sent %d bytes\n", sent_bytes);
    // while (((sent_bytes = sendfile(peer_sock, fd, &offset, FILETRANSFER_BUFSIZE)) > 0) && (remain_data > 0))
    // {
    //         printf("in send while loop\n");
    //         fprintf(stdout, "1. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
    //         remain_data -= sent_bytes;
    //         fprintf(stdout, "2. Server sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
    // }
    // off_t offset = 0;
    // while (remain_data > 0 )
    // {
    //     ssize_t sent = sendfile(peer_sock, fd, &offset, remain_data);
    //     if (sent <= 0)
    //     {
    //         if (sent != 0)
    //             perror("sendfile");
    //         break;
    //     }
    //     printf("sent %d\n", sent );

    //     offset += sent;
    //     remain_data -= sent;
    // }

    //Sending of actual data, byte by byte
    for (int i = 0; i < fileLen; i++){
        char c = getc(fp);
        send(peer_sock, &c, 1,0);
        printf("Sent byte %d of %d, which is %c \n",i + 1, fileLen, c );
    }

    //send(peer_sock, fp, fileLen,0);
    printf("finished send while loop\n");

    fclose(fp);

    close(peer_sock);

    return 0;            

}

void downloadFromPeer(struct in_addr peerIP, char *file_to_download, int filename_size){

        char hostname[50];
        struct sockaddr_in address;
        char *buffer = malloc(FILETRANSFER_BUFSIZE);
        int sock_fd;
        FILE *received_file;
        int remain_data = 0;

        printf("Enter peer name to connect:");
        scanf("%s", hostname);
        struct hostent* host;
        host = gethostbyname(hostname);     //get host structure from gethostbyname
        if (host== NULL){
            printf("%s\n","Invalid Server" );
            return -1;
        }
        char* ip = inet_ntoa(peerIP);

        // Create socket
        if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("\n\nsocket failed\n\n");
            return -1;
        }
        printf("\n\n%s\n","Peer to peer Socket created" );

        
        memset(&address, '0', sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(PEERTOPEER_PORT);




       // Convert IPv4 addresses from IP to binary form and pack it
        if(inet_pton(AF_INET, ip, &address.sin_addr)<=0) 
        {
            printf("\nInvalid peer address to connect \n");
            return -1;
        }

        //Connect 
        if (connect(sock_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            printf("\nConnection to peer Failed \n");
            return -1;
        }
        printf("%s\n","Peer connected to another peer" );


        /*send file name as request */
        int file_name_request;
        char filename[FILE_NAME_LEN];
        memcpy(filename, file_to_download, filename_size);
        if ((file_name_request = send(sock_fd, filename, sizeof(filename), 0)) < 0){
          printf("couldn't send file name\n");
          return -1;
        }


        /* Receiving file size */
        int file_size_received;
        if (recv(sock_fd, &file_size_received, sizeof(file_size_received), 0) < 0){
          printf("couldn't receive file size\n");
          return -1;

        }
        fprintf(stdout, "\nFile size : %d\n", file_size_received);

        char newfilename[FILE_NAME_LEN + 3];
        newfilename[0] = 'n';
        newfilename[1] = 'e';
        newfilename[2] = 'w';
        strcat(newfilename, filename );

        received_file = fopen(newfilename, "w");
        if (received_file == NULL)
        {
                fprintf(stderr, "Failed to open file foo --> %s\n", strerror(errno));

                exit(EXIT_FAILURE);
        }
        printf("received file pointer valid\n");

        remain_data = file_size_received;
        int len;
        int bytes_received = 0;
        int bytes_to_receive = file_size_received;
        // while((bytes_received = recv(sock_fd, buffer, bytes_to_receive, 0)) > 0 ) {
        //    printf("in receive while loop\n");
        //    fwrite(buffer, sizeof(char), bytes_received, received_file);
        //    bytes_to_receive -= bytes_received;
        //    fprintf(stdout, "Receive %d bytes and we hope :- %d bytes , remain data %d\n", bytes_received, bytes_to_receive, remain_data);

        // }
        for (int i = 0; i < bytes_to_receive; i++){
            char c;
            recv(sock_fd, &c, 1,0 );
            fputc(c, received_file);
            printf("Received byte %d of %d \n",i + 1, bytes_to_receive );
        }
        printf("%s\n","Exiting download" );
        fclose(received_file);

        close(sock_fd);
        removeFromCurrentDownloads(file_to_download, filename_size);
}


void file_modified( char * file_name)
{
    /*
     * this is triggered locally when a file is modified by user him/herself
     *
     *
     * */
    char current_file_name[FILE_NAME_LEN];
    memcpy(current_file_name, file_name, FILE_NAME_LEN  );
    ptp_peer_t* segtosend = malloc(sizeof(ptp_peer_t) );
    segtosend->type = FILE_UPDATE;
    memcpy(segtosend->file_information.filename, current_file_name, FILE_NAME_LEN);
    segtosend->file_information.status = MODIFIED;
    segtosend->file_information.file_name_size = strlen(file_name);
    send(tracker_connection , segtosend , sizeof(ptp_peer_t), 0 );
    free(segtosend);
}

void file_created ( char * file_name)
{
    /*
     * this is triggered locally when a file is created by user him/herself
     *
     *
     * */
    char current_file_name[FILE_NAME_LEN];
    memcpy(current_file_name, file_name, sizeof(current_file_name)  );
    printf("Comparing current filename %s", current_file_name);
    ptp_peer_t* segtosend = malloc(sizeof(ptp_peer_t) );
    segtosend->type = FILE_UPDATE;
    memcpy(segtosend->file_information.filename, current_file_name, sizeof(current_file_name));
    segtosend->file_information.latest_timestamp = time(NULL);
    segtosend->file_information.status = ADDED;
    segtosend->file_information.file_name_size = strlen(file_name);
    send(tracker_connection , segtosend , sizeof(ptp_peer_t), 0 );
    free(segtosend);
}

void file_deleted (char *file_name)
{
    /*
     * this is triggered locally when a file is deleted by user him/herself
     *
     *
     * */

    char current_file_name[FILE_NAME_LEN];
    memcpy(current_file_name, file_name, sizeof(current_file_name)  );
    ptp_peer_t* segtosend = malloc(sizeof(ptp_peer_t) );
    segtosend->type = FILE_UPDATE;
    memcpy(segtosend->file_information.filename, current_file_name, sizeof(current_file_name));
    segtosend->file_information.status = DELETED;
    segtosend->file_information.file_name_size = strlen(file_name);
    send(tracker_connection , segtosend , sizeof(ptp_peer_t), 0 );
    free(segtosend);
}

void *monitor(void *arg) {
    // see if directory exists
    DIR* dir = opendir(DIRECTORY_NAME);

    if (dir)
    {
        /* Directory exists. */
        // update drop box directory
        //update_drop_box_directory();
        const int event_size = sizeof(struct inotify_event);
        const int buf_len = 1024 * (event_size + FILENAME_MAX);
        int length, i = 0;
        int fd;
        char buffer[BUF_LEN], root[MAX_LEN];


        strcpy(root,DIRECTORY_NAME);
        if(root[strlen(root)-1]!='/')
            strcat(root,"/");
        puts(root);

        /* Set up logger*/
        fp_log = fopen("inotify_logger.log","a");
        if (fp_log == NULL)
        {
            printf("Error opening logger. All output will be redirected to the stdout\n");
            fp_log = stdout;
        }

        fd = inotify_init();
        if ( fd < 0 ) {
            perror( "Couldn't initialize inotify");
        }

        /* Read the sub-directories at one level under argv[1]
         * and monitor them for access */
        add_watches(fd,root);

        /* do it forever*/
        while(1)
        {

            /*char buff[buf_len];
            int no_of_events, count = 0;

            no_of_events = read (fd, buff, buf_len);

            while (count < no_of_events) {
                struct inotify_event *event = (struct inotify_event *)&buff[count];


                if (event->len){
                    if (event->mask & IN_CLOSE_WRITE)
                        if(!(event->mask & IN_ISDIR)){
                            printf( "The file %s was modified done.\n", event->name );
                            fflush(stdout);
                        }
                    if ( event->mask & IN_CREATE) {
                        if (event->mask & IN_ISDIR)
                            printf("DIR::%s CREATED\n", event->name );
                        else
                            printf("FILE::%s CREATED\n", event->name);
                    }

                    if ( event->mask & IN_MODIFY) {
                        if (event->mask & IN_ISDIR)
                            printf("DIR::%s MODIFIED\n",event->name );


                    }

                    if ( event->mask & IN_DELETE) {
                        if (event->mask & IN_ISDIR)
                            printf(" DIR::%s DELETED\n",event->name );
                        else
                            printf("FILE::%s DELETED\n", event->name );
                    }
                }
                count += event_size + event->len;
            }
*/

            i = 0;
            length = read( fd, buffer, BUF_LEN );

            if ( length < 0 ) {
                perror( "read" );
            }
            printf("got event: %d \n", length);
            // Read the events



            while ( i < length ) {
                struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
                if ( event->len ) {
                    if ( event->mask & IN_CREATE) {
                            if (event->name[0] != ':' && event->name[0] != '.')
                            {
                                if (event->mask & IN_ISDIR) {
                                    printf("DIR::%s CREATED\n", event->name);
                                }
                                else
                                {
                                    printf("FILE::%s CREATED\n", event->name);
                                    file_added = true;
                                    file_created(event->name);
                                }

                            }
                    }

                    if ( event->mask & IN_MODIFY) {
                        if (event->mask & IN_ISDIR)
                        {

                        }
                            printf("DIR::%s MODIFIED\n",event->name );


                    }
                    if ( event->mask & IN_CLOSE_WRITE) {
                        if (!file_added)
                        {
                            if(modifying_global){
                                if (event->name[0] != ':' && event->name[0] != '.')
                                {
                                    if (event->mask & IN_ISDIR) {
                                        pthread_t modifying_thread;
                                        pthread_create(&modifying_thread, NULL, modify, (void *) (char*)event->name);
                                    }
                                    else
                                    {
                                        pthread_t modifying_thread;
                                        pthread_create(&modifying_thread, NULL, modify, (void *) (char*)event->name);
                                    }

                                }

                            }

                        }
                        else
                        {
                            file_added = false;
                        }


                    }

                    if ( event->mask & IN_DELETE) {
                        if (event->name[0] != ':' && event->name[0] != '.')
                        {
                            if (event->mask & IN_ISDIR) {
                                printf(" DIR::%s DELETED\n",event->name );
                            }
                            else
                            {
                                printf("FILE::%s DELETED\n", event->name );
                                file_deleted(event->name);
                            }

                        }
                    }

                    i += EVENT_SIZE + event->len;
                }
            }
        }
        /* Clean up*/
        closedir(dir);
        ( void ) close( fd );
        return 0;
    }
    else if (ENOENT == errno)
    {
        //printf("here1 \n");
        /* Directory does not exist. */
        create_drop_box_directory();
        //get_all_files();
    }
    else
    {
        printf("some other error");
        pthread_exit(NULL);
    }

    return NULL;
}



void *modify(void *arg) {
    modifying_global = false;
    sleep(1);
    char * file_name = ((char *) arg);
   // char * file_name = *(char*)arg;
    file_modified(file_name);
    modifying_global = true;
    pthread_exit(NULL);
}

