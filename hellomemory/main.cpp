#include <stdlib.h>
#include "Alloctor.h"



int main(){

    char* data = new char[128];
    delete[] data;

    char* data1 = new char;
    delete data1;

    char* data2 = new char[64];
    delete[] data2;

    system("pause");

    return 0;
}