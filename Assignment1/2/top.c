#include<stdio.h>
#include<stdlib.h>
#include <dirent.h>
#include<string.h>
#include<ctype.h>
#include <unistd.h>
#include <sys/sysinfo.h>


int contains(char *args[],char *arg){
    for(int i=1;args[i]!=NULL;i++){
        if(strcmp(args[i],arg) == 0){
            return i;
        }
    }
    return 0;
}

long get_uptime()
{
    struct sysinfo s_info;
    int error = sysinfo(&s_info);
    if(error != 0)
    {
        printf("code error = %d\n", error);
    }
    return s_info.uptime;
}


char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

/*Reads a stat/statm file word by word and stores the value into the passed buffer*/
void getStat(long pid, char *buffer[],char path[]){
//get virt, res, shr values from statm
    char buf[20];
    int i=0;
    //snprintf(path, 40, "/proc/%ld/statm", pid);
    FILE *fp = fopen(path, "r");
    while( fscanf(fp, "%s", buf) != EOF ){
        buffer[i] = malloc(strlen(buf)+1);
        strcpy(buffer[i],buf);
        i++;
    }
    fclose(fp);
}

/* calculates and returns CPU usage % given process parameters like time spent in user mode and
uptime of the system*/
float calCPU(char *buffer[]){
    char *ptr;
    long utime = (long)strtol(buffer[13], &ptr, 10);//time spent executing process in user mode
    long stime = (long)strtol(buffer[14], &ptr, 10);//time spent executing process in system mode
    //long cutime = (long)strtol(buffer[15], &ptr, 10);
    //long cstime = (long)strtol(buffer[16], &ptr, 10);
    long starttime = (long)strtol(buffer[21], &ptr, 10);//start time of the process
    long totaltime = utime + stime;
    //totaltime = totaltime + cutime + cstime;
    long hertz = sysconf(_SC_CLK_TCK);//no of clock cycles per secondof the CPU
    long uptime = get_uptime();//the time for which CPU has been on

    /*starttime and totaltime are stored in terms of no of clock ticks so need to divide by clock cycles per second*/
    long seconds = uptime - (starttime / hertz);
    float cpuusage = 100 * ((totaltime / hertz) / seconds);
    return cpuusage;
}

void print_status(FILE *ofp,long pid) {
    //get virt, res, shr values from /proc/[pid]/statm
     char * bufferM[4];
     char pathM[40];
     snprintf(pathM, 40, "/proc/%ld/statm", pid);
     getStat(pid,bufferM,pathM);

     //get remaining values form /proc/[pid]/stat
     char *buffer[53];
     char path[40];
     snprintf(path, 40, "/proc/%ld/stat", pid);
     getStat(pid,buffer,path);
    
    char *ptr;
    /*Memory parameters*/
    long pagesize = (long)(getpagesize()/1024);
    long virt = (long)strtol(bufferM[0], &ptr, 10) * pagesize;
    long res = (long)(strtol(bufferM[1], &ptr, 10)) * pagesize;
    long shr = (long)(strtol(bufferM[2], &ptr, 10)) * pagesize;
    long phypages = sysconf(_SC_PHYS_PAGES);
    long totalmem = phypages * pagesize;
    float mem = res/totalmem;
    /*long virt = (long)strtol(buffer[22], &ptr, 10);
    long res = (long)(strtol(buffer[23], &ptr, 10));
    long shr = 0;
    printf("Total mem, phy pages, Page size, RES: %ld %ld %ld %ld\n",totalmem, phypages, pagesize, res);*/
    
    float cpuusage = calCPU(buffer);
    fprintf(ofp,"%s %s %s %ld %ld %ld %s %f %f\n",buffer[0],buffer[17],buffer[18],virt,res, shr, buffer[1],cpuusage,mem);
}


int main(int argc, char *argv[]){
	
    char *outFile;
    long n = -1;
    int position = contains(argv,"-n");
    if(position != 0){
        char *ptr;
        n = strtol(argv[position+1],&ptr,10);
        if(strlen(ptr) != 0){//ex. if commands like top -n 33def or top -n abc are entered by user
            printf("Invalid argument passed for -n %s\n", argv[position+1]);
            return 1;
        }
    }

    FILE *ofp;
    char *mode = "n";
    int positionWrite = contains(argv,">");
    int positionAppend = contains(argv,">>");

    if(positionWrite != 0){//if user enterd >
        if(strlen(argv[positionWrite+1]) != 0){
            outFile = argv[positionWrite+1];
            mode = "w";
        }
    }
    else if(positionAppend != 0){//if user enterd >>
        if(strlen(argv[positionAppend+1]) != 0){
            outFile = argv[positionAppend+1];
            mode = "a";
        }
    }
    else{
        ofp = fdopen(1,"w");//opening std out descriptor
    }
   
    if(strcmp(mode,"n") != 0){
        ofp = fopen(outFile,mode);
        //dup2(ofd,1);
    }

     
    //printf("Output file %s Mode %s n value %ld",outFile, mode, n); 
	while(n != 0){//if -n parameter is not passed to top, n will remain -1 and so this will be an infinite loop
	    if(n > 0){ //if -n is passed to top (n>0) then we will keep decrementing n till it becomes 0
            n--;
            //printf("Value of n %ld",n);
        }
        DIR *proc = opendir("/proc");
        if(proc == NULL) {
                perror("opendir(/proc)");
                return 1;
        }
        fprintf(ofp,"PID PR NI VIRT RSS SHR COMMAND CPU MEM\n");
       
    	struct dirent *entry;
    	long pid;
    	while(entry = readdir(proc)) {//fetches individual sub directory entry in /proc in each iteration
    	    if(isdigit(*entry->d_name)){
    	    	print_status(ofp,strtol(entry->d_name, NULL, 10));//will result in print_status(ofp,pid)
    	    }

    	   //sleep(3);
    	   //printf("\n");
	       }
        fprintf(ofp,"\n");
	    closedir(proc);
    }
    
	return 0;
}
