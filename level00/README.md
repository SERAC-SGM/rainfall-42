# Level0

level0@RainFall:~$ ls -l
total 732
-rwsr-x---+ 1 level1 users 747441 Mar  6  2016 level0

Using gdb, we can see that the first argument is being compared with 0x1a7 :

	[...]
	0x08048ed9 <+25>:	cmp    $0x1a7,%eax
	[...]

0x1a7 is 423 in base 10.

	level0@RainFall:~$ ./level0 423
	$ id
	uid=2030(level1) gid=2020(level0) groups=2030(level1),100(users),2020(level0)
	$ cd ../level1	
	$ cat .pass
	1fe8a524fa4bec01ca4ea2a869af2a02260d4a7d5fe7e7c24d8617e6dca12d3a

