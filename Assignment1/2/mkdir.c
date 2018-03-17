#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include<string.h>
#include<ctype.h>

/*"-m","-v","-p" are currently supported by mkdir*/
int isSupportedArg(char arg){
	switch(arg){
		case 'm':
		case 'v':
		case 'p':
		return 1;
		default: return 0;
	}
}

int contains(char *args[],char *arg){
	for(int i=1;args[i]!=NULL;i++){
		if(strcmp(args[i],arg) == 0){
			return 1;
		}
	}
	return 0;
}

/*used to make only a single directory ex. mkdir filename*/
void makeDirectory(char *dirName, mode_t mode, char *args[]){
		int status = mkdir(dirName, mode);
		
		if(status == -1){
			perror("mkdir");
			//printf("mkdir: cannot create directory '%s' : No such file or directory",argv[1]);
		}
		else{
			if(contains(args,"-v")){
				printf("mkdir: Created directory %s ",dirName);
			}
		}
}

/*Used for -p option of mkdir ex. if user enters mkdir -p f1/f2/f3, this fuction will create the entire structure f/f2/f3
This function just creates the first directory in the structure and then parses the sting to create the 
subdirectories using the above function makeDirectory to create a single directory.*/
void recursiveCreate(char *dir, mode_t mode, char *args[]){
			char tmp[256];
	        char *p = NULL;
	        size_t len;

	        snprintf(tmp, sizeof(tmp),"%s",dir);
	        len = strlen(tmp);
	        if(tmp[len - 1] == '/'){
	                tmp[len - 1] = 0;
	        }
	        for(p = tmp + 1; *p; p++)
	                if(*p == '/') {
	                    *p = 0;
	                    //printf("%s\n",tmp);
	                    makeDirectory(tmp, mode,args);
	                    *p = '/';
	                }
	        makeDirectory(tmp, mode,args);	
}

/*Just checks if user entered -p option or not and calls appropriate function based on that*/
void createDirectory(char *dirName, mode_t mode, char *args[]){
	for(int i=0;args[i]!=NULL;i++){
		printf("%s",args[i]);
	}
	if(contains(args,"-p")){
		printf(" -p ");
		recursiveCreate(dirName,mode,args);	
	}
	else{
		makeDirectory(dirName,mode, args);
	}	
}

int main(int argc, char *argv[]){
	if(argc<2){//User enterd mkdir
		printf("mkdir: missing operand\n");
		printf("Usage: mkdir [OPTION]... DIRECTORY...\n");
		return 1;
	}
	char *args[4];//for storing arguments i.e -m, -p, -v
	char *dirs[8];//for storing directory names
	int j=0,k=0;
	mode_t mode = 0755;//our default mode 755 is rwx for owner rx for group and others

	/*This loop is used to separate the entered command into options like -m,-v and directory names.
	This loop supports commands like mkdir -m mode -v -p d1 d2 or mkdir d1 -m mode -p d2 -v   and so on.
	That is, the positon of the options and directory names dont matter. The arguments which dont start 
	with - are considered as the directories that need to be created*/
	for(int i = 1; i < argc;i++){
		if(argv[i][0] == '-'){
			int isSupported = isSupportedArg(argv[i][1]);
			if(isSupported && strlen(argv[i]) == 2){
				args[j] = malloc(strlen(argv[i]) + 1);
				args[j] = strcpy(args[j],argv[i]);
				if(argv[i][1] == 'm'){
					char *ptr;
					/*If user enterd -m then the very next arguement needs to be mode number else error*/
	    			mode = strtol(argv[++i], &ptr, 8);
	    			//printf("Mode %o",mode);
	    			if(strlen(ptr) != 0){//mode contains characters
	    				printf("Invalid mode %s ",argv[i]);
						return 1;
	    			}
				}
			}
			else{
				printf("Unsupported arguements");
				return 1;
			}
				//printf("jth arg%s\n",args[j] );
				j++;
		}
		else{ //is a directory name, not an arguement like -p
			dirs[k] = malloc(strlen(argv[i]) + 1);
			dirs[k] = strcpy(dirs[k],argv[i]);
			k++;
		}
	}
	args[j] = NULL;
	dirs[k] = NULL;
	for(int i=0;dirs[i] != NULL;i++){
		createDirectory(dirs[i], mode, args);
	}
	
	
	return 0;
}
