#include <stdio.h>
#include <unistd.h>
#include "buffer.h"
#include <stdlib.h>



int main(void){

    char c[10];
    buffer * b = buff_create(0,64);
    if (b == NULL){
        return 1;

    }
    while(buff_fgets(b,c,sizeof(c))!=NULL){
        printf("on a lu : %s ", c);
    }
    printf("fin EOF");
    buff_free(b);
    return 0;

}