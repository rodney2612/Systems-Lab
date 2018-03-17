struct QueueNode{
	int key;
	struct QueueNode *next;
};

struct Queue{
	struct QueueNode *front;
	struct QueueNode *rear;
	int size;
};

void enqueue(struct Queue *queue, int key);
int dequeue(struct Queue *queue);
void printQueue(struct Queue *queue);
