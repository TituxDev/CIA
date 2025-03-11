/*
	Compilación:
		Usar la opción -lm en gcc:
		gcc <fichero_de_entrada> -o <fichero_binario> -lm

	Ejecución:
		./<fichero_binario> <nombre_de_red> <fichero_de_entrenamiento> <modo_de_impresión>

	Descripción:
		Este programa crea y entrena una red neuronal en función de los datos proporcionados en los archivos de topología y lista de perceptrones.
		Utiliza un conjunto de datos de entrenamiento para ajustar los pesos y sesgos de la red.
		Después del entrenamiento, imprime información de la red y guarda los nuevos pesos en el archivo de perceptrones.

	Archivos de entrada:
		1. Fichero de topología:
			- Contiene una lista de números enteros.
			- El primer número indica la cantidad de entradas de la red (mínimo 2).
			- El segundo número representa la cantidad de capas ocultas (mínimo 1).
			- Los siguientes números indican la cantidad de perceptrones por capa (mínimo 2 en ocultas, mínimo 1 en la capa de salida).

		2. Fichero de lista de perceptrones:
			- Contiene la información de cada perceptrón de la red.
			- Cada línea tiene:
				- Un entero que selecciona la función de activación.
				- Un número decimal que representa el sesgo del perceptrón.
				- Los siguientes números son los pesos correspondientes a cada entrada.

		3. Fichero de entrenamiento:
			- Contiene una tabla con datos de entrada y sus salidas esperadas.
			- Cada fila representa un conjunto de pruebas.
			- Los primeros valores corresponden a las entradas de la red.
			- El último valor de cada fila representa la salida esperada.

		4. Modo de impresión:
			- Un número entero que determina el formato de impresión de los resultados.

	Ejemplo de archivos:
		Archivo de topología (nombre.topology):
			2 1 1  -> Red con 2 entradas, 1 capa oculta, 1 perceptrón en la capa de salida.

		Archivo de perceptrones (nombre.neurons):
			0 0 0 0  -> Perceptrón con función de activación 0, sesgo 0 y pesos inicializados en 0.

		Archivo de entrenamiento (entrenamiento.txt):
			0 0 0  -> Entrenamiento para la función lógica AND.
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
#include <string.h>
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
void load_net( struct net_t ** , char * ) ;
void load_test_data( char * , struct test_t ** , int , int ) ;
void close_net( struct net_t * , char * ) ;
void free_test( struct test_t * , int ) ;
void execute( struct net_t * ) ;
int train( struct net_t * , struct test_t ,  float , float , float ) ;
void print_net( struct net_t * , struct test_t , int ) ;

int main( int argc , char **argv ){
	struct test_t *data ;
	struct net_t *net ;
	load_net( &net , argv[1] ) ;
	load_test_data( argv[2] , &data , net->inputs , 4 ) ;
	printf( "\nMUESTRA:" ) ;
	for( int i= 0 ; i < data->samples ; i++ ){
		putchar( '\n' ) ;
		for( int j= 0 ; j < net->inputs ; j++ ) printf( "[ %.0f ] " , data->in[i][j] ) ;
		printf( "= %.0f" , data->out[i] ) ;
	}
	putchar( '\n' ) ;
	printf( "\nORIGINAL" ) ;
	print_net( net , *data , argv[3][0] - '0' ) ;
	printf( "\n\nEPOCAS: %i\n" , train( net , *data , 0.04 , 0.03 , 100000 ) ) ;
	printf( "\nRESULTADO" ) ;
	print_net( net , *data , argv[3][0] - '0' ) ;
	close_net( net , argv[1] ) ;
	free_test( data , net->inputs ) ;
	putchar( '\n' ) ;
	return 0 ;
}

void load_topo( char * , struct net_t **) ;
void load_neurons( char * , struct net_t **) ;
void load_net( struct net_t **net , char *file ){
	int l= strlen( file ) ;
	char name[l+10] ;
	strcpy( name , file ) ;
	strcat( name , ".topology" ) ;
	load_topo( name , net ) ;
	strcpy( &name[l] , ".neurons" ) ;
	load_neurons( name , net ) ;
}
void save_net( struct net_t * , char * );
void free_net( struct net_t * ) ;
void close_net( struct net_t *net , char *name ){
	save_net( net , name ) ;
	free_net( net ) ;
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
void free_test( struct test_t *data , int inputs ){
	for( int i= 0 ; i < inputs ; i++ ) free( data->in[i] ) ;
	free( data->in ) ;
	free( data->out ) ;
	free( data ) ;
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
float bk_error( struct perceptron_t * , float * , int , int ) ;
float derivate( struct perceptron_t * ) ;
void adjust( struct perceptron_t * , float * , float , int ) ;
int train( struct net_t *net , struct test_t data ,  float learning_rate , float tolerance , float max_attemps ){
	int counter= 0 ;
	float error ;
	int ly= net->layers - 1;
	do{
		error= 0 ;
		for( int i= 0 ; i < data.samples ; i++ ){
			for( int j= 0; j < net->inputs ; j++ ) net->in[j]= data.in[i][j] ;
			execute( net ) ;
			for( int j= 0 ; j < net->topo[ly] ; j++ ) error= fmax(fabs( data.out[i] - net->neuron[ly][j].out ) , error ) ;
			if ( error < tolerance ) continue ;
			float delta_[net->topo[0]] ;
			float back ;
			for( int j= ly ; j >= 0 ; j-- ){
				for( int k= 0 ; k < net->topo[j + ( j < ly )] ; k++ ) delta_[k]= derivate( ( struct perceptron_t * )&net->neuron[j][k] ) * ( j == ly ? data.out[i] - net->neuron[j][k].out : back );
				adjust( ( struct perceptron_t * )net->neuron[j] , delta_ , learning_rate ,net->topo[j] ) ;
				back= j ? bk_error( ( struct perceptron_t * )net->neuron[j] , delta_ , net->topo[j - 1] , net->topo[j] ) : 0 ;
			}
		}
	} while( error > tolerance && ++counter < max_attemps ) ;
	return counter ;
}

float weigh( struct perceptron_t perceptron ){
	float s= perceptron.bias ;
	for( int i= 0 ; i < perceptron.inputs ; i++ ) s+= *perceptron.in[i] * perceptron.weight[i] ;
	return s ;
}
float evaluate( struct perceptron_t *perceptron ){
	return perceptron->out= perceptron->fun[0]( weigh( *perceptron ) ) ;
}
void execute( struct net_t *net  ){
	for( int i= 0 ; i < net->layers ; i++ ) for( int j= 0 ; j < net->topo[i] ; j++ ) evaluate( &net->neuron[i][j] ) ;
}
float bk_error( struct perceptron_t *layer , float *delta_ , int prev_layer , int neurons ){
	float b= 0 ;
	for( int i= 0 ; i < prev_layer ; i ++ ) for( int j= 0 ; j < neurons ; j++ ) b+= delta_[i] * layer[j].weight[i] ;
	return b ;
}
float derivate( struct perceptron_t *neuron ){
	return neuron->fun[1]( neuron->out ) ;
}
void adjust( struct perceptron_t *neuron , float *delta_ , float learning_rate , int neurons ){
	for( int k= 0 ; k < neurons ; k++ ){
		for( int l= 0 ; l < neuron[k].inputs ; l++ ) neuron[k].weight[l]+= delta_[k] * *neuron[k].in[l] * learning_rate ;
		neuron[k].bias+= delta_[k] * learning_rate ;
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
void save_net( struct net_t *net , char *file ){
	int l= strlen( file ) ;
	char name[l+10] ;
	strcpy( name , file ) ;
	strcat( name , ".topology" ) ;
	save_topo( name , net ) ;
	strcpy( &name[l] , ".neurons" ) ;
	save_neurons( name , net ) ;
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
