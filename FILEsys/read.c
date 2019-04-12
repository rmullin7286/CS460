int myread(int fd, char *buf, int nbytes)
{
    MINODE *mip = running->fd[fd]->minodePtr;
    OFT *op = running->fd[fd];
    int count = 0, avil = mip->inode.i_size - op->offset;
    char *cq = buf;
    uint32_t dbuf[256];
    int blk;

    while(nbytes > 0 && avil > 0)
    {
        int lbk = op->offset / BLKSIZE, startByte = op->offset % BLKSIZE;

        if (lbk < 12)
        {
            blk = mip->inode.i_block[lbk];
        }
        else if(lbk >= 12 && lbk < 268)
        {
            get_block(mip->dev,mip->inode.i_block[12], (char*)dbuf);// Get to blocks from indirect blocks
            blk = dbuf[lbk - 12];
        }
        else
        {
            get_block(mip->dev,mip->inode.i_block[13], (char*)dbuf); // Get to indirect blocks from double indirect
            //cq = dbuf + ((lbk - 268) / 256); // Find which indirect block to go to
            get_block(mip->dev, dbuf[(lbk - 268) / 256], (char*)dbuf); // Get to indirect block
            //cq = dbuf + ((lbk -268) % 256); // Go to direct block from indirect block
            blk = dbuf[(lbk-268) % 256];// (int) *cq? Save direct block value to blk
        }

        // Get data block into readbuf
        char readbuf[BLKSIZE];
        get_block(mip->dev,blk, readbuf);

        // Copy from startByte to buf[], at most remain bytes in the block
        cq = readbuf + startByte;
        int remainingBytes = BLKSIZE - startByte;
        if(nbytes < remainingBytes)
        {
            remainingBytes = nbytes;
        }
        memcpy((buf + count), cq, remainingBytes);
        op->offset += remainingBytes;
        count += remainingBytes;
        avil -= remainingBytes;
        nbytes -= remainingBytes;
        if (nbytes <= 0 || avil <= 0)
            break;
    }
    printf("myread: read %d char from file descriptor %d.\n", count, fd);
    return count;
}

void read_file()
{
    int fd = atoi(pathname), nbytes = atoi(parameter);
    char buf[nbytes + 1];
    if(running->fd[fd] == 0 || (running->fd[fd]->mode != R && running->fd[fd]->mode != RW))
    {
        printf("ERROR: FD %d is not open for read.\n", fd);
        return;
    }
    int ret = myread(fd, buf, nbytes);
    buf[ret] = 0;
    printf("%s\n", buf);
}

void mycat()
{
    int fd = myopen(pathname, R);
    if(fd == -1)
    {
        return;
    }
    char *buf = malloc(sizeof(char) * (running->fd[fd]->minodePtr->inode.i_size + 1));
    myread(fd,buf,running->fd[fd]->minodePtr->inode.i_size);
    buf[running->fd[fd]->minodePtr->inode.i_size] = 0;
    printf("%s\n", buf);
    free(buf);
    close_file(fd);
}
