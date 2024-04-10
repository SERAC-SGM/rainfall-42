int main(void) {
    char inputBuffer[128]; 
    char *inputLine;
    char commandArg[125];

    while (1) {
        // Print addresses of `auth` and `service`
        printf("%p, %p \n", auth, service);

        inputLine = fgets(inputBuffer, 128, stdin);
        if (inputLine == NULL) {
            return 0;
        }

        // Process "auth " command
        if (strncmp(inputBuffer, "auth ", 5) == 0) {
            // Allocate memory for `auth` and initialize it
            auth = malloc(4);
            *auth = 0;
            // If the command argument is of a certain length, copy it into `auth`
            if (strlen(commandArg) < 0x1e) {
                strcpy((char *)auth, commandArg);
            }
        }

        // Process "reset" command
        if (strncmp(inputBuffer, "reset", 5) == 0) {
            free(auth);
        }

        // Process "service" command
        if (strncmp(inputBuffer, "service", 7) == 0) {
            service = strdup(commandArg);
        }

        // Process "login" command
        if (strncmp(inputBuffer, "login", 5) == 0) {
            // If `auth` is not properly set, print "Password:\n", else give shell access
            if (auth != NULL && auth[8] == 0) {
                fwrite("Password:\n", 1, 10, stdout);
            } else {
                system("/bin/sh");
            }
        }
    }
    return 0;
}
