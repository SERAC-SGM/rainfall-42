#include <stdlib.h>
#include <unistd.h>

void n(void)
{
  system("/bin/cat /home/user/level7/.pass");
  return;
}

void m(void)
{
  puts("Nope");
  return;
}

void main(int arc, char **argv)
{
  char *dest;
  char **ppcVar1;
  
  dest = (char *)malloc(64);
  ppcVar1 = (code **)malloc(4);
  *ppcVar1 = m;
  strcpy(dest,*(char **)(param_2 + 4));
  (**ppcVar1)();
  return;
}
