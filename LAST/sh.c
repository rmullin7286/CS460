#include "ucode.c"

void waitpid(int pid)
{
	int status;
	while(wait(&status) != pid);
}

void redirect(char * command)
{
	int len = strlen(command);
	for(int i = 0; i < len; i++)
	{
		if(command[i] == '>')
		{
			if(i < len - 1 && command[i + 1] == '>')
			{
				command[i] = command[i + 1] = '\0';
				char * filename = strtok(command + i + 2, ' ');
				close(1);
				open(filename, O_WRONLY|O_APPEND);
			}
			else
			{
				command[i] = '\0';
				char * filename = strtok(command + i + 1, ' ');
				close(1);
				open(filename, O_WRONLY|O_CREAT);
			}
		}
		else if(command[i] == '<')
		{
			command[i] = '\0';
			char * filename = strtok(command + i + 1, " ");
			close(0);
			open(filename, O_RDONLY);
		}
	}

}

void run_commands_helper(char ** commands)
{
	char * head = *commands;
	redirect(head);
	char ** tail = commands + 1;
	if(*tail == NULL)
	{
		exec(head);
		exit(0);
	}
	else
	{
		int pid, pd[2];
		pipe(pd);
		pid = fork();
		if (pid)
		{
			close(pd[1]);
			dup2(pd[0], 0);
			run_commands_helper(tail);
		}
		else
		{
			close(pd[0]);
			dup2(pd[1], 1);
			exec(head);
		}
	}
}

void run_commands(char ** commands)
{
	char copy[100];
	strcpy(copy, *commands);
	char * tok = strtok(copy, ' ');
	if(strcmp(tok, "cd") == 0)
	{
		tok = strtok(NULL, ' ');
		chdir(tok);
		return;
	}
	int pid = fork();
	if(pid)
	{
		int status;
		while(1)
		{
			int child = wait(&status);
			if(child == pid) return;
		}
	}
	else
	{
		run_commands_helper(commands);
	}
}

int is_logout(char * input)
{
	char copy[100];
	strcpy(copy, input);
	char * tok = strtok(copy, ' ');
	printf(tok);
	return strcmp(copy, "logoff") == 0;
}

char * leading_wspace(char * s)
{
	while(*s == ' ')
		s++;
	return s;
}

int main(int argc, char argv[])
{
	char input[1000];
	char *piped_commands[50];

	while(1)
	{
		printf("> ");
		gets(input);

		piped_commands[0] = strtok(input, '|');

		for(int i = 1; piped_commands[i] = strtok(NULL, '|'); i++);
		if(is_logout(piped_commands[0]))
			return;
		for(int i = 1; piped_commands[i] != NULL; i++)
			piped_commands[i] = leading_wspace(piped_commands[i]);
		run_commands(piped_commands);
	}

}
