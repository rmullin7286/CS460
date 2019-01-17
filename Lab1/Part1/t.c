#define ENTER_KEY 13

void prints(char * s)
{
    while(*s != '\0')
        putc(*s++);
}

void gets(char * s)
{
    char in;
    while((in = getc()) != ENTER_KEY)
        putc((*s++ = in));
    *s = '\0';
}

int main(void)
{
    char ans[64];
    while(1)
    {
        prints("What's your name? ");
        gets(ans); prints("\n\r");
        if(ans[0] == '\0')
        {
            prints("return to assembly and hang\n\r");
            return 1;
        }
        prints("Welcome "); prints(ans); prints("\n\r");
    }
}