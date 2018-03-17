#include<stdio.h>
#include<stdlib.h>
#include "queue.h"

int fifo(struct Queue *queue,int page_no,int frame_count){
	int dequeued = -1;
	if(queue->size >= frame_count){
		dequeued = dequeue(queue);
	}
	enqueue(queue,page_no);
	return dequeued;
}

/*Queue unit test*/
/*int main(){
	struct Queue *queue = malloc(sizeof(struct Queue));
	for(int i=0;i<2048;i++){
		fifo(queue,i);
	}
	printQueue(queue);
	printf("%d",queue->size);
	return 0;
}*/