#include "ucode.c"

char buffer[1024];

int main(int argc, char * argv[])
{
	if(argc < 3)
	{
		printf("Usage: cp src dest\n");
		return 0;
	}
	close(0);
	open(argv[1], O_RDONLY);
	close(1);
	open(argv[2], O_WRONLY|O_CREAT);
	while(read(0, buffer, 1024))
	{
		buffer[strlen(buffer) - 1] = 0;
		prints(buffer);
	}

	return 0;
}
