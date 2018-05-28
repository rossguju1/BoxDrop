


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <limits.h>
#include <pthread.h>

#include "monitor.h"
#include "../common/constants.h"
#include "../common/file.h"

/**************** global variables ****************/
//char *files[MAX_FILES];

/* Log file*/
FILE *fp_log;



void get_all_files()
{
    // get all files from server
    return;
}

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
    wd = inotify_add_watch(fd, dp, IN_CREATE | IN_DELETE | IN_CLOSE_WRITE);//);
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
    abs_dir = (char *)malloc(MAX_LEN);
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
    }

    closedir(dp);
//    free(abs_dir);
}

/* Main routine*/
int main( int argc, char **argv )
{
    pthread_t monitor_thread;
    // file dummies;
    /*files[0] = "DROPBOX/overlay.c";
    files[1] = "DROPBOX/1.odt";*/
    pthread_create(&monitor_thread, NULL, monitor, (void *) 0);
    pthread_join(monitor_thread, NULL);
    return 0;

}

void *monitor(void *arg) {
    // see if directory exists
    DIR* dir = opendir(DIRECTORY_NAME);

    if (dir)
    {
        /* Directory exists. */
        // update drop box directory
        update_drop_box_directory();
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
                        if (event->mask & IN_ISDIR)
                            printf("DIR::%s CREATED\n", event->name );
                        else
                            printf("FILE::%s CREATED\n", event->name);
                    }

                    if ( event->mask & IN_MODIFY) {
                        if (event->mask & IN_ISDIR)
                            printf("DIR::%s MODIFIED\n",event->name );


                    }
                    if ( event->mask & IN_CLOSE_WRITE) {
                        printf( "The file %s was modified done.\n", event->name );


                    }

                    if ( event->mask & IN_DELETE) {
                        if (event->mask & IN_ISDIR)
                            printf(" DIR::%s DELETED\n",event->name );
                        else
                            printf("FILE::%s DELETED\n", event->name );
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
        printf("here1 \n");
        /* Directory does not exist. */
        create_drop_box_directory();
        get_all_files();
    }
    else
    {
        printf("some other error");
        pthread_exit(NULL);
    }

}
