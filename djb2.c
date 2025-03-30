#include <stdio.h>
#include <stdlib.h>

unsigned long djb2(char *input) {
    unsigned long hash = 5381;
    int c;

    while (c = *input++)
        hash = ((hash << 5) + hash) + c;

    return hash;
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        printf("Error: only one argument is expected.\n");
    printf("DJB2 Hash calculating ...\n");

    unsigned long result = djb2(argv[0]);
    
    printf("Hash value: %lu\n", result);
}