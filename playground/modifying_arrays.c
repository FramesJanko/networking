#include <stdio.h>
#include <stdlib.h>

void update_array(int* array, int num_to_add, int* count){
    array[*count] = num_to_add;
    *count = *count+1;
}

int main(){
    int connections[10] = {0};
    int connectionsCount = 0;
    for (int i = 0; i < sizeof(connections)/sizeof(int); i++) {
        update_array(connections, i, &connectionsCount);
    }
    for(int i = 0; i < sizeof(connections)/sizeof(int); i++){
        printf("%d ", connections[i]);
    }
}
