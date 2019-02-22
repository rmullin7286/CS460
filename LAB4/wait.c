int tswitch();

int ksleep(int event)
{
  int sr = int_off();
  //printf("proc %d going to sleep on event=%d\n", running->pid, event);

  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  //printList("sleepList", sleepList);
  tswitch();
  int_on(sr);
}

int kwakeup(int event)
{
  PROC *temp, *p;
  temp = 0;
  int sr = int_off();
  
  //printList("sleepList", sleepList);

  while (p = dequeue(&sleepList)){
     if (p->event == event){
	//printf("wakeup %d\n", p->pid);
	p->status = READY;
	enqueue(&readyQueue, p);
     }
     else{
	enqueue(&temp, p);
     }
  }
  sleepList = temp;
  //printList("sleepList", sleepList);
  int_on(sr);
}
void cleanOrphanedChildren(PROC * p)
{
	PROC * p1, * child;
	p1 = proc + 1;
	child = p->child;

	while(child)
	{
		child->parent = p1;
		child->ppid = 1;
		p->child = child->sibling;
		child->sibling = p1->child;
		p1->child = child;
		child = p->child;
	}
}


int kexit(int exitValue)
{
  printf("proc %d in kexit(), value=%d\n", running->pid, exitValue);
  running->exitCode = exitValue;
  running->status = ZOMBIE;
  cleanOrphanedChildren(running);
  tswitch();
}



void removeProc(PROC * p)
{
	PROC *prev;

	if(p->parent->child == p)
	{
		p->parent->child = p->sibling;
	}
	else
	{
		for(prev = p->parent->child; prev->sibling != p; prev = prev->sibling);
		prev->sibling = p->sibling;
	}
	p->parent = p->sibling = p->child = p->ppid = 0;
}


int kwait(int * status)
{
  PROC * p;

  if(!running->child)
    return -1;

  while(1)
  {
    for(p = running->child; p; p = p->sibling)
      if(p->status == ZOMBIE)
        break;
    if(p)
    {
      *status = p->exitCode;
      enqueue(freeList, p);
	  removeProc(p);
      return p->pid;
    }
    ksleep(running);
  }
}
