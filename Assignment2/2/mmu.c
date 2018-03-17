#include<stdio.h>
#include <sys/types.h>
#include  <sys/ipc.h>
#include <unistd.h>
#include<stdbool.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include<string.h>
#include "pagetable.h"

volatile sig_atomic_t usr_interrupt = 0;

void synch_signal (int sig)
{
	usr_interrupt = 1;
}

void prepareSignalParameters(sigset_t block_mask,sigset_t mask){
	struct sigaction usr_action;
	sigfillset (&block_mask);
	usr_action.sa_handler = synch_signal;
	usr_action.sa_mask = block_mask;
	usr_action.sa_flags = 0;
	sigaction (SIGCONT, &usr_action, NULL);
}

char* concat(const char *s1, const char *s2)
{
	char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the null-terminator
   result = strcpy(result, s1);
   result = strcat(result, s2);
   return result;
}

int power(int base,int exponent){
	int result= 1;
	for(int i=0;i<exponent;i++){
		result *= base;
	}
	return result;
}

void hexToBin(char *address,char *binAddress,int address_bits){
	int i, j;
   address += 2;

   for(j = i = 0; address[i]; ++i, j += 4){
      switch(address[i]){
        case '0': strcpy(binAddress + j, "0000"); break;
        case '1': strcpy(binAddress + j, "0001"); break;
        case '2': strcpy(binAddress + j, "0010"); break;
        case '3': strcpy(binAddress + j, "0011"); break;
        case '4': strcpy(binAddress + j, "0100"); break;
        case '5': strcpy(binAddress + j, "0101"); break;
        case '6': strcpy(binAddress + j, "0110"); break;
        case '7': strcpy(binAddress + j, "0111"); break;
        case '8': strcpy(binAddress + j, "1000"); break;
        case '9': strcpy(binAddress + j, "1001"); break;
        case 'a': strcpy(binAddress + j, "1010"); break;
        case 'b': strcpy(binAddress + j, "1011"); break;
        case 'c': strcpy(binAddress + j, "1100"); break;
        case 'd': strcpy(binAddress + j, "1101"); break;
        case 'e': strcpy(binAddress + j, "1110"); break;
        case 'f': strcpy(binAddress + j, "1111"); break;
        default:
            printf("invalid character %c\n", address[i]);
            strcpy(binAddress + j, "0000"); break;
      }
    }
  	int difference = address_bits - strlen(binAddress);//22 is no of bits in virtual address
   char *zeroes = malloc(difference + 1 + strlen(binAddress));

   while(difference-- > 0){
   	zeroes = strcat(zeroes,"0");
   }
   zeroes = concat(zeroes,binAddress);
   strcpy(binAddress,zeroes);
}

void getSharedPageTable(int page_bits,char pid){
	key_t key = ftok("shm.txt", pid);
	int page_count =  power(2,page_bits);
	//printf("Page count  %d\n", page_count);
	int id = shmget(key, page_count * sizeof(struct PageTable), 0666);
    	if (id < 0) {
    		printf("*** shmget error (mmu) ***\n");
    		exit(1);
    	}
    	pt = (struct PageTable *) shmat(id, NULL, 0);
    	printf("MMU is attached with the page table");
}

void handlePageAccess(int os_pid,char *address,char mode,int page_bits,int offset_bits,int *page_hit,int *page_fault){
	sigset_t block_mask,mask;
	int addr_len = (page_bits + offset_bits) + (page_bits + offset_bits) % 4 + 1;
	char binAddress[addr_len]; 
	hexToBin(address,binAddress,page_bits+offset_bits);
	//printf("Binary address %s \n",binAddress);
	char *pageno = malloc(page_bits+1);
	pageno = strncpy(pageno,binAddress,page_bits);
	pageno[page_bits]='\0';
	int page = (int) strtol(pageno, NULL, 2);
	printf("Request for page %d in mode %c\n",page, mode);

	if(pt[page].valid == 1){//requested page already in memory
		pt[page].requested = -1;//set to -1 to indicate to os that page is already in memory
		(*page_hit)++;
	}
	else{	//requested page not in memory
		printf("Not present in Page Table : Page Fault\n");
		(*page_fault)++;
		pt[page].requested = getpid();
		if(mode == 'W'){
			pt[page].dirty = 1;
		}
				
	}
	kill (os_pid, SIGUSR1);
	prepareSignalParameters(block_mask,mask);
	while (!usr_interrupt){
	}
	usr_interrupt = 0;
}

void printPageTable(int page_count){
	printf("Page No Valid Frame Dirty Requested\n");
	for(int i=0;i<page_count;i++){
		printf("%d %d %d %d %d\n", i,pt[i].valid,pt[i].frame,pt[i].dirty,pt[i].requested);
	}
}

int main(int argc, char *argv[]){
	system("pidof ./mmu > mmupid.txt");
	sleep(10);
 	int page_bits = (int)strtol(argv[2],NULL,10);
 	int offset_bits = (int)strtol(argv[3],NULL,10);

	int os_pid;
	FILE *fp;
	while(access("ospid.txt",F_OK) == -1){
	}
	fp = fopen("ospid.txt", "r");
	fscanf(fp, "%d", &os_pid);
	fclose(fp);

	char pid = argv[4][0];
	getSharedPageTable(page_bits,pid);

	int read=0,page_hit=0,page_fault=0;
	sigset_t block_mask,mask;
	int addr_len = (page_bits + offset_bits) + (page_bits + offset_bits) % 4 + 1;
	char address[addr_len];
	char mode;	
	fp = fopen(argv[1], "r");
	int page_count =  power(2,page_bits);
	while (1){
	   read = fscanf(fp, "%s %c\n", address, &mode);
	   if( read == -1 ){
	   break;
	   } 
	  	handlePageAccess(os_pid,address,mode,page_bits,offset_bits,&page_hit,&page_fault);  
	   printPageTable(page_count);
	}

	fclose(fp);
	kill (os_pid, SIGUSR1);
	printf("Page hit counter %d\n", page_hit);
	printf("Page fault counter %d\n", page_fault);
  	return 0;
}
