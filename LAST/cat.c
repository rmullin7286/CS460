#include "ucode.c"

char buffer[1024];

int main(int argc, char * argv[])
{
	if(argc > 1)
	{
		close(0);
		open(argv[1], O_RDONLY);
	}

	while(read(0, buffer, 1024))
		prints(buffer);
	
	return 0;
}
