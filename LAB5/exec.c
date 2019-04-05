int exec(char * cmdline)
{
	int i, upa, usp;
	char * cp, kline[128], file[32], filename[32];
	PROC * p = running;
	strcpy(kline, cmdline);
	cp = kline;
	i = 0;
	while(*cp != ' ')
	{
		filename[i] = *cp;
		i++;
		cp++;
	}
	filename[i] = 0;
	file[0] = 0;
	upa = (int*)(p->pgdir[2048] & 0xFFFF0000);
	if(!load(filename, p))
		return -1;
	usp = upa + 0x100000 - 128;
	strcpy((char *)usp, kline);
	p->usp = (int*)VA(0x100000 - 128);
	for(i = 2; i < 14; i++)
		p->kstack[SSIZE - i] = 0;
	p->kstack[SSIZE - 1] = (int)VA(0);
	return (int)p->usp;
}
