#include "ucode.c"

int parent(int console)
{
    while(1)
    {
        printf("INIT: wait for ZOMBIE child\n");
        int status;
        pid = wait(&status);
        if(pid == console)
        {
            printf("INIT: forks a new console login\n");
            console = fork();
            if (console)
                continue;
            else
                exec("login /dev/tty0");
        }
        printf("INIT: I just buried an orphan child proc %d\n", pid);
    }
}

int main(void)
{
    int in = open("/dev/tty0", O_RDONLY);
    int out = open("/dev/tty0", O_WRONLY);
    printf("INIT: fork a login proc on console\n");
    int console = fork();
    if(console)
        parent;
    else
        exec("login /dev/tty0");
}