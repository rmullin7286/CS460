#include "ucode.c"

int stdin_copy;
char buffer[1024];

char * stdin_name = "/dev/tty0";

int main(int argc, char * argv[])
{
	int real_stdin = open("/dev/tty0", O_RDONLY);

	if(argc > 1)
	{
		close(0);
		open(argv[1], O_RDONLY);
	}

	int n = 20;
	while(getline(buffer))
	{
		for(int i = 0; i < n; i++)
		{
			prints(buffer);
			if(!getline(buffer))
				return;
		}
		char c;
		while(1)
		{
			read(real_stdin, &c, 1);
			if(c == ' ')
			{
				n = 20;
				break;
			}
			else
			{
				n = 1;
				break;
			}
		}
	}	
}
