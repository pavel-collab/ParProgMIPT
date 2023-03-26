#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>

int main(int argc, char* argv[]) {
    
    std::string s = "abcde";

    int fd2 = open("compare.txt", O_RDONLY, 0644);

    if (fd2 < 0) {
        perror("[-] Failed for open file for read.");
        return -1;
    }

    char* buf = (char*) calloc(strlen(s.c_str()), sizeof(char));
    if (read(fd2, buf, strlen(s.c_str())*sizeof(char)) < 0) {
        perror("[-1] Failed read from the file.");
        close(fd2);
        return -1;
    }

    if (strcmp(buf, s.c_str()) == 0) {
        printf("[+] Same strings!\n");
    }
    else {
        printf("[!] Strings are different\n");
        printf("String 1 is %s\n", buf);
        printf("String 2 is %s\n", s.c_str());
    }

    if (close(fd2) < 0) {
        perror("[-] Failed close file.");
        return -1;
    }

    return 0;
}