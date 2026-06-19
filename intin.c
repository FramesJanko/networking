#include <stdio.h>
#include <stdlib.h>

int intin(int num, int* list, size_t list_size){
    for (size_t i = 0; i < list_size; i++){
        if (num == list[i])
            return 1;
    }
    return 0;
}

int main(){
    int int_list[] = {0, 5, 10};
    printf("%d\n", intin(2, int_list, sizeof(int_list)));
    printf("%d\n", intin(5, int_list, sizeof(int_list)));
}
