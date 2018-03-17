#include<stdio.h>
#include<stdlib.h>
#include "ll.h"
#include "map.h"


int containsItem(struct unordered_map *unordered_map,int key,int frame_count){
	int position = key % frame_count;
	return (getValue(unordered_map->map[position],key)) == -1 ? 0 : 1;
}

void put(struct unordered_map *unordered_map,int key, long value,int frame_count){
	int position = key % frame_count;
	if(containsItem(unordered_map,key,frame_count)){
		changeValue(unordered_map->map[position],key,value);
	}
	else{
		insertNode(&(unordered_map->map[position]),key,value);
		unordered_map->size += 1;
		unordered_map->page_faults += 1;
	}
}

long get(struct unordered_map *unordered_map,int key,int frame_count){
	int position = key % frame_count;
	return getValue(unordered_map->map[position],key);
}



void removeItem(struct unordered_map *unordered_map,int key,int frame_count){
	int position = key % frame_count;
	removeNode(&(unordered_map->map[position]),key);
	unordered_map->size -= 1;
}

/*Map unit testing*/
/*int main(){
	unordered_map = malloc(sizeof(struct unordered_map));
	put(1,10);
	put(2,11);
	put(3,12);
	printf("%d",get(3));
	put(4,13);
	put(3,15);
	removeItem(2);
	printf(" %d ",get(2));
	printf(" %d ",get(3));
}*/