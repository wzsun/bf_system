// Basic File System
// Wesley Sun

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#define FILEPATH "/home/wzsun/Desktop/proj4/mmapped.bin"
#define SIZE 1000
#define FILESIZE (SIZE * sizeof(char))
#define BYTES 1024

/*
to do	to do	to do to do to do to do to do to do to do to do to do to do
--------------------------------------------------------------------
		MVDIR DOES NOT WORK (test it you'll see)
		MKFIL seems to have issues when size > BYTES..
		SZFIL DOES NOT WORK
		RMDIR WORKS BUT NEEDS RECURSIVE DELETING
 --------------------------------------------------------------------
 */

/*--------------------------------------------------------------------------------*/

int debug = 1;  // extra output; 1 = on, 0 = off

/*--------------------------------------------------------------------------------*/


// using linked allocation
struct file{
	char* name;

	struct file* start; // files
	struct file* nextfileblock; // files
	struct file* end; // files

	int flag; // 0 if directory, 1 if file, 2 if freemem
	struct file *next; // freemem linked list, and for directories to point to next file

	struct file *place; // stores the link for CWD
	char block[BYTES]; // each block size
};

int chdir(char* folder, struct file* disk){
	//disk[0].place is linked to Current Working Directory
	if(strcmp(folder,"..") == 0){
		if(debug == 1) { printf("chdir to ..\n"); }
		disk[0].place = disk[0].place -> place;
	}else{
		// head to CWD , search through that linked list for file name
		//printf("Got In to else statement\n");
		struct file* temp = disk[0].place;
		//printf("%p: %s, %p: %s \n", temp, temp -> name, disk[0].place, disk[0].place -> name);
		//temp = temp -> next;
		//printf("%p: %s \n", temp -> next, temp -> name);
		while(temp -> next != NULL){
			temp = temp -> next;
			//printf("%p: %s \n",temp->next, temp -> name);
			if(strcmp(temp ->name,folder) == 0 && temp -> flag == 0){
				if(debug == 1) { printf("chdir to [%s]\n", temp->name); }
				disk[0].place = temp;
				return 0;
			}
		}
	}
	return 1;
}

void mkdir(char* name, struct file* disk){
	// Go into free mem linked list to find next availible free block
	// This is disk[1];
	//printf("Got in \n");
	struct file* temp = disk[1].next;
	//printf("%p, %p, %p \n", temp, disk[1].next, &disk[3]);
	temp -> name = name; // Changes the name
	temp -> flag = 0; // Sets flag to directory
	temp -> place = disk[0].place; // Links the place to the CWD

	disk[1].next = temp->next; // Changes the free mem linked list so spot is gone
	temp -> next = NULL; // Sets the next slot to 0

	// Links the new directory to the end of the Linklist tied to the directory above it.
	struct file* temp2 = disk[0].place;
	printf("%p: %s \n", temp2 -> next, temp2 -> name);
	if(temp2 -> next == NULL){
		temp2 -> next = temp;
		printf("%p: %s \n", temp2 -> next, temp2 -> next ->name);
	} else {
		if(debug == 1) { printf("mkdir linking new dir to linkedlist\n"); }
		while(temp2 -> next != NULL){

			printf("%p: %s \n", temp2, temp2->name);
			printf("%p: %s \n", temp2 -> next, temp2 -> next -> name);
			temp2 = temp2 -> next;
			if( temp2 -> next == NULL ){
				printf("switched \n");
				temp2 -> next = temp;
				return;
			}
		}
	}
}

void mkfil(char* name, int size, struct file* disk){
	if(debug == 1) { printf("mkfil beginning \n"); }
	struct file* temp = disk[1].next;
	if(size > BYTES) {
		int amountOfBlocks = size/BYTES;
		if(size % BYTES != 0){
			amountOfBlocks++;
		}

		int i;

		struct file* startblock = temp;

		for( i=0; i<amountOfBlocks; i++){
			printf("%p \n", temp);
			temp = temp -> next;
		}

		struct file* endblock = temp;

		// re set free mem linked list
		disk[1].next = endblock -> next;

		if(debug == 1) { printf("mkfil (size > BYTES) \n"); }

		temp = startblock;
		//printf("%p \n", temp);
		for( i=0; i<amountOfBlocks; i++){
			//disk[1] for free mem
			printf("%p \n", temp);
			temp -> name = name;
			temp -> place = disk[0].place; // Links the place to the CWD
			temp -> flag = 1;
			temp -> start = startblock;
			temp -> end = endblock;
			temp -> nextfileblock = temp -> next;
			//temp -> next = temp -> next -> next;
			temp = temp -> next;
			//temp -> next = temp;
		}
		//temp -> next = NULL;
		startblock -> next = NULL;
		endblock -> next = NULL;
		endblock -> nextfileblock = NULL;

		if(debug == 1) { printf("after mkfil (size > BYTES) \n"); }
		// link it to CWD
		// Links the new file to the end of the Linklist tied to the directory above it.
		struct file* temp2 = disk[0].place;
		printf("mkfil: %p: %s \n", temp2 -> next, temp2 -> name);
		if(temp2 -> next == NULL){
			temp2 -> next = startblock;
		}else{
			while(temp2 -> next != NULL){
				temp2 = temp2 -> next;
				if( temp2 -> next == NULL ){
					if(debug == 1) { printf("(mkfil- size > BYTES) switched \n"); }
					temp2 -> next = startblock;
					return;
				}
			}
		}

	}else{
		if(debug == 1) {
			printf("mkfil: size < BYTES. (beep boop) \n");
			printf("pre-variable fields filled for new file: %p: %s \n", temp, temp -> name);

		}
		temp -> name = name;
		temp -> place = disk[0].place; // Links the place to the CWD
		temp -> flag = 1;
		temp -> start = temp;
		temp -> end = temp;
		temp -> nextfileblock = NULL;
		disk[1].next = temp -> next; // updates free mem linked list
		temp -> next = NULL;
		printf("post-variable fields filled: %p: %s \n", temp, temp -> name);

		// links to CWD
		struct file* temp2 = disk[0].place;
		printf("%p: %s \n", temp2 -> next, temp2 -> name);
		if(temp2 -> next == NULL){
			temp2 -> next = temp;
		}else{
			while(temp2 -> next != NULL){
				printf("%p: %s \n", temp2, temp2->name);
				temp2 = temp2 -> next;
				if( temp2 -> next == NULL ){
					//printf("switched /n");
					temp2 -> next = temp;
					return;
				}
			}
		}

	}
}

void mvdir(char* oldname, char* newname, struct file* disk){
	// looks at CWD, search through linked list, find the file name
	struct file* temp = disk[0].place;
	while( temp -> next != NULL){
		temp = temp -> next;
		if((strcmp(temp -> name, oldname) == 0)){
			temp -> name = newname;
		}
	}
}

void mvfil(char* oldname, char* newname, struct file* disk){
	struct file* temp = disk[0].place;
	while ( temp -> next != NULL ){
		temp = temp -> next;
		if((strcmp(temp -> name, oldname) == 0)){
			temp -> name = newname;
		}
	}
}

void rmfil(char* name, struct file* disk){
	struct file* temp = disk[0].place;
	struct file* predecessor;
	printf("%p: %s \n", temp -> next -> next, temp -> next -> name);
	while( temp -> next != NULL){
		predecessor = temp;
		temp = temp -> next;

		if(strcmp(temp -> name, name) == 0 && temp -> flag == 1){
			// reset fields
			while( temp -> nextfileblock != NULL){
				temp -> name = "";
				temp -> flag = 2;
				temp -> start = NULL;
				temp -> end = NULL;
				temp -> place = NULL;
				temp = temp -> nextfileblock;
			}

			// connect predecessor
			predecessor -> next = temp -> next;
			temp -> next = NULL;

			// add to free mem linked list
			predecessor = disk[1].next;
			while( predecessor -> next != NULL){
				predecessor = predecessor -> next;
			}
			predecessor -> next = temp;

		}
	}
}

void rmdir(char* name, struct file* disk){
	struct file* temp = disk[0].place;
	struct file* predecessor;
	printf("%p: %s \n", temp -> next -> next, temp -> next -> name);
	while( temp -> next != NULL){
		predecessor = temp;
		temp = temp -> next;

		// need to recursively delete files if its non-empty (still in progress)
		if(strcmp(temp -> name, name) == 0 && temp -> flag == 0){
			// reset fields
			while( temp -> next != NULL ){
				temp -> name = "";
				temp -> flag = 2;
				temp -> start = NULL;
				temp -> nextfileblock = NULL;
				temp -> end = NULL;
				temp -> place = NULL;
				temp = temp -> next;
			}

			// connect predecessor
			predecessor -> next = temp -> next;
			temp -> next = NULL;

			// add to free mem linked list
			predecessor = disk[1].next;
			while( predecessor -> next != NULL){
				predecessor = predecessor -> next;
			}
			predecessor -> next = temp;

		}
	}
}

void szfil(char* name, int size, struct file* disk){
	// 1. find amount of blocks in file using while loop and counter on nextfileblock

	// if larger
	// go to -> end and add more blocks from free mem linked list

	// if smaller go to start and count until amount of blocks, then wipe and remove the end by place it in free mem linked list.
}

int main(int argc, char *argv[])
{
	int i;
	//-------------------------------------------------------------

	// Sets the disk to all link to each other, this sets up the free memmory map starting at disk[0]
	// all

	// current working directory
	struct file* disk = malloc(SIZE * sizeof(struct file));
	disk[0].name = "Current Working Directory";
	disk[0].place = &disk[2];

	// Sets free mem linked list at disk[1]
	for(i = 1; i<SIZE-1; i++){
		disk[i].flag = 2;
		disk[i].name = "";
		disk[i].next = &disk[i+1];
	}

	// Sets the last disk spot to be null
	disk[SIZE].flag = 2;
	disk[SIZE].name = "";
	disk[SIZE].next = NULL;

	//disk[3].name = "test";

	// disk[2] will be our root directory
	disk[2].flag = 0;
	disk[2].name = "root";
	disk[2].next = NULL;
	disk[0].place = &disk[2];
	disk[1].next = &disk[3];

	mkdir("SUPERMAN", disk);
	chdir("SUPERMAN", disk);
	printf("CHDIR:%p: %s type:%d \n", disk[0].place, disk[0].place -> name, disk[0].flag);
	mkdir("BATMAN", disk);
	//mkdir("WONDERWOMAN", disk);
	//mkdir("MFDoom", disk);
	mvdir("BATMAN","FLASH",disk);
	chdir("FLASH",disk);
	printf("CHDIR:%p: %s type:%d \n", disk[0].place, disk[0].place -> name, disk[0].flag);
	//rmdir("FLASH", disk);
	mkfil("ROBIN", 2000, disk);
	rmfil("ROBIN", disk);
	mkfil("Neymar", 1000, disk);
	

	//chdir("..",disk);


	// This only prints the CWD kind of
	printf("start printing our list of dirs/files out\n");
	struct file* temp = disk[0].place;
	while(temp -> next != NULL){
		printf("%p: %s type:%d \n", temp, temp->name, temp->flag);
		temp = temp -> next;

	}
	//temp = temp -> next;
	printf("%p: %s type:%d \n", temp, temp->name, temp->flag);

	//printf("%d \n",chdir("FLASH", disk));

	//for(i=0; i<SIZE;i++){
	//printf("%s \n", disk[0].place -> name);
	//printf("%s \n", disk[1].place -> name);
	//printf("%s \n", disk[2].place -> name);






	//--------------------------------------------------------------

 	//printf("%d\n",*(root -> arr[0]));
	return 0;
}
