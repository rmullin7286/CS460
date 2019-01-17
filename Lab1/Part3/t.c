#include "ext2.h"
#include <string.h>

#define BLK 1024
#define ENTER_KEY 13

char buf1[BLK], buf2[BLK];
static char *names[] = {"boot", "mtx"};

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

void getblk(u16 blk, char *buf)
{
    readfd( blk/18, ((blk)%18)/9, ( ((blk)%18)%9)<<1, buf);
}

u16 search(INODE * ip, char * name)
{
    int i;
    DIR * dp;
    char * cp;
    char temp[64];

    for(i = 0; i < 12 && ip->i_block[i] != 0; i++)
    {
        getblk((u16)ip->i_block[i], buf2);
        cp = buf2;
        dp = (DIR*)buf2;

        while(cp < buf2 + BLK)
        {
            strncpy(temp, dp->name, dp->name_len)[dp->name_len] = '\0';
            if(strcmp(temp, name) == 0)
                return (u16)dp->inode - 1;
            dp = (DIR*)(cp += dp->rec_len);
        }

    }

    error();
}

int main(void)
{
    INODE * ip;
    int iblk, i;
    u32 * up;
    u16 inode;

    getblk(2, buf1);
    iblk = ((GD*)buf1)->bg_inode_table;
    getblk(iblk, buf1);
    ip = (INODE*)buf1 + 1;
    
    for(i = 0; i < 2; i++)
    {
        inode = search(ip, names[i]);
        getblk(iblk + (inode / 8), buf1);
        ip = (INODE*)buf1 + (inode % 8);
    }

    getblk((u16)ip->i_block[12], buf2);
    up = (u32*)buf2;

    setes(0x1000);
    
    for(i = 0; i < 12; i++)
    {
        putc('*');
        getblk((u16)(ip->i_block[i]), 0);
        inces();
    }

    while(*up)
    {
        putc('.');
        getblk(*up++, 0);
        inces();
    }

    prints("\n\rgo?"); getc();
    return 1;
}