#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include<time.h>
#include<stdbool.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<errno.h>
#include<syslog.h>


#define CRONTAB_FILE "/home/mycrontab"
#define NO_OF_COLS 6 //no of columns in crontab file with command counted as a single column

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the null-terminator
    result = strcpy(result, s1);
    result = strcat(result, s2);
    return result;
}

/*
    Retrieves the current time i.e hour, minute, day, month, and weekday name into an integer array and returns it
*/
void getCurrentTime(int current_time[]){
    time_t timer;
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    current_time[0] = tm_info->tm_min;  //minutes [0,59]
    current_time[1] = tm_info->tm_hour;  //hour [0,23]
    current_time[2] = tm_info->tm_mday; //day of month [1,31]
    current_time[3] = tm_info->tm_mon;   //month of year [0,11]
    current_time[4] = tm_info->tm_wday;  //day of week [0,6] (Sunday = 0)
}

/*
    It is used to match time(could be min, hour, month, day or weekday) in the format (number-number).
*/
bool matchHyphen(int time,int time1,int time2){
    if(time1 <= time && time <= time2){
        return true;
    }
    return false;
}

/*
    It is used to match time(could be min, hour, month, day or weekday) in the format (number,number...)
*/
bool matchComma(int length,char *slicedComma[],int time){
    for(int i=0; i<length; i++){
        if(time == (int)strtol(slicedComma[i],NULL,10)){
            return true;
        }
    }
    return false;
}

/*
    separates given string into array based on separator provided
*/
int tokenise(char *time,char *sliced[],char *separator){
    char *tmp = malloc(strlen(time)+1);
    tmp = strcpy(tmp,time);
    int i = 0;
    while(true){
        sliced[i] = strsep(&tmp, separator);
        if(sliced[i] == NULL) break;
        i++;
    }
    return i;
    
}

/*
    Uses tokenise,matchHyphen and matchComma functions to match time(could be min, hour, month, day or weekday) 
    whether it is in the format (number,number...) or (number-number) or (number) or (*)
*/
bool match(char *arguments[],int current_time[], int index){
    char *sliced[3];
    bool time_match = false;
    int time_hyphen = tokenise(arguments[index],sliced,"-");
    char *slicedComma[61];
    int time_comma = tokenise(arguments[index],slicedComma,",");
    if(time_comma > 1){
        time_match = matchComma(time_comma,slicedComma,current_time[index]);
        //printf("%s %s %s %s %s %d %d\n",slicedComma[0],slicedComma[1],slicedComma[2],slicedComma[3],slicedComma[5],time_match,current_time[index]);
    }

    if(time_hyphen == 2){
        time_match =  matchHyphen(current_time[index],(int)strtol(sliced[0],NULL,10),(int)strtol(sliced[1],NULL,10));
    } 

    if(time_hyphen == 1 && time_comma == 1){
        time_match = strcmp(arguments[index],"*") == 0 || current_time[index] == (int)strtol(arguments[index],NULL,10);
    }
    return time_match;
}

/*
    used to find errors in time fields eg if user enters a-z in time field
*/
bool findError(char *time,char *argv[]){
    printf("%s\n", time);
    //char *cmd = malloc(strlen("bash") + strlen("findError.sh") + strlen(time) + 3);
    char *cmd = "bash ";
    cmd = concat(cmd,argv[1]);
    cmd = concat(cmd," ");
    cmd = concat(cmd,time);
    FILE *fp = popen(cmd,"r");
    char buf[100];
    char *line = fgets(buf,sizeof(buf),fp);
    printf("%s",line);
    return strcmp(line,"false\n") == 0;

}


/*
    It is used to check whether the current time matches with an entry in the crontab file(i.e. the time in argument array).
    Uses the function match to match individual sub fields of time
*/
bool isTimeMatching(int current_time[],char *arguments[],char *argv[]){
    bool matching = true;
    for(int i=0; i<5; i++){
        if(strcmp(arguments[i],"*") != 0){//* not handled using regex
            if(findError(arguments[i],argv)){
            syslog(LOG_NOTICE,"Error in time field %s\n",arguments[i]);
            matching = false;
            break;
            }
        }
        if(i != 2 && i != 4){//we first match the time fields at index 0(minute),1(hour) and 3(month)
            matching = matching && match(arguments,current_time,i);
            syslog(LOG_NOTICE,"Matching %d %d %s\n", i, matching,arguments[i]);
            if(!matching)
                break;
        }
    }
    if(matching){//if minute, hour and month match, try to match day and weekday
        matching = matching && (match(arguments,current_time,2) || match(arguments,current_time,4));
    }
    return matching;
   
   /* return (
                 (strcmp(arguments[0],"*") == 0 || current_time[0] == (int)strtol(arguments[0],NULL,10)
                 && (strcmp(arguments[1],"*") == 0 || current_time[1] == (int)strtol(arguments[1],NULL,10)
                 && (strcmp(arguments[3],"*") == 0 || current_time[3] == (int)strtol(arguments[3],NULL,10)
                 && (strcmp(arguments[2],"*") == 0 || strcmp(arguments[4],"*") == 0
                 || current_time[2] == (int)strtol(arguments[2],NULL,10) || current_time[4] == (int)strtol(arguments[index],NULL,10))
                 );*/
}


time_t getFileModifiedTime(const char *path) {
    struct stat file_stat;
    int err = stat(path, &file_stat);
    if (err != 0) {
        perror(" [file_is_modified] stat");
        exit(errno);
    }
    return file_stat.st_mtime;
}

/*
    Creates a daemon so that the crontab program can run in the background as no user interaction is needed for cron.
*/
void createDaemon(){
        pid_t pid, sid;
        pid = fork();
        if (pid < 0) {
                exit(EXIT_FAILURE);
        }
        if (pid > 0) {
                exit(EXIT_SUCCESS);
        }
        umask(0);     
        sid = setsid();
        if (sid < 0) {
                exit(EXIT_FAILURE);
        }
        
        if ((chdir("/")) < 0) {
                exit(EXIT_FAILURE);
        }
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
}

/*
    Reads the crontab file into a dynamically created array
*/
int readFile(FILE *fp,char **arguments[]){
    //arguments = NULL;
    if (fp == NULL)
            exit(EXIT_FAILURE);
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int j = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        char *newline = malloc(strlen(line)+1);
        arguments[j] = malloc(6*strlen(line)+1);
        newline = strcpy(newline,line);         
        for(int i = 0; i < NO_OF_COLS; i++) {
            arguments[j][i] = strsep(&newline, " ");
            if(arguments[j][i] == NULL) break;
        }
        // printf("%s",newline);
        arguments[j][NO_OF_COLS-1] = concat(arguments[j][NO_OF_COLS-1]," ");
        arguments[j][NO_OF_COLS-1] = concat(arguments[j][NO_OF_COLS-1],newline);
        j++;
        }
    free(line);
    return j;
}

int main(int argc,char *argv[]) {
        createDaemon();
        syslog(LOG_NOTICE, "Cron Daemon created");
        FILE *fp = fopen(CRONTAB_FILE,"r");
        time_t prev_file_mod_time = getFileModifiedTime(CRONTAB_FILE);
        char **arguments[7];
        int current_time[5];
        int no_of_lines = readFile(fp,arguments);
        time_t timer;
        while (1) {
            time(&timer);
            /*Check if file modified and if it is, read file again into arguments array*/
            time_t current_file_mod_time = getFileModifiedTime(CRONTAB_FILE);
            if(current_file_mod_time > prev_file_mod_time){
                syslog (LOG_NOTICE, "File Modified by User at %ld", current_file_mod_time);
                prev_file_mod_time = current_file_mod_time;
                fclose(fp);
                fp = fopen(CRONTAB_FILE,"r");
                no_of_lines = readFile(fp,arguments);
            }

            getCurrentTime(current_time);
            for(int i=0; i<no_of_lines;i++){
                if(isTimeMatching(current_time,arguments[i],argv)){
                    syslog(LOG_NOTICE, "Command to be exceuted is %s \n\n", arguments[i][NO_OF_COLS - 1]);
                    system(arguments[i][NO_OF_COLS - 1]);
                }
            }
            sleep(60); /* wait 60 seconds */
        }
   exit(EXIT_SUCCESS);
}
    