#include <stdlib.h>
#include <stdio.h>

void flip_bits(char input){
    printf("Showing bits for %c\n", input);
    printf("%d\n", input);
    for (int i = 7; i >= 0; i--) {
        if(1 << i & (unsigned char)input){
            printf("1 ");
        } else {
            printf("0 ");
        }
        if (i == 0)
            printf("\n");
    }
    unsigned char return_value = (unsigned char)input;
    for (int i = 7; i>= 0; i--) {
        return_value = return_value ^ 1 << i;
        printf("%c ", return_value & 1 << i ? '1' : '0');
        if (i == 0)
            printf("\n");
    }
}

int main(){
    long numConnections = 0;
    char input[10];
    while(1){
        
        // printf("Add connection y/n?\n");
        fgets(input, sizeof(input), stdin);
        char decision = input[0];
        printf("%c\n", decision);
        flip_bits(decision);
        // if (decision == 'y'){
        //     numConnections+=1;
        // }
        // printf("%li\n", numConnections);
        // printf("Showing connections as a bit in a long number:\n");
    }
}
