//----- (080484B4) --------------------------------------------------------
char *p(char *dest, char *s)
{
  char buf[4104]; // [esp+10h] [ebp-1008h] BYREF

  puts(s);
  read(0, buf, 4096);
  *strchr(buf, '\n') = 0;
  return strncpy(dest, buf, 20);
}

//----- (0804851E) --------------------------------------------------------
char *pp(char *dest)
{
  char src[20]; // [esp+28h] [ebp-30h] BYREF
  char v3[28]; // [esp+3Ch] [ebp-1Ch] BYREF

  p(src, " - ");
  p(v3, " - ");
  strcpy(dest, src);
  dest[strlen(dest)] = " ";
  return strcat(dest, v3);
}

//----- (080485A4) --------------------------------------------------------
int  main(int argc, const char **argv, const char **envp)
{
  char s[42]; // [esp+16h] [ebp-2Ah] BYREF

  pp(s);
  puts(s);
  return 0;
}
