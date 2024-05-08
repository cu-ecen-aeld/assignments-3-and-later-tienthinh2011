#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

int main (int argc, char* argv[])
{
    openlog(NULL, 0, LOG_USER);
    if (argc < 3) {
        printf("Error: Number of argument should be 3 (including the cmd):\nUsage: ./writer /tmp/aesd/assignment1/sample.txt ios\n");
        syslog(LOG_ERR, "Number of argument should be 3 (including the cmd)");
        return 1;
    } else {
        printf("Prepare to write to %s text:'%s'\n", argv[1], argv[2]);
    }
    FILE *fd;
    fd = fopen(argv[1], "w");
    if (fd == NULL) {
        printf("Error: Cannot open / create file\n");
        syslog(LOG_ERR, "Cannot open / create file");
        return 1;
    } else {
        int len = strlen(argv[2]);
        int ret = fprintf(fd, "%s", argv[2]);
        syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
        if (ret != len) {
            printf("Number of bytes written is %d, which is different from %d:\n", ret, len);
            syslog(LOG_ERR, "Number of bytes written is %d, which is different from %d:\n", ret, len);
        } else {
            printf("Succesful writing %s to %s\n", argv[1], argv[2]);
        }
    }

}