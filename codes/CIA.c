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
		topologia: 		2 1 1 (esto crea una red con dos entradas con una sola capa de un unico perceptron).
		perceptrones: 	0 0 0 0 ( esto crea un perceptron que selecciona la funcion de activacion 0 {x >= 0} y los valores de sesgo y pesos inicializados en 0.
		entrenamiento: 	0 0 0	Con esta tabla estrenamos el perceptron para resolver la fncion logica AND.
			    	   	0 1 0
			    		1 0 0
			       		1 1 1
*/
float boolean( float ) ; float boolean_d( float ) ; // 		      ( x >= 0 ) | ( 1 )
float sigmoid( float ) ; float sigmoid_d( float ) ; // 	( 1 / ( 1 + exp( - x ) ) | sigmoid( x ) * ( 1 - sigmoid( x ) )
float ( *function[2][2] )( float )={
	[0]= { boolean , boolean_d } ,
	[1]= { sigmoid , sigmoid_d }
} ;
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
void execute( struct net_t * ) ;
int train( struct net_t * , struct test_t ,  float , float , float ) ;
void print_net( struct net_t * , struct test_t , int ) ;
void load_topo( char * , struct net_t ** ) ;
void load_neurons( char * , struct net_t ** ) ;
void load_test_data( char * , struct test_t ** , int , int ) ;
void save_neurons( char * , struct net_t * ) ;
void save_topo( char * , struct net_t * ) ;
void free_test( struct test_t * , int );
void free_net( struct net_t * ) ;
float evaluate( struct perceptron_t * ) ;
float weigh( struct perceptron_t ) ;
void normalize( struct perceptron_t * , float , float ) ;
int main( int argc , char **argv ){
	struct net_t *net ;
	load_topo( argv[1] , &net ) ;
	load_neurons( argv[2] , &net ) ;
	struct test_t *data ;
	load_test_data( argv[3] , &data , net->inputs , 4 ) ;
	printf( "\nMUESTRA:" ) ;
	for( int i= 0 ; i < data->samples ; i++ ){
		putchar( '\n' ) ;
		for( int j= 0 ; j < net->inputs ; j++ ) printf( "[ %.0f ] " , data->in[i][j] ) ;
		printf( "= %.0f" , data->out[i] ) ;
	}
	putchar( '\n' ) ;
	printf( "\nORIGINAL" ) ;
	print_net( net , *data , argv[4][0] - '0' ) ;
	printf( "\n\nEPOCAS: %i\n" , train( net , *data , 0.04 , 0.03 , 100000 ) ) ;
	printf( "\nRESULTADO" ) ;
	print_net( net , *data , argv[4][0] - '0' ) ;
	save_neurons( argv[2] , net ) ;
	save_topo( argv[1] , net ) ;
	free_test( data , net->inputs ) ;
	free_net( net ) ;
	putchar( '\n' ) ;
	return 0 ;
}
void execute( struct net_t *net  ){
	for( int i= 0 ; i < net->layers ; i++ ) for( int j= 0 ; j < net->topo[i] ; j++ ) evaluate( &net->neuron[i][j] ) ;
}
int train( struct net_t *net , struct test_t data ,  float learning_rate , float tolerance , float max_attemps ){
	int counter= 0 ;
	float error ;
	float errors ;
	int ly= net->layers - 1;
	do{
		errors= 0 ;
		for( int i= 0 ; i < data.samples ; i++ ){
		    for( int j= 0; j < net->inputs ; j++ ) net->in[j]= data.in[i][j] ;
			execute( net ) ;
		    error= data.out[i] - net->neuron[ly][0].out ;
			errors= fmax( fabs( error ) , errors ) ;
            float delta_o[net->topo[ly]] ;
			for( int j= 0 ; j < net->topo[ly] ; j++ ) delta_o[j]= ( data.out[i] - net->neuron[ly][j].out ) * net->neuron[ly][j].fun[1]( net->neuron[ly][j].out) ;
		    float delta_h[ly][net->topo[0]] ;
			for( int j= 0 ; j < net->topo[ly] ; j++ ){
				for( int k= 0 ; k < net->neuron[ly][j].inputs ; k++ ) net->neuron[ly][j].weight[k]+= delta_o[j] * ( *net->neuron[ly][j].in[k] ) * learning_rate ;
 				net->neuron[ly][j].bias+= delta_o[j] * learning_rate ;
 			}
		    for( int j= ly - 1 ; j >= 0 ; j-- ) for( int k= 0 ; k < net->topo[j] ; k++ ){
				float back= 0 ;
				for( int l= 0 , f= j + 1 ; l < net->topo[f] ; l++ ) back+= ( j == ly - 1 ? delta_o[l] : delta_h[f][l] ) * net->neuron[f][l].weight[k] ;
				delta_h[j][k]= back * net->neuron[j][k].fun[1]( net->neuron[j][k].out ) ;
				for( int l= 0 ; l < net->neuron[j][k].inputs ; l++ ) net->neuron[j][k].weight[l]+= delta_h[j][k] * ( *net->neuron[j][k].in[l] ) * learning_rate ;
				net->neuron[j][k].bias+= delta_h[j][k] * learning_rate ;
			}
			for( int j= 0 ; j < net->layers ; j++ ) for( int k= 0 ; k < net->topo[j] ; k++ ) normalize( &net->neuron[j][k] , 1 , -1 ) ;
    	}
	} while( errors > tolerance && ++counter < max_attemps ) ;
	return counter ;
}
void print_net( struct net_t *net , struct test_t data , int decimals ){
	decimals%= 10 ;
	decimals+= '0' ;
	char format_in[]={ '[' , ' ' , '%' , '.' , decimals , 'f' , ' ' , ']' , ' ' , 0 } ;
	char format_out[]={ '%' , '.' , decimals , 'f' , '\t' , 0 } ;
	for( int i= 0  ; i < data.samples ; i++ ){
		putchar( '\n' ) ;
		for( int j= 0 ; j < net->inputs ; j++ ) printf( format_in , net->in[j]= data.in[i][j] ) ;
		putchar( '\t' ) ;
		execute( net ) ;
		for( int j= 0 ; j < net->layers ; j++ ){
			printf( "| " );
			for( int k= 0 ; k < net->topo[j] ; k++ ) printf( format_out , net->neuron[j][k].out ) ;
		}
	}
}
void load_topo( char *file , struct net_t **net  ){
	*net= malloc( sizeof( struct net_t ) ) ;
	FILE *s ;
	s= fopen( file , "r" ) ;
	fscanf( s , "%i" , &( *net )->inputs ) ;
	fscanf( s , "%i" , &( *net )->layers ) ;
	( *net )->topo= malloc( ( *net )->layers * sizeof( int ) ) ;
	for( int i= 0 ; i < ( *net )->layers ; i++ ) fscanf( s , "%i" , &( *net )->topo[i] ) ;
	fclose( s ) ;
	( *net )->in= malloc( ( *net )->inputs * sizeof( float ) ) ;
	( *net )->neuron= malloc( ( *net )->layers * sizeof( struct perceptron_t * ) ) ;
	for( int i= 0 , insize= ( *net )->inputs ; i < ( *net )->layers ; i++ ) {
		fscanf( s , "%i" , &( *net )->topo[i] ) ;
		( *net )->neuron[i]= malloc( ( *net )->topo[i] * sizeof( struct perceptron_t ) ) ;
		for( int j= 0 ; j < ( *net )->topo[i] ; j++ ){
			( *net )->neuron[i][j].inputs= insize ;
			( *net )->neuron[i][j].in= malloc( ( *net )->neuron[i][j].inputs * sizeof( float * ) ) ;
			for( int k= 0 ; k < ( *net )->neuron[i][j].inputs ; k++ ) ( *net )->neuron[i][j].in[k]= i ? &( *net )->neuron[i - 1][k].out : &( *net )->in[k] ;
			( *net )->neuron[i][j].weight= malloc( ( *net )->neuron[i][j].inputs * sizeof( float ) ) ;
		}
		insize= ( *net )->topo[i] ;
	}
}
void load_neurons( char *file , struct net_t ** net ){
	FILE *s ;
	s= fopen( file , "r" ) ;
	for( int i= 0 ; i < ( *net )->layers ; i++ ) for( int j= 0 ; j < ( *net )->topo[i] ; j++ ){
		fscanf( s , "%i" , &( *net )->neuron[i][j].function ) ;
		( *net )->neuron[i][j].fun[0]= function[( *net )->neuron[i][j].function][0] ;
		( *net )->neuron[i][j].fun[1]= function[( *net )->neuron[i][j].function][1] ;
		fscanf( s , "%f" , &( *net )->neuron[i][j].bias ) ;
		for( int k= 0 ; k < ( *net )->neuron[i][j].inputs ; k++ ) fscanf( s , "%f" , &( *net )->neuron[i][j].weight[k] ) ;
	}
	fclose( s ) ;	
}
void load_test_data( char *file , struct test_t **data , int inputs , int samples){
	*data= malloc( sizeof( struct test_t ) ) ;
	FILE *f ;
	( *data )->samples= samples ;
	( *data )->in= malloc( ( *data )->samples * sizeof( float * ) ) ;
	( *data )->out= malloc( ( *data )->samples * sizeof( float ) ) ;
	f= fopen( file , "r" ) ;
	for( int i= 0 ; i < ( *data )->samples ; i++ ){
		( *data )->in[i]= malloc( inputs * sizeof( float ) ) ;
		for( int j= 0 ; j < inputs ; j++ ) fscanf( f , "%f" , &( *data )->in[i][j] ) ;
		fscanf( f , "%f" , &( *data )->out[i] ) ;
	}
	fclose( f ) ;
}
void save_neurons( char *file , struct net_t *net ){
	FILE *f ;
	f= fopen( file , "w" ) ;
	for( int i= 0 ; i < net->layers ; i++ ){
		for( int j= 0 ; j < net->topo[i] ; j++ ){
			fprintf( f , "%i %f" , net->neuron[i][j].function , net->neuron[i][j].bias ) ;
			for( int k= 0 ; k < net->neuron[i][j].inputs ; k++ ) fprintf( f , " %f" , net->neuron[i][j].weight[k] ) ;
			fprintf( f , "\n" ) ;
		}
	}
	fclose( f ) ;
}
void save_topo( char *file , struct net_t *net ){
	FILE *f ;
	f= fopen( file , "w" ) ;
	fprintf( f , "%i %i" , net->inputs , net->layers ) ;
	for( int i= 0 ; i < net->layers ; i++ ) fprintf( f , " %i" , net->topo[i] ) ;
	fclose( f ) ;
}
void free_test( struct test_t *data , int inputs ){
	for( int i= 0 ; i < inputs ; i++ ) free( data->in[i] ) ;
	free( data->in ) ;
	free( data->out ) ;
	free( data ) ;
}
void free_net( struct net_t *net ){
	free( net->in ) ;
	for( int i= 0 ; i < net->layers ; i++ ){
		for( int j= 0 ; j < net->topo[i] ; j++ ){
			free( net->neuron[i][j].in ) ;
			free( net->neuron[i][j].weight ) ;
		}
		free( net->neuron[i] ) ;
	}
	free( net->neuron ) ;
	free( net->topo ) ;
}
float weigh( struct perceptron_t perceptron ){
	float s= perceptron.bias ;
	for( int i= 0 ; i < perceptron.inputs ; i++ ) s+= *perceptron.in[i] * perceptron.weight[i] ;
	return s ;
}
float evaluate( struct perceptron_t *perceptron ){
	return perceptron->out= perceptron->fun[0]( weigh( *perceptron ) ) ;
}
void normalize( struct perceptron_t *neuron , float max , float min ){
float avrg= 0 , delta= max - min ;
	if( neuron->inputs ) return ;
	for( int i= 0 ; i < neuron->inputs ; i++ ) avrg+= fabs( neuron->weight[i] ) ;
	avrg/= neuron->inputs ;
	avrg+= !avrg ;
	for( int i= 0 ; i < neuron->inputs ; i++ ){
		neuron->weight[i]/= avrg ;
		neuron->weight[i]*= delta ;
		neuron->weight[i]+= min ;
		neuron->bias*= delta ;
		neuron->bias+= min ;
	}
}
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
