#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main(){
    char * data_to_send;
    char test[4] = "test";
    data_to_send = test;
    char * new_data = (char*)malloc(sizeof(char) * (strlen(data_to_send) + 1));
    for (int i = 0; i < strlen(data_to_send); i++) {
        new_data[i] = data_to_send[i];
    }
    new_data[strlen(data_to_send)] = '\n';
    new_data[strlen(data_to_send) + 1] = '\0';
    printf("%s\n", data_to_send);
    printf("%zu\n", strlen(data_to_send));
    printf("%s\n", new_data);
    printf("%zu\n", strlen(new_data));
}
