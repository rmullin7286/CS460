#include "ucode.c"

char buffer[1024];

int main(int argc, char * argv[])
{
	if(argc == 2)
	{
		close(0);
		open(argv[1], O_RDONLY);
		close(1);
		open(argv[2], O_WRONLY|O_CREAT);
	}

	while(getline(buffer))
		
}
