undefined4 main(int ac, char **av)

{
  int uVar1;
  int iVar2;
  char *local_98 [16];
  FILE *local_14;
  
  local_14 = fopen("/home/user/end/.pass","r");
  if ((local_14 == (FILE *)0x0) || (ac != 2)) {
    uVar1 = 0xffffffff;
  }
  else {
    fread(local_98,1,0x42,local_14);
    local_57 = 0;
    local_98[atoi(av[1])] = 0;
    fclose(local_14);
    iVar2 = strcmp((char *)local_98,*(char **)(av[1]));
    if (iVar2 == 0) {
      execl("/bin/sh","sh",0);
    }
    else {
      puts(local_14);
    }
    uVar1 = 0;
  }
  return uVar1;
}
