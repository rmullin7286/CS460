#pragma once
//kc has a bunch of missing declarations in his code that causes implicit declarations.
//This is why you use header files!!!!


void lock(void);
void unlock(void);
void int_on(int);
int int_off(void);
int kgets(char s[]);
void kputc(char c);
