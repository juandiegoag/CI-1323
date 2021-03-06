#include <mpi.h>
#include <math.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

void blanquear(int a[], int size){ //recibe un array, y lo llena con 0
  for (int j = 0; j < size; j++) {
    a[j]=0;
	}
}

int main (int argc,  char *argv[] ){
  int id, numproc, *matriz, *rbuf, *aparicionesFila, *totalColumnas, *conteoFilas, *sumaColumnas, *aparicionesColumnas, *conteoColumnas;

  MPI_Init( &argc , &argv );
  MPI_Comm_size( MPI_COMM_WORLD , &numproc );
  MPI_Comm_rank( MPI_COMM_WORLD , &id );

  int a        = atoi(argv[1]);
  int filas    = a * numproc;
  int columnas = atoi(argv[2]);
  int cantidad = a * columnas;

  if( id == 0 ){
    matriz              = ( int * ) malloc ( filas * columnas * sizeof( int ) );
    aparicionesFila     = new int[5*filas]; //matriz final de apariciones en las filas
    totalColumnas       = new int[columnas];
    aparicionesColumnas = new int[5*columnas];

    for( int i = 0; i < filas; i++ ){
      for( int j = 0; j < columnas; j++ ){
         *(matriz + i*columnas + j) = rand() % 5;
         cout<<*(matriz + i*columnas + j)<<" ";
      }
      cout<<endl;
    }

    blanquear(totalColumnas,columnas);
  }
  // Repartición de la matriz entre los procesos pertenecientes al MPI_COMM_WORLD
  // a cada proceso se le repartirá a filas de c enteros ==> a * C
  rbuf = new int[cantidad];
  MPI_Scatter(matriz, cantidad, MPI_INT, rbuf, cantidad, MPI_INT ,0 ,MPI_COMM_WORLD);

  conteoFilas    = new int[5*a]; //arreglo que contiene el numero de apariciones por procesos
  //a cada proceso le corresponde un arreglo.
  conteoColumnas = new int[5*columnas];//contiene las apariciones de los numeros en las columnas.
  sumaColumnas   = new int[columnas];//contiene la suma de todas las columnas, correspondientes proceso,
  //es de tamano del numero de elementos en una fila
  int offset    = 0;//se usa para llevar la cuenta de las apariciones de un numero en una fila, cuando se termina de leer una fila, y se pasa a la otra el offset se incrementa
  int offsetCol = 0;

  blanquear(conteoFilas   , 5*a       );//llena los vectores con cero.
  blanquear(conteoColumnas, 5*columnas);
  blanquear(sumaColumnas  , columnas  );

  for (int i = 0; i < cantidad; i++) {
    //PARTE 1: Suma de columnas.
    sumaColumnas[i%columnas]+=rbuf[i];//va sumando los valores de las columnas, en el vector sumaColumnas

    //PARTE 2: Conteo de apariciones en filas.
    if (i%columnas == 0 && i>0) {
      offset+=5;//actualiza el offset si ya se leyo una fila totalmente
    }
    *(conteoFilas + offset + rbuf[i]) += 1; //conteo de apariciones en filas

    //PARTE 3: Conteo de apariciones en columnas.
    offsetCol = i%columnas;                            //actualiza el offset del conteo de columnas
    *(conteoColumnas + (offsetCol*5) + rbuf[i]) += 1; //cuenta las apariciones de numeros en las columnas
  }

  MPI_Gather(conteoFilas, 5*a, MPI_INT, aparicionesFila, 5*a, MPI_INT, 0, MPI_COMM_WORLD);

  MPI_Reduce(sumaColumnas  , totalColumnas      , columnas  , MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(conteoColumnas, aparicionesColumnas, 5*columnas, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  MPI_Barrier(MPI_COMM_WORLD); //espera a que todos lleguen a este punto

  //liberar memoria
  delete conteoFilas;
  delete conteoColumnas;
  delete rbuf;
  delete sumaColumnas;

  if (id == 0) {
    free(matriz);
    int numFila=0, numColumna = 0;

    cout << "PROCESO 0:" <<endl;
    cout<<endl;

    for (int i = 0; i < 5*filas; i++) {
      int posicion=i%5;
      if(posicion==0){
        numFila++;
        cout<<"[FILA "<<numFila<<"]: "<<endl;
      }
      cout<<"\t"<<"Numero de "<<posicion<<": "<<aparicionesFila[i]<<endl;
    }

    cout<<endl;

    for (int i = 0; i < columnas; i++) {
      cout<<"La suma de elementos de la columna "<<i<<" es:"<<endl;
      cout<<"\t"<<totalColumnas[i]<<endl;
    }

    cout<<endl;

    for (int i = 0; i < 5*columnas; i++) {
      int posicion=i%5;
      if(posicion==0){
        numColumna++;
        cout<<"[COLUMNA "<<numColumna<<"]: "<<endl;
      }
      cout<<"\t"<<"Numero de "<<posicion<<": "<<aparicionesColumnas[i]<<endl;
    }
    delete aparicionesColumnas;
    delete aparicionesFila;
    delete totalColumnas;
  }

  MPI_Finalize();
  return 0;
}
