#define MAX 24
#define START_ID 1

struct locker {
	char name[MAX];
	int id;
	char thing[MAX];
	struct locker *next;
};
