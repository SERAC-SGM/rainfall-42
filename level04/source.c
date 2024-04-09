#include <stdio.h>
#include <stdlib.h>

void n(void)
{
  char buffer [520];
  fgets(buffer, 512, stdin);
  p(buffer);
  if (m == 0x1025544) { //16930116 in decimal
    system("/bin/cat /home/user/level5/.pass");
  }
  return;
}

void main(void)
{
  n();
  return;
}
