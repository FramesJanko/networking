#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char *bytelist;
} Data;

int main(){
    char myByteList[2] = {0, 0};
    Data my_data = {&myByteList[0]};
    myByteList[0] = 97;

    printf("First byte in my_data is %x\n", my_data.bytelist[0]);
}
