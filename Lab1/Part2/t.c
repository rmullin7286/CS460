#include "ext2.h"
#include <string.h>

#define BLK 1024
#define ENTER_KEY 13

GD *gp;
INODE *ip;
DIR *dp;

char buf1[BLK], buf2[BLK];
u8 ino;

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

int main(void)
{
    u16 i, iblk;
    char temp[256];

    prints("read block #2 (GD)\n\r");
    getblk(2, buf1);

    //1. Get iblk = bg_inode_table block number
    iblk = ((GD*)buf1)->bg_inode_table;
    prints("inode_block="); putc(iblk + '0'); prints("\n\r");

    //2. Get root inode
    prints("read inodes begin block to get root inode\n\r");
    getblk(iblk, buf1);
    ip = (INODE*)buf1 + 1;

    //3. Print root directory
    prints("**********************\n\r");
    prints("Printing root directory\n\r");
    for(i = 0; i < 12 && ip->i_block[i]; i++)
    {
        getblk((u16)ip->i_block[i], buf2);
        dp = ((DIR*)buf2);

        while((char*)dp < buf2 + BLK)
        {
            strncpy(temp, dp->name, dp->name_len)[dp->name_len] = '\0';
            prints(temp); prints("\n\r");
            dp = (DIR*)((char*)dp + dp->rec_len);
        }
    }
    prints("DONE");
    return 1;
}