void write_file()
{
    int fd = atoi(pathname);
    if(running->fd[fd]->refCount == 0)
    {
        printf("File descriptor not open\n");
        return;
    }
    if(running->fd[fd]->mode == R)
    {
        printf("File is open for read\n");
        return;
    }

    mywrite(fd, parameter, strlen(parameter));
}

int mywrite(int fd, char buf[], int nbytes)
{
    int original_nbytes = nbytes;
    OFT * oftp = (running->fd[fd]);
    MINODE * mip = oftp->minodePtr;
    char * cq = buf;
    while(nbytes > 0)
    {
        int lbk = oftp->offset / BLKSIZE, startByte = oftp->offset % BLKSIZE, blk;

        if(lbk < 12)
        {
            if(mip->inode.i_block[lbk] == 0)
                mip->inode.i_block[lbk] = balloc(mip->dev);
            blk = mip->inode.i_block[lbk];
        }
        else if(lbk >= 12 && lbk < 256 + 12)
        {
            if(mip->inode.i_block[12] == 0)
            {
                mip->inode.i_block[12] = balloc(mip->dev);
                zero_block(mip->dev, mip->inode.i_block[12]);
            }
            uint32_t ibuf[256];
            get_block(mip->dev, mip->inode.i_block[12], (char*)ibuf);
            blk = ibuf[lbk - 12];
            if(blk == 0)
            {
                if((blk = ibuf[lbk - 12] = balloc(mip->dev)) == 0)
                {
                    printf("Ran out of disk space!!!!!!!!\n");
                    return original_nbytes - nbytes;
                }
                put_block(mip->dev, mip->inode.i_block[12], (char*)ibuf);
            }
        }
        else
        {
            int indirect1 = (lbk - 256 - 12) / 256;
            int indirect2 = (lbk - 256 - 12) % 256;
            uint32_t ibuf[256];
            if(mip->inode.i_block[13] == 0)
            {
                mip->inode.i_block[13] = balloc(mip->dev);
                zero_block(mip->inode.i_block[13], mip->inode.i_block[13]);
            }
            get_block(mip->dev, mip->inode.i_block[13], (char*)ibuf);
            if(ibuf[indirect1] == 0)
            {
                ibuf[indirect1] = balloc(mip->dev);
                zero_block(mip->dev, ibuf[indirect1]);
                put_block(mip->dev, mip->inode.i_block[13], (char*)ibuf);
            }
            uint32_t ibuf2[256];
            get_block(mip->dev, ibuf[indirect1], (char*)ibuf2);
            if(ibuf2[indirect2] == 0)
            {
                ibuf2[indirect2] = balloc(mip->dev);
                zero_block(mip->dev, ibuf2[indirect2]);
                put_block(mip->dev, ibuf[indirect1], (char*)ibuf2);
            }
            blk = ibuf2[indirect2];
        }

        char wbuf[BLKSIZE];
        zero_block(mip->dev, blk);
        get_block(mip->dev, blk, wbuf);
        char * cp = wbuf + startByte;
        int remain = BLKSIZE - startByte;
        if(nbytes < remain)
            remain = nbytes;
        memcpy(cp, cq, remain);
        cq += remain;
        oftp->offset += remain;
        nbytes -= remain;
        mip->inode.i_size += remain;
        put_block(mip->dev, blk, wbuf);
        // Unoptimized
        // while(remain > 0)
        // {
        //     *cp++ = *cq++;
        //     nbytes--;
        //     remain--;
        //     oftp->offset++;
        //     if(oftp->offset > mip->inode.i_size)
        //         mip->inode.i_size++;
        //     if(nbytes <= 0)
        //         break;
        // }
    }

    mip->dirty = 1;
    printf("wrote %d char into the file descriptor fd=%d\n", original_nbytes, fd);
    return original_nbytes;

    //TODO: OPTMIZE
}
