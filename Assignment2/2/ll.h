struct Node{
	int key;
	long value;
	struct Node *next;
};
void insertNode(struct Node **head, int key, long value);
void removeNode(struct Node **head,int key);
int contains(struct Node *head, int key);
void printList(struct Node *head);
void changeValue(struct Node *head, int key, long value);
long getValue(struct Node *head, int key);
