#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<errno.h>
 
#define GRN "\x1B[32m"
#define BLU "\x1B[34m"
#define WHT  "\x1B[37m"
#define NO_OF_BUILTINS 2
#define MAX_PARAMS 30

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the null-terminator
    result = strcpy(result, s1);
    result = strcat(result, s2);
    return result;
}

//Replaces /home/username with ~
//todo:change so taht only 6home/username and not /home/anyotherfolder changes to ~
char* replaceWithTilde(char *pwd){
	char* username = getenv("USER");
	char *dest = malloc(strlen(pwd)); //used to store initial 6 characters of the command so as to compare with /home/
	strncpy(dest, pwd, 6);
	printf("dest %s pwd %s",dest,pwd);
	dest = strcat(dest,username);
	char *path = malloc(strlen(pwd));
	char *home = "/home/";
	path = strcat(home,username);//error on this statement
	printf("%s", path);
	//printf("dest %s path %s",dest,path);
	/*if(strcmp(dest,path)){
		printf("~");
		pwd += 6; //getting past /home/
		while(1){
			if(*pwd == '/'){ //trying to get past the / after the user name
				break;
			}
			pwd++;
		}
	}*/
	return pwd;
}

/*Displays username, hostname and current directory just as in linux shell
Currently ~ is not displayed instead /home/username is only displayed*/
void printPrompt(){
	char* username = getenv("USER");
	printf(GRN "\n%s@", username);
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	printf("%s:", hostname);
	char pwd[1024];
   	getcwd(pwd, sizeof(pwd));
	//char* processedPwd = replaceWithTilde(pwd);
	//printf(BLU "%s", processedPwd);
	printf(BLU "%s", pwd);
}


// Function to get user command
int getCommand(char *cmd){
	printf(WHT);
    char *buf = readline("$");
	if (strlen(buf) != 0) {
		add_history(buf); //Adding command to history so that we can get the command quickly next time by pressing up and down keys
		strcpy(cmd, buf);
        return 0;
	} 
        return 1;
}

/*This function checks if the entered command is supported and if supported execues the command using execv 
by passing to it the command executable.*/
void processCommand(char **arguments){
	printf("Command :%s %s",arguments[0], arguments[1]);
	int commandSupported = strcmp(arguments[0],"cd") == 0 || strcmp(arguments[0],"cat") == 0
	|| strcmp(arguments[0],"mkdir") == 0 || strcmp(arguments[0],"top") == 0;
	if(commandSupported == 0){
		printf("Command not supported");
	}
	else{
		execv(arguments[0],arguments);
		//If Error occurred while executing command using execv
        char* error = strerror(errno);
        printf("shell: %s: %s\n", arguments[0], error);
	}
}

/*Checks if the entered command is builtin*/
int isBuiltin(char **arguments){
	if (strcmp(arguments[0], "cd") == 0 || strcmp(arguments[0], "help") == 0) 
	{
		return 1;	
	} 
	return 0;
}

/*Displays help for the supported command names. ex. help cd will result in cd.txt being opened and printed on the screen*/
int printHelp(char *fileName)
{
	FILE *ifp;
	int ifd=0; 
	
	fileName = concat(fileName,".txt");
	//printf("%s",fileName);
	ifp=fopen(fileName, "r");
	ifd = fileno(ifp);
    char buffer[4096];
    int noOfBytes;
    while ((noOfBytes = read(ifd, buffer, sizeof(buffer))) > 0)
    {
        if (write(1, buffer, noOfBytes) != noOfBytes)
            return -1;
    }
    return (noOfBytes < 0) ? -1 : 0;
}

/*Used to execute builtin commands(commands which dont require forking a child process) i.e cd and help*/
void execBuiltin(char **arguments){
	if(strcmp(arguments[0], "cd") == 0){
		int status = chdir(arguments[1]);
		//printf("Status of cd: %i",status);
		char* error = strerror(errno);
		printf("cd: %s\n", error);
	}
	else{ //help
		char *builtins[NO_OF_BUILTINS] = {"cd", "help"};
		for(int i=0; i<NO_OF_BUILTINS; i++){
		
			if(strcmp(arguments[0],builtins[i]) == 0){
				printHelp(arguments[1]);
				//printf("helpp %s ",arguments[0]);
			}
		}
	}
}

/*Used to tokenise the entered command into individual strings which are stored in an array*/
void parseCommand(char *cmd, char **arguments){
	for(int i = 0; i < MAX_PARAMS; i++) {
        arguments[i] = strsep(&cmd, " ");
        if(arguments[i] == NULL) break;
    }
}

/*Gets user command, parses it into tokens and then executes the command based on whether 
it is builtin or requires a new pprocess to be created*/
int main(){
	system("clear");
	char cmd[2000];
	/*stores each token of the command eg for cd dirname arguments[o] will store cd and arguments[1] will store dirname*/
	char *arguments[MAX_PARAMS];
	while(1){
		printPrompt();
		getCommand(cmd);
		parseCommand(cmd, arguments);
		
		if(strcmp(arguments[0],"exit") == 0){
			break;
		}

		if (isBuiltin(arguments)){
		    execBuiltin(arguments);
		} 
		else {		
			int childPid = fork();
			if (childPid == -1) {
		        char* error = strerror(errno);
		        printf("fork: %s\n", error);
    		}
			else if (childPid == 0){//child which executes the commands using execv
				processCommand(arguments);
			} 
			else {//parent(main shell) waits for the child to complete
				int status;
				childPid = wait(&status);
		    	}
	    }	
		
	}
	return 0;
}
