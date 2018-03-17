#include<stdio.h>
#include<stdlib.h>
#include "ll.h"

void insertNode(struct Node **head, int key, long value){
	struct Node *last = *head;
	struct Node *new_node = malloc(sizeof(struct Node));
	new_node->key = key;
	new_node->value = value;
	new_node->next = NULL;
	if (*head == NULL){
       *head = new_node;
       return;
    }
    while (last->next != NULL){
        last = last->next;
    }

    last->next = new_node;
    return;    
}

void removeNode(struct Node **head,int key){
	struct Node *iter = *head;
	if (iter != NULL && iter->key == key)
    {
        *head = iter->next;
        free(iter);
        return;
    }
	struct Node *prev = NULL;
	while(iter != NULL && iter->key != key){
		prev = iter;
		iter = iter->next;
	}
	if (iter == NULL){
		return;
	} 	
	prev->next = iter->next;
	free(iter);
}

void changeValue(struct Node *head, int key, long value){
	struct Node *iter = head;
	while(iter != NULL && iter->key != key){
		iter = iter->next;
	}
	if(iter != NULL){
		iter->value = value;
	}
}

long getValue(struct Node *head, int key){
	struct Node *iter = head;
	while(iter != NULL && iter->key != key){
		iter = iter->next;
	}
	return iter !=NULL ? iter->value : -1;
}

int contains(struct Node *head, int key){
	struct Node *iter = head;
	while(iter != NULL && iter->key != key){
		iter = iter->next;
	}
	if(iter == NULL){
		return 0;
	}
	return 1;
}

void printList(struct Node *head){
    struct Node *iter = head;
    while (iter != NULL)
    {
        printf(" %d ", iter->key);
        printf(" %ld ", iter->value);
        iter = iter->next;
    }
}
