This binary will open the password of the next level and save it as a stream to local_14. It then waits for 1 argument. The variable local_98 is used to store the content of the password file using fread. local_98 is null-terminated at the index atoi(av[1]). After that, we compare local_98 and av[1] using strcmp. If this comparison is equal, a shell is launched.

So we simply need the comparison between local_98 and av[1] to be equal. Since local_98 is null-terminated at the index provided by av[1], passing an empty string will render local_98 empty (null-terminated at index 0). The comparison should be equal :

    $ ./bonus3 ""
    $ cd ../end
    $ cat .pass
    3321b6f81659f9a71c76616f606e4b50189cecfea611393d5d649f75e157353c
