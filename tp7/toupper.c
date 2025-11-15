#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define HOST_NAME_LEN 128

int main() {
    char buffer[BUFFER_SIZE];
    char host[HOST_NAME_LEN];
    
    gethostname(host, HOST_NAME_LEN);

    while(fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        sleep(3);
        
        // fgets() always add '\0' in the end of buffer, so we can just check where it is
        // this way we don't need to see all the BUFFER_SIZE and can always end correctly
        for(size_t i = 0; buffer[i] != '\0'; i++) {
            buffer[i] = toupper(buffer[i]);
        }

        printf("%s[%d]: %s", host, getpid(), buffer);
        fflush(stdout);
    }
}
