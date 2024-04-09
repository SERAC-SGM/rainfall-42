#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* p(void)
{
    char inputBuffer[76];
    fflush(stdout);
    gets(inputBuffer); 
    if (((unsigned int)inputBuffer & 0xb0000000) == 0xb0000000) {
        printf("(%p)\n", (void*)inputBuffer);
        exit(1);
    }
    puts(inputBuffer);
    strdup(inputBuffer);
    return;
}

void main() {
    p();
    return;
}
