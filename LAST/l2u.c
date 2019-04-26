#include "ucode.c"

char buffer[1024];

int islower(char c)
{
	return c >= 'a' && c <= 'z';
}

void toupper(char * c)
{
	int len = strlen(c);
	for(int i = 0; i < len; i++)
		c[i] = islower(c[i]) ? c[i] + 'A' - 'a' : c[i];
}

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
	{
		toupper(buffer);
		prints(buffer);
	}
		
}
