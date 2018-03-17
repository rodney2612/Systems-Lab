#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>


//supported arguements -> -E, -n
//-E still not working properly for every case, Only works for cat -E filename
int isSupportedArg(char arg){
	switch(arg){
		case 'E':
		case 'n':
		return 1;
		default: return 0;
	}
}

/*Checks if passed arguement array(which contains the entered command) contains a given string.*/
int contains(char *args[],char *arg){
	for(int i=0;args[i]!=NULL;i++){
		if(strcmp(args[i],arg) == 0){
			return 1;
		}
	}
	return 0;
}

/*This function is called when user enters cat > filename or cat >> filename or cat.
Reading input char by char is essential since we have to detect when user presses 
CTRL+D which will be difficult with line by line reading or reading entire file in one go.*/
void writeCharByChar(int ifd, FILE *ofp)
{
	int ofd = fileno(ofp);
    char ch;
    while (read(ifd, &ch, 1) > 0)
    {
        write(ofd, &ch, 1);
    }
}

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the null-terminator
    result = strcpy(result, s1);
    result = strcat(result, s2);
    return result;
}

/*Used to append a dollar to end of line when user passes -E option to the cat command*/
char * appendDollar(char *line){
	char *result = malloc(strlen(line)+2);
    result = strchr(line, '\n');
    *result = '$';
    result = concat(line,"\n");
    return result;
}

/*This function is called when user wants to read file(s) using cat filename(s) or when
 he wants to concatenate files using > or >> operators. The number of input files that 
 is supported has been limited to 9. Also -n and -E arguments are supported by this function
 This function returns value of i to the calling function to support commands like
 cat -n f1 f2 f3 f4 or cat -n f1 f2 > f3. Also ;line by line writing is used to support 
 -n and -E else would have read the entire file in 1 go*/
int writeLineByLine(FILE *ifp,FILE *ofp, char *args[], int i){
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    if (ifp == NULL)
        exit(EXIT_FAILURE);
    while ((read = getline(&line, &len, ifp)) != -1) {
       // printf("Retrieved line of length %zu :\n", read);
       // printf("%d %s", ++i, line);
    	int containsN = contains(args, "-n");
    	int containsE = contains(args,"-E");
    	if(containsN && containsE){
			fprintf (ofp, "\t %d %s",++i,appendDollar(line));
		}
		else if(containsN && !containsE){
			fprintf (ofp, "\t %d %s",++i,line);
		}
		else if(!containsN && containsE){
    		fprintf (ofp, "%s",appendDollar(line));
		}
		else{
			fprintf (ofp, "%s",line);
		}
        
    }

    free(line);
    return i;
}

/*This function simply reads the array of input files and passes them to the writeLineByLine
  function for writing/appending to the desired file.*/
void catForArrayOfFiles(char *args[],char *files[], int k, char *mode, FILE *ofp){
	FILE *ifp[k+1];
	int start = 0; 
	for(int i=0; files[i] != NULL; i++){
		//printf("File name %s \n",files[i]);
		ifp[i] = fopen(files[i],"r");
		//ifd[i] = fileno(ifp[i]);
		start = writeLineByLine(ifp[i],ofp, args,start);
	}
}

int main(int argc, char *argv[]){
	//printf("%i",argc);
	if(argc == 1){//user enterd cat without any argument
		writeCharByChar(0,fdopen(1,"w"));
	}

	char *args[4];//for storing arguements i.e -m, -p, -v
	char *files[10];//for storing input file names i.e the files names passed to cat other than the one next to > or >> operators
	int j=0,k=0;
	char mode = 'n';//Used to store wheteherfile should be opened in append or write mode.
	char *outFile = NULL;//for storing output file name i.e the file anme after > or >> operators
	
	/*This loop is used to separate the entered command into options like -n -E and filenames
	This function supports commands like cat -n f1 f2 f3 > f4 or cat f1 f2 -n > f3 f4 anso on.
	That is, the positon of the option and filenames dont matter. Only thing that matters is 
	a filename being given after the > or >> operators(this is the output filename). 
	The rest of the arguments which dont start with - are considered as input file names*/
	for(int i = 1; argv[i] != NULL ;i++){
		printf(" Arg %s \n",argv[i] );
		switch(argv[i][0]){
			case ' ':
						break;
			case '>':
						//printf("> case detected");
						if(argv[i][1] && argv[i][1] == '>'){ //if >>
							mode = 'a';
							if(argv[i][2]){ //if >>filename
								outFile = malloc(strlen(argv[i]));
								outFile = strcpy(outFile,++argv[i]);
							}

						}
						else if(!argv[i][1]){//if >
							mode = 'w';
						}
						else{ //if >someOutputFileName
							outFile = malloc(strlen(argv[i]));
							outFile = strcpy(outFile,++argv[i]);
						}
						if(!outFile){//if argument contained only > or >> and not >filename
							if(argv[++i]){
								outFile = malloc(strlen(argv[i]) + 1);
								outFile = strcpy(outFile,argv[i]);
							}
							else{//user entered nothing after the > or >> operator
								printf("bash: syntax error near unexpected token `newline'");
								return 1;
							}
							
						}
			break;

			case '-':
						//printf("- case detected");
						;
						int isSupported = isSupportedArg(argv[i][1]);
						if(isSupported && strlen(argv[i])== 2){
							args[j] = malloc(strlen(argv[i]) + 1);
							args[j] = strcpy(args[j],argv[i]);
						}
						else{
							printf("Unsupported arguments");
							return 1;
						}
							//printf("jth arg%s\n",args[j] );
							j++;
					
			break;

			default: //is a file name, not an arguement like -n or operator like >,>>
						//printf("file name case detected %s \n",argv[i]);
						if(strlen(argv[i])>0){
							if( access(argv[i], F_OK ) == -1 ) {
							printf(" %s ",argv[i]);
						    printf("File doesn't exist");
						    return 1;
						}
							//file exists
							files[k] = malloc(strlen(argv[i]) + 1);
							files[k] = strcpy(files[k],argv[i]);
							k++;
							
						}
						break;
						
					
		}
	}
	args[j] = NULL;
	files[k] = NULL;
	/*for(int i=0;args[i]!= NULL;i++){
		printf(" %s ",args[i]);
	}
	for(int i=0;files[i]!= NULL;i++){
		printf(" %s ",files[i]);
	}*/
	FILE *ofp;
	switch(mode){
		//'a' means  >> is there in argument list
		case 'a': 
		//'w' means > is there in argument list
		case 'w':{ 
					ofp = fopen(outFile, &mode);
				}
					
		break;
		//mode being 'n' results in default case where we dont have redirection operators. We only have cat [filename(s)]
		default:
					//ofd = 1;//std output file descriptor
					ofp = fdopen(1, "w");
	}

	if(files[0] == NULL){//No input files i.e user executed cat >/>> filename(s)
		ofp = fopen(outFile, &mode);
		writeCharByChar(0,ofp);
	}
	else{
		catForArrayOfFiles(args,files,k,&mode,ofp);
	}

	return 0;
}
