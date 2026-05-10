/* Sasmithan Satkunarajah 12403246
        Je déclare quil sagit de mon propre travail.
        Ce travail a été réalisé intégralement par un être humain. */

#include <stdio.h>
#include <unistd.h>
#include "buffer.h"
#include <stdlib.h>



int main(void){

    char c[512];
    buffer * b = buff_create(0,16);
    if (b == NULL){
        return 1;

    }
    while(buff_fgets(b,c,sizeof(c))!=NULL){
        printf("on a lu : %s ", c);
    }
    printf("fin ");
    buff_free(b);
    return 0;

}