#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD "hello"

typedef struct{
	int a;
	int b;
}add_t;

int main(int argc, char const *argv[])
{
	/* code */
	add_t add;
	char buf[100];

	add.a = 1;
	add.b = -1;

	//snprintf(buf, sizeof(buf), "%s", (char *)&add);
	//snprintf(buf, sizeof(buf), "%d%d", add.a, add.b);

	memcpy(buf, (char *)&add, sizeof(add));

	printf("buf is %*.s, len is %d\n",8, buf, (int)strlen(buf));



	printf("a is %d\n", *(int *)(buf));
	printf("b is %d\n", *((int *)buf +1));

	add_t *test = (add_t *)buf;


	printf("%d\n", test->a );
	//printf("a is %d, b is %d\n", *(int *)((add_t *)buf->a), *(int *)((add_t *)buf->b));
	return 0;
}