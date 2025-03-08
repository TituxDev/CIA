/*
	Compilar usando la opcion -lm en gcc ( gcc <fichero de entrada> -o <fichero binario> -lm ).

	Ejecucion: ./<fichero binario> <fichero de topologia> <fichero con lista de perceptrones> <fichero con datos de entrenamiento>

	El programa crea una red neuronal en funcion de los datos de los ficheros de topologia y lista se perceptrones.
	Crea un banco de pruebas enfuncion de la informacion del fichero de entrenamiento.
	Imprime la informacion del banco de pruebas
	Imprime la configuracion actual de la red.
	Entrena a la red en fucion de los dato de entrenamiento e imprime la cantidad de ciclos del dentrenamiento.
	Imprime El estado final de la red.
	Guarda los nuevos pesos en el fichero de la lista de perceptrones.

	Fichero de topologia: Fichero que guarda una lista de numeros enteros.
		El primer numero indica la cantidad de entradas que debe tener la red ( valor minimo 2 )
		El segundo numero es la cantidad de capas en la red ( valor minimo 1 ).
		Los siguientes numeros son la cantidad de perceptrones por cada capa( valor minimo 2 para las capas ocultas y 1 para la capa de salida ).
	Fichero de lista de perceptrones: Fichero que guarda la informacion de cada perceptron de la red.
		El primer numero es un entero positivo que se usa como indice para seleccionar la funcion de activacion.
		El segundo numero el un decimal con el valor del sesgo del perceptron.
		Los siguientes numeros de la fila son los pesos correspodientes a cada entrada del perceptron.
	Fichero de entrenamiento: Se guarda una tabla de con los datos de entrada para pruebas y sus salidas esperadas.
		Cada fila representa una prueba.
		Los primeros numeros de la fila son los indicados como numero de entradas de las pruebas( debe coicidir con el numero de entradas de la red)
		el ultimo numero de cada fila representa la salida esperada para cada prueba.

	Ejepmlo de ficheros:
		topologia: 2 1 1 (esto crea una red con dos entradas con una sola capa de un unico perceptron).
		perceptrones: 0 0 0 0 ( esto crea un perceptron que selecciona la funcion de activacion 0 {x >= 0} y los valores de sesgo y pesos inicializados en 0.
		entrenamiento: 0 0 0	Con esta tabla estrenamos el perceptron para resolver la fncion logica AND.
			       0 1 0
			       1 0 0
			       1 1 1
*/

//BIBLIOTECAS NECESARIAS
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

//DEFINICION DE ESTRUCTURAS.
struct perceptron_t{
	int inputs ;
	int function ;
	float bias ;
	float *weight ;
	float **in ;
	float ( *fun[2] )( float ) ;
	float out ;
} ;
struct net_t{
	int inputs ;
	int layers ;
	float *in ;
	int *topo ;
	struct perceptron_t **neuron ;
} ;
struct test_t{
	int samples ;
	float **in ;
	float *out ;
} ;

//DECLARACION DE FUNCIONES.
//Ponderacion de entradas y pesos.
float weigh( struct perceptron_t ) ;
//Calculo del perceptron.
float evaluate( struct perceptron_t * ) ;
//Calculo de la red
void execute( struct net_t * ) ;
//Entrenamiento de la red
int train( struct net_t * , struct test_t ,  float , float , float ) ;

//	    ( x >= 0 ) | ( 1 )
float boolean( float ) ; float boolean_d( float ) ;
//( 1 / 1 + exp( -x ) )| ( sigmoid * ( 1 - sigmoid ) )
float sigmoid( float ) ; float sigmoid_d( float ) ;

//LISTA DE FUNCIONES DE ACTIVACION
float ( *function[2][2] )( float )={
	[0]= { boolean , boolean_d } ,
	[1]= { sigmoid , sigmoid_d }
} ;

//INICIO DEL PROGRAMA.
int main( int argc , char **argv ){
//	Carga de la red neuronal.
	struct net_t net ;
/*	Lee el fichero asignado en argv[1].
	Asignacion de los valore net_t.inputs ( cantidad de entradas en la red ) y
	net_t.layers ( cantidad de capas en la red ).
	Asigna el espacio de memoria necesario a net_t.topo ( describe la cantidad de perceptrones en capa )
	en funcion del valor de net_t.layers.
	Crea el espacio necesario para cada capa sobre la mariz net_t.neuron siguiendo la descripcion de net_t.topo.
*/	FILE *s ;
	s= fopen( argv[1] , "r" ) ;
	fscanf( s , "%i" , &net.inputs ) ;
	fscanf( s , "%i" , &net.layers ) ;
	net.topo= malloc( net.layers * sizeof( int ) ) ;
	for( int i= 0 ; i < net.layers ; i++ ) fscanf( s , "%i" , &net.topo[i] ) ;
	fclose( s ) ;
	net.in= malloc( net.inputs * sizeof( float ) ) ;
	net.neuron= malloc( net.layers * sizeof( struct perceptron_t * ) ) ;
/*	Carga una estructura perceptron_t en cada espacio de la pamtiz net_t.neuron[capa][perceptron] y
	realiza las conexiones donde los perceptrones de la capa 0 tienen como entrada las entradas asignadas en
	net_t.in y las capas subsecuentes tienen como entradas los perceptrones de la capa anterior.
*/	for( int i= 0 , insize= net.inputs ; i < net.layers ; i++ ) {
		fscanf( s , "%i" , &net.topo[i] ) ;
		net.neuron[i]= malloc( net.topo[i] * sizeof( struct perceptron_t ) ) ;
		for( int j= 0 ; j < net.topo[i] ; j++ ){
			net.neuron[i][j].inputs= insize ;
			net.neuron[i][j].in= malloc( net.neuron[i][j].inputs * sizeof( float * ) ) ;
			for( int k= 0 ; k < net.neuron[i][j].inputs ; k++ ) net.neuron[i][j].in[k]= i ? &net.neuron[i - 1][k].out : &net.in[k] ;
			net.neuron[i][j].weight= malloc( net.neuron[i][j].inputs * sizeof( float ) ) ;
		}
		insize= net.topo[i] ;
	}
/*	Lee el fichero asignado en argv[2].
	Asigna el valor perceptron_t.function ( indice para seleccionar la funcion de activacion y su derivada )
	para cada perceptron de la red.
	Selecciona una pareja de funciones para el arreglo perceptron_t.fun degun el indice de perceptron_t.function.
	Carga los valores perceptron_t.bias ( valor de sesgo de perceptron ) y
	perceptron.weight ( pesos asociados a cada entrada del perceptron ).
*/	s= fopen( argv[2] , "r" ) ;
	for( int i= 0 ; i < net.layers ; i++ ) for( int j= 0 ; j < net.topo[i] ; j++ ){
		fscanf( s , "%i" , &net.neuron[i][j].function ) ;
		net.neuron[i][j].fun[0]= function[net.neuron[i][j].function][0] ;
		net.neuron[i][j].fun[1]= function[net.neuron[i][j].function][1] ;
		fscanf( s , "%f" , &net.neuron[i][j].bias ) ;
		for( int k= 0 ; k < net.neuron[i][j].inputs ; k++ ) fscanf( s , "%f" , &net.neuron[i][j].weight[k] ) ;
	}
	fclose( s ) ;
//	Carga del banco de pruebas.
/*	Asigna el valor de test_t.samples ( cantidad de muestas ) con un valor de potencia 2
	en funcion de la cantidad de entradas de net_t considerando un banco de pruebas binario.
	Define la cantidad de muestras en test_t.in ( valores de entrada para las pruebas) y
	test_t.out ( salidas esperadas para cada prueba.
*/	struct test_t data={ .samples= pow( 2 , net.inputs ) } ;
	data.in= malloc( data.samples * sizeof( float * ) ) ;
	data.out= malloc( data.samples * sizeof( float ) ) ;
/*	Lee el fichero asignado en argv[3].
	Carga los valores de prueba en test_t.in y test_t.out.
*/	s= fopen( argv[3] , "r" ) ;
	for( int i= 0 ; i < data.samples ; i++ ){
		data.in[i]= malloc( net.inputs * sizeof( float ) ) ;
		for( int j= 0 ; j < net.inputs ; j++ ) fscanf( s , "%f" , &data.in[i][j] ) ;
		fscanf( s , "%f" , &data.out[i] ) ;
	}
	fclose( s ) ;
//	Impreion de los datos de prueba.
	printf( "\nMUESTRA:" ) ;
/*	Imprime los valores de test_t.
*/	for( int i= 0 ; i < data.samples ; i++ ){
		putchar( '\n' ) ;
		for( int j= 0 ; j < net.inputs ; j++ ) printf( "[ %.0f ] " , data.in[i][j] ) ;
		printf( "= %.0f" , data.out[i] ) ;
	}
	putchar( '\n' ) ;
//	Impresion de los resultados de la red en su estado inicial.
	printf( "\nORIGINAL" ) ;
/*	Ejecuta el calculo de la red net_t  por cada muestra diponible en test_t e
	imprime los valores de entrada de la red y las salidas de todos sus perceptrones
*/	for( int i= 0  ; i < data.samples ; i++ ){
		putchar( '\n' ) ;
		for( int j= 0 ; j < net.inputs ; j++ ) printf( "[ %.0f ] " , net.in[j]= data.in[i][j] ) ;
		putchar( '\t' ) ;
		execute( &net ) ;
		for( int j= 0 ; j < net.layers ; j++ ){
			printf( "| " );
			for( int k= 0 ; k < net.topo[j] ; k++ ) printf( "%.0f\t" , net.neuron[j][k].out ) ;
		}
	}
//	Entrenamiento de la red e impresion del numero de ciclos en el entrenamiento
/*	Establece los parametros de entrenamiento de la red en funcion de los datos de test_t y
	los valores de proporcion de aprendizaje, tolerancia de error y maxima cantidad de intentos.
*/	printf( "\n\nEPOCAS: %i\n" , train( &net , data , 0.04 , 0.03 , 100000 ) ) ;
//	Ipresion de los resultados de la red despues del entrenamiento.
	printf( "\nRESULTADO" ) ;
/*	Ejecuta el calculo de la red net_t  por cada muestra diponible en test_t e
	imprime los valores de entrada de la red y las salidas de todos sus perceptrones
*/	for( int i= 0 ; i < data.samples ; i++ ){
		putchar( '\n' ) ;
		for( int j= 0 ; j < net.inputs ; j++ ) printf( "[ %.0f ] " , net.in[j]= data.in[i][j] ) ;
		putchar( '\t' ) ;
		execute( &net ) ;
		for( int j= 0 ; j < net.layers ; j++ ){
			printf( "| " ) ;
			for( int k= 0 ; k < net.topo[j] ; k++ ) printf( "%.0f\t" , net.neuron[j][k].out ) ;
		}
	}
//	Guardado de la informacion de la red y liberacion de memoria.
/*	Libera la memoria de tet_t
*/	free( data.out ) ;
	for( int i= 0 ; i < net.inputs ; i++ ) free( data.in[i] ) ;
	free( data.in ) ;
/*	Libera los espacios creados en la estructura net_t y Actualiza la informacion de los 
	ficheros argv[1] y argv[2].
*/	free( net.in ) ;
	s= fopen( argv[2] , "w" ) ;
	for( int i= 0 ; i < net.layers ; i++ ){
		for( int j= 0 ; j < net.topo[i] ; j++ ){
			free( net.neuron[i][j].in ) ;
			fprintf( s , "%i %f" , net.neuron[i][j].function , net.neuron[i][j].bias ) ;
			for( int k= 0 ; k < net.neuron[i][j].inputs ; k++ ) fprintf( s , " %f" , net.neuron[i][j].weight[k] ) ;
			fprintf( s , "\n" ) ;
			free( net.neuron[i][j].weight ) ;
		}
		free( net.neuron[i] ) ;
	}
	free( net.neuron ) ;
	fclose( s ) ;
	s= fopen( argv[1] , "w" ) ;
	fprintf( s , "%i %i" , net.inputs , net.layers ) ;
	for( int i= 0 ; i < net.layers ; i++ ) fprintf( s , " %i" , net.topo[i] ) ;
	free( net.topo ) ;
	fclose( s ) ;
	putchar( '\n' ) ;
	return 0 ;
}
//FIN DEL PROGRAMA

//DEFINICION DE FUNCIONES
int train( struct net_t *net , struct test_t data ,  float learning_rate , float tolerance , float max_attemps ){
	int counter= 0 ;
/*	error en cada prueba.
*/	float error ;
/*	Maximo error de cada ciclo de pruebas.
*/	float errors ;
/*	Indice de la ultima capa de la red.
*/	int ly= net->layers - 1;
	do{
/*		Inicio del ciclo de entrenamiento
*/		errors= 0 ;
		for( int i= 0 ; i < data.samples ; i++ ){
		      /*Carga de entradas y ejecucion de la red.
        	      */for( int j= 0; j < net->inputs ; j++ ) net->in[j]= data.in[i][j] ;
			execute( net ) ;
		      /*Calculo del error y asignacion del error maximo del ciclo.
		      */error= data.out[i] - net->neuron[ly][0].out ;
			errors= fmax( fabs( error ) , errors ) ;
                      /*Calculo del gradiente de la capa de salida.
		      */float delta_o[net->topo[ly]] ;
			for( int j= 0 ; j < net->topo[ly] ; j++ ) delta_o[j]= ( data.out[i] - net->neuron[ly][j].out ) * net->neuron[ly][j].fun[1]( net->neuron[ly][j].out) ;
		      /*Ajuste de pesos y sesgos de la capa de salida.
		      */float delta_h[ly][net->topo[0]] ;
			for( int j= 0 ; j < net->topo[ly] ; j++ ){
				for( int k= 0 ; k < net->neuron[ly][j].inputs ; k++ ) net->neuron[ly][j].weight[k]+= delta_o[j] * ( *net->neuron[ly][j].in[k] ) * learning_rate ;
 				net->neuron[ly][j].bias+= delta_o[j] * learning_rate ;
 			}
		      /*Calculo del gradiente de las capas ocultas y
			ajuste de los pesos y sesgos de las capas ocultas
		      */for( int j= ly - 1 ; j >= 0 ; j-- ) for( int k= 0 ; k < net->topo[j] ; k++ ){
				float back= 0 ;
				for( int l= 0 , f= j + 1 ; l < net->topo[f] ; l++ ) back+= ( j == ly - 1 ? delta_o[l] : delta_h[f][l] ) * net->neuron[f][l].weight[k] ;
				delta_h[j][k]= back * net->neuron[j][k].fun[1]( net->neuron[j][k].out ) ;
				for( int l= 0 ; l < net->neuron[j][k].inputs ; l++ ) net->neuron[j][k].weight[l]+= delta_h[j][k] * ( *net->neuron[j][k].in[l] ) * learning_rate ;
				net->neuron[j][k].bias+= delta_h[j][k] * learning_rate ;
			}
    		}/*
		Fin del ciclo de entrenamiento.*/
/*	Se ejecuta el el entrenamiento hata que el valor sea menor que la tolerancia establecida o
	se alcance el maximo numero de intentos.
*/	} while( errors > tolerance && ++counter < max_attemps ) ;
	return counter ;
}
float weigh( struct perceptron_t perceptron ){
/*	Suma al valor del sesgo del perceptron el resultado de la suma de los productos de
	cada entrada con su respectivo peso y regresa la ponderacion.
*/	float s= perceptron.bias ;
	for( int i= 0 ; i < perceptron.inputs ; i++ ) s+= *perceptron.in[i] * perceptron.weight[i] ;
	return s ;
}
float evaluate( struct perceptron_t *perceptron ){
/*	Usa la ponderacion del perceptro como parametro de la funcion de activacion asignada en
	function[perceptron.function][0] y asignando el resultado a perceptron.out.
*/	return perceptron->out= perceptron->fun[0]( weigh( *perceptron ) ) ;
}
void execute( struct net_t *net  ){
/*	Ejecuta el calculo de cada perceptron en la res.
*/	for( int i= 0 ; i < net->layers ; i++ ) for( int j= 0 ; j < net->topo[i] ; j++ ) evaluate( &net->neuron[i][j] ) ;
}

//Funciones de activacion.
float boolean( float sum ){
	return sum >= 0 ;
}
float boolean_d( float sum){
	return 1 ;
}
float sigmoid( float sum ){
	return 1 / ( 1 + exp( -sum ) ) ;
}
float sigmoid_d( float sig ){
	return sig * ( 1 - sig ) ;
}
