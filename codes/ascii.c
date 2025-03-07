#include <stdio.h>

int main( void ){
    signed char i= 0 ;
    while( ++i > 0 ) printf( "[ %c ] - [ %i ]\n" , i , i ) ;
    return 0 ;
}
/*
El programa aprovecha el tamano de la variable tipo char el cual es de 8 bits
por lo que el maximo valor que puede representar es 1 1 1 1  1 1 1 1 = 255,
si se suma 1 a ese valor la memoria pasara a su valor 0 0 0 0  0 0 0 0.
Al declarar la variable como signed el maximo valor que puede representar es
0 1 1 1  1 1 1 1 = 127 ya que el bit mas significativo se usa para indicar si
el valor es positivo( 0 ) o negativo( 1 ), cuando se incrementa en 1 el valor
el nuevo estado de los bits sera 1 0 0 0  0 0 0 0 = -127 que es menor que 0,
por lo que se cumple la condicion de salida del bucle while.
*/