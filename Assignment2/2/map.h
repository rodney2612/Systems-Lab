struct unordered_map{
	int size;
	long page_faults;
	struct Node *map[];
};

void put(struct unordered_map *unordered_map,int key, long value,int frame_count);
long get(struct unordered_map *unordered_map,int key,int frame_count);
void removeItem(struct unordered_map *unordered_map,int key,int frame_count);
int containsItem(struct unordered_map *unordered_map,int key,int frame_count);
