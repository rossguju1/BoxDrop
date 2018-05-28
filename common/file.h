
#ifndef FILE_H
#define FILE_H



typedef struct file_info {

    int wd;
	char file_path; // Path of the file

	unsigned long size; // Size of the file

	unsigned long lastModifyTime; // time stamp of last modification

} FileInfo_t;


void update_drop_box_directory();
void create_drop_box_directory();


char *get_file_path(char *filename);


int get_file_size(char *filename);





#endif







