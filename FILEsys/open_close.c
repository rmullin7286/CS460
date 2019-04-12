#include <time.h>
#include <stdint.h>

enum FILEMODE
{
    R,
    W,
    RW,
    APPEND
};

int open_file()
{
	int mode;
	if(strcmp(parameter, "r") == 0)
		mode = R;
	else if(strcmp(parameter, "w") == 0)
		mode = W;
	else if(strcmp(parameter, "rw") == 0)
		mode = RW;
	else if(strcmp(parameter, "append") == 0)
		mode = APPEND;

    int fd = myopen(pathname, mode);
    if(fd >= 0 && fd < 10)
        printf("Opened file descriptor at %d\n", fd);
}

int myopen(char * name, int mode)
{
    int ino = getino(name);

    if(mode < 0 || mode > 3)
    {
        printf("Invalid mode\n");
        return -1;
    }
    if(!ino)
    {
        printf("File does not exist\n");
        return -1;
    }
    
    MINODE * mip = iget(dev, ino);
    if(!S_ISREG(mip->inode.i_mode))
    {
        printf("Not a regular file\n");
        return -1;
    }

    for(int i = 0; i < NOFT; i++)
        if(oft[i].minodePtr == mip && oft[i].mode != R)
        {
            printf("File already open\n");
            return -1;
        }

    for(int i = 0; i < NOFT; i++)
        if(oft[i].refCount == 0)
        {
            oft[i] = (OFT){.mode = mode, .refCount = 1, .minodePtr = mip, .offset = (mode == APPEND ? mip->inode.i_size : 0)};
            if(mode == W)
            {
                mytruncate(mip);
                mip->inode.i_size = 0;
            }
            for(int j = 0; j < NFD; j++)
                if(!running->fd[j])
                {
                    running->fd[j] = (oft + i);
                    break;
                }
            time_t current_time = time(0L);
            mip->inode.i_atime = current_time;
            if(mode != R)
                mip->inode.i_mtime = current_time;
            mip->dirty = 1;
            return i;
        }
    return -1;
};

void close_file(int fd)
{
    OFT *op = running->fd[fd];
    op->refCount--;
    if(op->refCount == 0)
        iput(op->minodePtr);

    oft[fd].minodePtr = NULL;
    running->fd[fd] = NULL;
}

void myclose()
{
	close_file(atoi(pathname));
}

void lseek_file()
{
    int fd, position;
    printf("Enter fd and position: ");
    scanf("%d %d", &fd, &position);
}

void mylseek(int fd, int position)
{
    int original = running->fd[fd]->offset;
    running->fd[fd]->offset = (position <= running->fd[fd]->minodePtr->inode.i_size && position >= 0) ? position : original;
}

int pfd()
{
    printf("fd  mode  offset  INODE\n");
    printf("--- ----  ------  -----\n");
    for(int i = 0; i < NOFT; i++)
        if(oft[i].refCount)
        {
            char * mode;
            switch(oft[i].mode)
            {
                case R: mode = "R";
                    break;
                case W: mode = "W";
                    break;
                case RW: mode = "RW";
                    break;
                case APPEND: mode = "A";
            }
            printf("%d  %s  %d  %d\n", i, mode, oft[i].offset, oft[i].minodePtr->ino);
        }
}

int dup(int fd)
{
    if(oft[fd].refCount == 0) return -1;
    for(int i = 0; i < NOFT; i++)
    {
        if(oft[i].refCount == 0)
        {
            oft[i] = oft[fd];
            oft[i].refCount = 1;
            return i;
        }
    }

    return -1;
}

int dup2(int fd, int gd)
{
    if(oft[fd].refCount == 0) return -1;
    if(oft[gd].refCount) close_file(gd);
    oft[gd] = oft[fd];
    oft[gd].refCount = 1;
}

