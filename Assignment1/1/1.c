#include<stdio.h>
#include <linux/perf_event.h> //for perf event open syscall
#include<stdint.h> //for u_int64
#include <unistd.h> //for syscall and read
#include<string.h> //for memset
#include<sys/ioctl.h> 
#include <asm/unistd.h> //for __NR_perf_event_open
#include <sys/mman.h> //for mmap
#include <fcntl.h> //for file open
#include <sys/stat.h>
#include <sys/resource.h> //for rusage syscall


static long perf_event_open(struct perf_event_attr *hw_event,
                pid_t pid,
                int cpu,
                int group_fd,
                unsigned long flags) {
  int ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
            group_fd, flags);
  return ret;
}

/*Read file using mmap. This is the function for which page fault is counted*/
void fileRead(char const *argv[]){
//file read
    unsigned char *f;
    int size;
    struct stat s;
    //const char * file_name = argv[1];
    int fd = open (argv[1], O_RDONLY);

    /* Get the size of the file. */
    int status = fstat (fd, & s);
    size = s.st_size;

    f = (char *) mmap (0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    for (int i = 0; i < size; i++) {
        char c;
        c = f[i];
        //putchar(c);
    }
}

/*Configures the pe_event_attr stucture array which is to be used for counting 
major and minor page faults using perf event system call*/
void createStructureArray(struct perf_event_attr pe_attr_page_faults[]){
	for(int i=0;i<2;i++){
		memset(&pe_attr_page_faults[i], 0, sizeof(pe_attr_page_faults[i]));
		pe_attr_page_faults[i].size = sizeof(pe_attr_page_faults[i]);
		pe_attr_page_faults[i].type =   PERF_TYPE_SOFTWARE;
		pe_attr_page_faults[i].disabled = 1;
		pe_attr_page_faults[i].exclude_kernel = 1;//exclude kernel page faults
	}
	pe_attr_page_faults[0].config = PERF_COUNT_SW_PAGE_FAULTS_MIN;//count minor page faults
	pe_attr_page_faults[1].config = PERF_COUNT_SW_PAGE_FAULTS_MAJ;//count minor page faults

}

int main(int argc, char const *argv[]){
	struct perf_event_attr pe_attr_page_faults[2];//1 element for major fault and 1 for minor fault
	createStructureArray(pe_attr_page_faults);
	int page_faults_fd[2];
	for(int i=0;i<2;i++){
		page_faults_fd[i] = perf_event_open(&pe_attr_page_faults[i], 0, -1, -1, 0);
		if (page_faults_fd[i] == -1) {
			printf("perf_event_open failed for page faults\n");
			break;
		}
	}
	// Start counting using perf event
	for(int i=0;i<2;i++){
		ioctl(page_faults_fd[i], PERF_EVENT_IOC_RESET, 0);
		ioctl(page_faults_fd[i], PERF_EVENT_IOC_ENABLE, 0);
	}
	
	//Start counting using rusage	
	struct rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	int startMajorFaults = usage.ru_majflt;
	int startMinorFaults = usage.ru_minflt;

	/*The event for which page faults need to be counted*/
	fileRead(argv);

	// Stop counting and read value
	for(int i=0;i<2;i++){
		ioctl(page_faults_fd[i], PERF_EVENT_IOC_DISABLE, 0);
	}

	//Get value counted by rusage
  	getrusage(RUSAGE_SELF, &usage);
  	int endMajorFaults = usage.ru_majflt;
	int endMinorFaults = usage.ru_minflt;
  	printf("Major page faults with rusage %i\n",(endMajorFaults - startMajorFaults));
	printf("Minor page faults with rusage %i\n",(endMinorFaults - startMinorFaults));

	//Get value counted by perf event system call
	uint64_t minor_page_faults_count;
	uint64_t major_page_faults_count;
	read(page_faults_fd[0], &minor_page_faults_count, sizeof(minor_page_faults_count));
	read(page_faults_fd[1], &major_page_faults_count, sizeof(major_page_faults_count));
	printf("Major page faults with perf event open %ld\n",major_page_faults_count);
	printf("Minor page faults with perf event open %ld\n" ,minor_page_faults_count);
	
	return 0;
}