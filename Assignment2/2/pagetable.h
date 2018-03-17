struct PageTable{
	int valid;
	int frame; //alloted frame
	int dirty;
	int requested; //pid of process which requested a frame for the page. 0 if not requested
};

struct PageTable *pt;