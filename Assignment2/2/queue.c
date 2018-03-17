#include<stdio.h>
#include<stdlib.h>
#include "queue.h"

void enqueue(struct Queue *queue, int key){
	struct QueueNode *newQueueNode = malloc(sizeof(struct QueueNode));
	newQueueNode->key = key;
	newQueueNode->next = NULL;
	queue->size += 1;
	if(queue->rear == NULL){
		queue->front = queue->rear = newQueueNode;
		return;
	}
	queue->rear->next = newQueueNode;
	queue->rear = newQueueNode;
}

int dequeue(struct Queue *queue){
	if(queue->front == NULL){
		printf("Queue empty");
		return -1;
	}
	int key = queue->front->key;
	queue->front = queue->front->next;
	if(queue->front == NULL){
		queue->rear = NULL;
	}
	queue->size -= 1;
	return key;
}

void printQueue(struct Queue *queue){
	struct QueueNode *iter = queue->front;
	while(iter != NULL){
		printf("%d\n",iter->key);
		iter = iter->next;
	}
}

/*Queue unit testing*/
/*int main(){
	struct Queue *queue = malloc(sizeof(struct Queue));
	for(int i=0;i<1024;i++){
		enqueue(queue,i);
	}
	printQueue(queue);
	dequeue(queue);
	
}*/