#include <sys/types.h>
#include  <sys/ipc.h>
#include <unistd.h>
#include<stdio.h>
#include<stdbool.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include<string.h>
#include "pagetable.h"
#include "queue.h"
#include "fifo.h"
#include "map.h"
#include "lru.h"
#include "ll.h"


int createSharedPageTable(int page_count,char pid){
	key_t key = ftok("shm.txt", pid);;
	int id = shmget(key, page_count * sizeof(struct PageTable), IPC_CREAT | 0666);
	if (id < 0) {
          printf("*** shmget error (os) ***\n");
          exit(1);
     }
	pt = (struct PageTable *) shmat(id, NULL, 0);
	for(int i=0; i<page_count; i++){
		pt[i].valid = 0;
		pt[i].frame = -1;
	}
	return id;
}

/* When a SIGUSR1 signal arrives, set this variable. */
volatile sig_atomic_t usr_interrupt = 0;
void synch_signal(int page_count){
	usr_interrupt = 1;	
}


void prepareSignalParameters(sigset_t block_mask,sigset_t mask){
	struct sigaction usr_action;
	sigfillset (&block_mask);
	usr_action.sa_handler = synch_signal;
	usr_action.sa_mask = block_mask;
	usr_action.sa_flags = 0;
	sigaction (SIGUSR1, &usr_action, NULL);
}


void printPageTable(int page_count){
	for(int i=0;i<page_count;i++){
		printf("%d %d %d %d %d\n", i,pt[i].valid,pt[i].frame,pt[i].dirty,pt[i].requested);
	}
}


void replacePage(int i,int rep_page,int *disk_access_counter){
	pt[rep_page].valid = 0;
	if(pt[rep_page].dirty == 1){
		pt[rep_page].dirty = 0;
		sleep(1);
		*disk_access_counter += 1;
	}
	pt[i].frame = pt[rep_page].frame;
	printf("Allocate page %d to frame = %d by taking frame from page %d\n",i,pt[i].frame,rep_page);
}


void handlePageRequest(int i,char *replacement_algo,struct Queue *queue,struct unordered_map *unordered_map,int frame_count,
					   int *frame,int *disk_access_counter,int *all_zero){
	//free frame allocation
	int rep_page;
	if(strcmp(replacement_algo,"fifo") == 0){
		rep_page = fifo(queue,i,frame_count);
	}
	else{
		rep_page = lru(unordered_map,i,frame_count);
	}
	
	if(rep_page == -1){
		pt[i].frame = (*frame)++;
		printf("Allocate page %d to free frame = %d\n",i,pt[i].frame);	
	}
	else{
		replacePage(i,rep_page,disk_access_counter);
	}
	sleep(1);
	*disk_access_counter += 1;
	pt[i].requested = 0;
	//pt[i].dirty = 0;
	pt[i].valid = 1;
	*all_zero = 0;
}

void handlePageAlreadyInMemory(int i,char * replacement_algo,struct unordered_map *unordered_map,int frame_count,int *all_zero){
	pt[i].requested=0;
	if(strcmp(replacement_algo,"lru") == 0){
		lru(unordered_map,i,frame_count);
	}
	*all_zero = 0;
}

int handlePageFault(int mmu_pid,struct Queue *queue,struct unordered_map *unordered_map,char *replacement_algo,int page_count,int frame_count,
					  int *disk_access_counter,int *frame){
	sigset_t block_mask,mask;
	prepareSignalParameters(block_mask,mask);
  	//wait for signal from mmu 
	while (!usr_interrupt){
	}
	usr_interrupt = 0;
	int all_zero = 1;
	for(int i=0;i<page_count;i++){
		if(pt[i].requested == -1){
			handlePageAlreadyInMemory(i,replacement_algo,unordered_map,frame_count,&all_zero);
			break;
		}
		if(pt[i].requested != 0){
			printf("Process %d has requested for page %d\n", mmu_pid,i);
			handlePageRequest(i,replacement_algo,queue,unordered_map,frame_count,frame,disk_access_counter,&all_zero);
			break;
		}
	}
	fflush(stdout);
	kill(mmu_pid,SIGCONT);
	return all_zero;
}


int main(int argc,char *argv[]){
	int page_count = (int) strtol(argv[1],NULL,10);
	int frame_count = (int) strtol(argv[2],NULL,10);
	char *replacement_algo = malloc(strlen(argv[3])+1);
	char pid = argv[4][0];
	replacement_algo = strcpy(replacement_algo,argv[3]);
	system("pidof ./os > ospid.txt");
	int id = createSharedPageTable(page_count,pid);

	int mmu_pid;
	FILE *fp = fopen("mmupid.txt", "r");
	fscanf(fp, "%d", &mmu_pid);
    fclose(fp);

    int frame=0,disk_access_counter=0;
    struct Queue *queue = malloc(sizeof(struct Queue));
	struct unordered_map *unordered_map = malloc(sizeof(struct unordered_map) + frame_count * sizeof(struct Node));
    
  	while(true){
		int stop = 1;
		stop = handlePageFault(mmu_pid,queue,unordered_map,replacement_algo,page_count,frame_count,&disk_access_counter,&frame);
		if(stop){
			break;
		}
	}
   	shmdt(pt);
    shmctl(id, IPC_RMID, 0);

	printf("Disk access counter %d\n",disk_access_counter);
	return 0;
}
