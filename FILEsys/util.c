/*********** util.c file ****************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

//#include "type.h"

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;
extern char gpath[256];
extern char *name[64];
extern int n;
extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, inode_start;
extern char line[256], cmd[32], pathname[256];


int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   
int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}   

int tokenize(char *pathname)
{
  int i;
  char *s;
  printf("tokenize %s\n", pathname);

  strcpy(gpath, pathname);
  n = 0;

  s = strtok(gpath, "/");
  while(s){
    name[n] = s;
    n++;
    s = strtok(0, "/");
  }

  for (i= 0; i<n; i++)
    printf("%s  ", name[i]);
  printf("\n");
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, disp;
  INODE *ip;

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       //printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
    
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
       //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev;
       mip->ino = ino;

       // get INODE of ino to buf    
       blk  = (ino-1) / 8 + inode_start;
       disp = (ino-1) % 8;

       //printf("iget: ino=%d blk=%d disp=%d\n", ino, blk, disp);

       get_block(dev, blk, buf);
       ip = (INODE *)buf + disp;
       // copy INODE to mp->inode
       mip->inode = *ip;
       return mip;
    }
  }   
  printf("PANIC: no more free minodes\n");
  return 0;
}

iput(MINODE *mip)
{
 int i, block, offset;
 char buf[BLKSIZE];
 INODE *ip;
 if (mip==0) 
     return;

 mip->refCount--; 
 if (mip->refCount > 0) return;
 if (!mip->dirty)       return;
 
 /* write back */
 printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino); 

 block =  ((mip->ino - 1) / 8) + inode_start;
 offset =  (mip->ino - 1) % 8;

 /* first get the block containing this inode */
 get_block(mip->dev, block, buf);

 ip = (INODE *)buf + offset;
 *ip = mip->inode;

 put_block(mip->dev, block, buf);

} 

int search(MINODE *mip, char *name)
{
   int i; 
   char *cp, c, sbuf[BLKSIZE];
   DIR *dp;
   INODE *ip;

   printf("search for %s in MINODE = [%d, %d]\n", name,mip->dev,mip->ino);
   ip = &(mip->inode);

   /**********  search for a file name ***************/
   for (i=0; i<12; i++){ /* search direct blocks only */
        if (ip->i_block[i] == 0) 
           return 0;
        printf("search: i=%d  i_block[%d]=%d\n", i, i, ip->i_block[i]);
	//getchar();

        get_block(dev, ip->i_block[i], sbuf);
        dp = (DIR *)sbuf;
        cp = sbuf;
        printf("   i_number rec_len name_len    name\n");

        while (cp < sbuf + BLKSIZE){
	    c = dp->name[dp->name_len];
            dp->name[dp->name_len] = 0;
           
            printf("%8d%8d%8u        %s\n", 
                    dp->inode, dp->rec_len, dp->name_len, dp->name);
            if (strcmp(dp->name, name)==0){
                printf("found %s : ino = %d\n", name, dp->inode);
                return(dp->inode);
            }
            dp->name[dp->name_len] = c;
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
   }
   return(0);
}

int getino(char *pathname)
{
  int i, ino, blk, disp;
  char buf[BLKSIZE];
  INODE *ip;
  MINODE *mip;

  printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
      return 2;

  if (pathname[0]=='/')
    mip = iget(dev, 2);
  else
    mip = iget(running->cwd->dev, running->cwd->ino);

  strcpy(buf, pathname);
  tokenize(buf);

  for (i=0; i<n; i++){
      printf("===========================================\n");
      printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);
 
      ino = search(mip, name[i]);

      if (ino==0){
         iput(mip);
         printf("name %s does not exist\n", name[i]);
         return 0;
      }
      iput(mip);
      mip = iget(dev, ino);
   }
   return ino;
}

int findmyname(MINODE *parent, u32 myino, char *myname) 
{
 int i;
 char buf[BLKSIZE], temp[256], *cp;  
 DIR    *dp;
 MINODE *mip = parent;

 /**********  search for a file name ***************/
 for (i=0; i<12; i++){ /* search direct blocks only */
     if (mip->inode.i_block[i] == 0) 
           return -1;

     get_block(mip->dev, mip->inode.i_block[i], buf);
     dp = (DIR *)buf;
     cp = buf;

     while (cp < buf + BLKSIZE){
       strncpy(temp, dp->name, dp->name_len);
       temp[dp->name_len] = 0;
       //printf("%s  ", temp);

       if (dp->inode == myino){
           strncpy(myname, dp->name, dp->name_len);
           myname[dp->name_len] = 0;
           return 0;
       }
       cp += dp->rec_len;
       dp = (DIR *)cp;
     }
 }
 return(-1);
}


int findino(MINODE *mip, u32 *myino) 
{
  char buf[BLKSIZE], *cp;   
  DIR *dp;

  get_block(mip->dev, mip->inode.i_block[0], buf);
  cp = buf; 
  dp = (DIR *)buf;
  *myino = dp->inode;
  cp += dp->rec_len;
  dp = (DIR *)cp;
  return dp->inode;
}

int tst_bit(char * buf, int bit)
{
    return buf[bit/8] & (1 << (bit%8));
}

void set_bit(char * buf, int bit)
{
    buf[bit/8] |= (1 << (bit%8));
}

void clr_bit(char * buf, int bit)
{
    buf[bit/8] &= ~(1 << (bit%8));
}

int decFreeInodes(int dev)
{
    char buf[BLKSIZE];
    get_block(dev, 1, buf);
    ((SUPER*)buf)->s_free_inodes_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    ((GD*)buf)->bg_free_inodes_count--;
    put_block(dev, 2, buf);
}

int incFreeInodes(int dev)
{
    char buf[BLKSIZE];
    // increment free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    ((SUPER *)buf)->s_free_inodes_count++;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    ((GD *)buf)->bg_free_inodes_count++;
    put_block(dev, 2, buf);
}

int ialloc(int dev)
{
    char buf[BLKSIZE];
    get_block(dev, imap, buf);
    for(int i = 0; i < ninodes; i++)
        if(tst_bit(buf, i) == 0)
        {
            set_bit(buf, i);
            decFreeInodes(dev);
            put_block(dev, imap, buf);
            return i + 1;
        }
    return 0;
}

int balloc(int dev)
{
    char buf[BLKSIZE];
    get_block(dev, bmap, buf);
    for(int i = 0; i < nblocks; i++)
    {
        if(tst_bit(buf, i) == 0)
        {
            set_bit(buf, i);
            decFreeInodes(dev);
            put_block(dev, bmap, buf);
            return i + 1;
        }
    }
        
    return 0;
}

void idalloc(int dev, int ino)
{
    char buf[BLKSIZE];
    get_block(dev, imap, buf);
    if(ino > ninodes)
    {
        printf("ERROR: inumber %d out of range.\n", ino);
        return;
    }
    get_block(dev,imap,buf);
    clr_bit(buf,ino-1);
    put_block(dev,imap,buf);
    incFreeInodes(dev);
}

void bdalloc(int dev, int bno)
{
    char buf[BLKSIZE];
    if(bno > nblocks)
    {
        printf("ERROR: bnumber %d out of range.\n", bno);
        return;
    }
    get_block(dev,bmap,buf);
    clr_bit(buf,bno-1);
    put_block(dev,bmap,buf);
    incFreeInodes(dev);
}

void mytruncate(MINODE * mip)
{
    for(int i = 0; i < 12 && mip->inode.i_block[i]; i++)
        bdalloc(mip->dev, mip->inode.i_block[i]);

    uint32_t buf[256];
    if(mip->inode.i_block[12] != 0)
    {
        get_block(mip->dev, mip->inode.i_block[12], (char*)buf);
        for(int i = 0; i < 256 && buf[i] != 0; i++)
            bdalloc(mip->dev, buf[i]);
    }

    if(mip->inode.i_block[13] != 0)
    {
        get_block(mip->dev, mip->inode.i_block[13], (char*)buf);  
        for(int i = 0; i < 256 && buf[i] != 0; i++)
        {
            uint32_t buf2[256];
            get_block(mip->dev, buf[i], (char*)buf2);
            for(int j = 0; j < 256 && buf2[j] != 0; j++)
            {
                bdalloc(mip->dev, buf2[j]);
            }

            bdalloc(mip->dev, buf[i]);
        }
    }
}

void zero_block(int dev, int blk)
{
    char buf[BLKSIZE];
    memset(buf, 0, BLKSIZE);
    put_block(dev, blk, buf);
}
