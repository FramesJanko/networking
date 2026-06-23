#include <stdio.h>
#include <stdint.h>

int main() {
    printf("Size of uint64_t: %zu bytes\n", sizeof(uint64_t));
    printf("Size of unsigned long: %zu bytes\n", sizeof(unsigned long));
    printf("Size of unsigned long long: %zu bytes\n", sizeof(unsigned long long));

    if (sizeof(uint64_t) == sizeof(unsigned long)) {
        printf("uint64_t is likely typedef'd as unsigned long\n");
    } else if (sizeof(uint64_t) == sizeof(unsigned long long)) {
        printf("uint64_t is likely typedef'd as unsigned long long\n");
    }

    return 0;
}
