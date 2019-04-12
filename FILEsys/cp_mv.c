void cp()
{
    int fdsource = open_file(pathname, R);
    char temp[strlen(parameter) + 1];
    strcpy(temp, parameter);
    int fddest = open_file(parameter, W);
    char * buffer = malloc(sizeof(char) * running->fd[fdsource]->minodePtr->inode.i_size + 1);
    myread(fdsource, buffer, running->fd[fdsource]->minodePtr->inode.i_size);
    buffer[running->fd[fdsource]->minodePtr->inode.i_size] = 0;
    mywrite(fddest, buffer, running->fd[fdsource]->minodePtr->inode.i_size);
    free(buffer);
    close_file(fdsource);
    close_file(fddest);
}

void mymov()
{
    char * path1temp = strdup(pathname);
    link();
    strcpy(pathname, path1temp);
    unlink();
    free(path1temp);
}
