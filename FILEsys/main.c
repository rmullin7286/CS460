/****************************************************************************
*                   KCW testing ext2 file system                            *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

#include "type.h"


MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;
OFT    oft[NOFT];

char gpath[256];
char *name[64]; // assume at most 64 components in pathnames
int  n;

int  fd, dev;
int  nblocks, ninodes, bmap, imap, inode_start;
char pathname[256], parameter[256];

#include "util.c"
MINODE *iget();


/*
#include "alloc_dealloc.c"
#include "cd_ls_pwd.c"
#include "mkdir.c"
#include "creat.c"
#include "rmdir.c"
#include "link.c"
#include "unlink.c"
*/

#include "open_close.c"
#include "read.c"
#include "write.c"
#include "cp_mv.c"


int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = 0;
    p->cwd = 0;
    p->status = FREE;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
  }
  for (i=0; i<NOFT; i++){
    oft[i].refCount = 0;
    oft[i].mode = -1;
  }
}

// load root INODE and set root pointer to it
int mount_root()
{  
  printf("mount_root()\n");
  root = iget(dev, 2);
}

char *disk = "mydisk";
int main(int argc, char *argv[ ])
{
  int ino;
  char buf[BLKSIZE];
  char line[256], cmd[64], path[128];

  if (argc > 1)
    disk = argv[1];

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);  exit(1);
  }
  dev = fd;

  /********** read super block at 1024 ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system *****************/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  inode_start = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

  init();  
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->status = READY;
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  //printf("hit a key to continue : "); getchar();
  while(1){
    printf("input command: [ls|cd|pwd|mkdir|creat|rmdir|link|symlink|unlink\n");
    printf("               |open|close|lseek|read|write|cat|cp|mv|quit] : ");
    fgets(line, 128, stdin);
    line[strlen(line) - 1] = 0;

    if (line[0]==0)
      continue;
    pathname[0] = 0;
    parameter[0] = 0;
    memset(parameter, 0, 256);
 
    sscanf(line, "%s %s %64c", cmd, pathname, parameter);
    printf("cmd=%s path=%s param=%s\n", cmd, pathname, parameter);

    if (strcmp(cmd, "ls")==0)
       list_file();
    if (strcmp(cmd, "cd")==0)
       change_dir();

    if (strcmp(cmd, "pwd")==0)
       pwd(running->cwd);

    if (strcmp(cmd, "mkdir")==0)
       make_dir();

    if (strcmp(cmd, "creat")==0)
       creat_file();

    if (strcmp(cmd, "rmdir")==0)
      rmdir();

    if (strcmp(cmd, "link")==0)
      link();

    if (strcmp(cmd, "unlink")==0)
      unlink();

    if (strcmp(cmd, "symlink")==0)
      symlink();

    if (strcmp(cmd, "readlink")==0){
      readlink(line);
      printf("symlink name = %s\n", line);
    }

    if (strcmp(cmd, "open")==0){
      open_file();
    }

    if (strcmp(cmd, "close")==0){
	  printf("CLOSING FILE");
      myclose();
    }

    if (strcmp(cmd, "lseek")==0){
      lseek_file();
    }
    if (strcmp(cmd, "pfd")==0){
      pfd();
    }

    if (strcmp(cmd, "read")==0){
      read_file();
    }

    if (strcmp(cmd, "write")==0){
      write_file();
    }

    if (strcmp(cmd, "cat")==0){
      mycat();
    }
    if (strcmp(cmd, "cp")==0){
      cp();
    }


    if (strcmp(cmd, "mv")==0){
      mymov();
    }

    if (strcmp(cmd, "quit")==0)
       quit();
  }
}
 
int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0) {
	  mip->refCount = 1;
      iput(mip);
	}
  }
  exit(0);
}
