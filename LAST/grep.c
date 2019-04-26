#include "ucode.c"

int issubstr(char * sub, char * full)
{
	for(; *full != '\0'; full++)
		if(*full == *sub)
			for(char * sub2 = sub; *full != '\0'; sub2++, full++)
				if(*sub2 == '\0')
					return 1;
				else if(*sub2 != *full)
					break;
	return 0;
}

int main(int argc, char * argv[])
{
	char buffer[1024];

	if(argc < 2)
	{
		prints("Usage: grep pattern [file]\n");
		return 0;
	}
	
	if(argc > 2)
	{
		close(0);
		open(argv[2], O_RDONLY);
	}

	while(getline(buffer))
		if(issubstr(argv[1], buffer) == 1)
			prints(buffer); //for some reason printf isn't working in this program

	return 0;
}
