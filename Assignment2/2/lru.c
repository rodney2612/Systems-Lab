#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<sys/time.h>
#include "ll.h"
#include "map.h"

int removeLRUPage(struct unordered_map *unordered_map,int frame_count){
	long min = 9999999999999;
	int lruPage = 0;
	for(int i=0; i<frame_count; i++){
		struct Node *head = unordered_map->map[i];
		while(head != NULL){
			if(min > head->value){
				min = head->value;
				lruPage = head->key;
			}
			head =  head->next;
		}
	}
	printf("LRU page no %d \n",lruPage);
	removeItem(unordered_map,lruPage,frame_count);
	return lruPage;
}


int lru(struct unordered_map *unordered_map,int page_no,int frame_count){
	int lruPage = -1;
	if (unordered_map->size >= frame_count){
		if(!containsItem(unordered_map,page_no, frame_count)){
			lruPage = removeLRUPage(unordered_map,frame_count);
		}
	}
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long timeSinceEpoch =
    (unsigned long long)(tv.tv_sec) + (unsigned long long)(tv.tv_usec);//sconds since epoch + microseconds since epoch
	put(unordered_map,page_no,timeSinceEpoch,frame_count);
	return lruPage;
}

/*LRU unit testing*/
/*int main(){
	struct unordered_map *unordered_map = malloc(sizeof(struct unordered_map));
	for(int i=0;i<MAP_SIZE;i++){
		unordered_map->map[i] = NULL;
	}
	for(int i=0;i<2048;i++){
		lru(unordered_map,i);
	}
	lru(unordered_map,1044);
	lru(unordered_map,2);
	lru(unordered_map,555);
	lru(unordered_map,980);
	lru(unordered_map,1552);
	lru(unordered_map,1944);
	lru(unordered_map,2043);
	printf("%ld ",unordered_map->page_faults);
	return 1;
}*/