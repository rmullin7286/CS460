#include "ucode.c"

char tokens[7];

void get_tokens(char * line)
{
    for(int i = 0; i < 7; i++)
    {
        tokens[i] = line;
        while(*line != ':' || *line != '\0')
            line++;
        *line = '\0';
        line++;
    }
}

char name[50], password[50];

int main(int argc, char * argv[])
{
    //close file descriptors for STDIN(0) and STDOUT(1)
    close(0);
    close(1);

    //open argv[1] 3 times for STDIN(0), STDOUT(1), and STDERR(2)
    int in = open(argv[1], O_RDONLY), out = open(argv[1], O_WRONLY), err = open(argv[1], O_WRONLY);
    fixtty(argv[1]);
    while(1)
    {
        printf("login: "); gets(name);
        printf("password: "); gets(password);
        
        int in_copy = dup(in);
        close(0);
    
        int pswd_file = open("/etc/passwd", O_RDONLY);
        dup2(pswd_file, 0);

        char line[256];
        while(getline(line))
        {
            get_tokens(line);
            if(strcmp(tokens[0], name) == 0 && stcmp(tokens[1], password) == 0)
            {
                chuid(atoi(tokens[3]), atoi(tokens[2]));
                chdir(tokens[5]);
                close(0);
                dup2(in_copy, 0);
                close(in_copy);
                exec(tokens[6]);
            }
        }

        close(0);
        dup2(in_copy, 0);
        close(in_copy);
    }
}