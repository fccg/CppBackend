#include <stdlib.h>
#include "Alloctor.h"



int main(){

    char* data = new char[128];
    delete[] data;

    char* data1 = new char;
    delete data1;


    char* data2 = (char*)malloc(64);
    free(data2);

    return 0;
}