//---------------------------LIBRERIAS------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>// permite usar fork, pipes entre otros
#include <math.h>
#include <stdbool.h>
#include <pthread.h> //Recordar que al compilar: gcc archivo.c -o como_le_pngas -pthread

//--------------------------ESTRUCTURAS------------------------------------

//visbilidad que se guardara en el buffer y luego se pasara  las hebras 
struct visibilidadParcial{
    //double u;
    //double v;
	double real;
	double imaginaria;
	double potencia;
	double total;
    double N;// se inicializa en 1
};

//Estructura que guara la visibilidad creada inicialmente luego de leer el archivo
struct visibilidad{
    double u;
    double v;
	double real;
	double imaginaria;
	//double potencia;
	double ruido;
};

//Estructura que alberga los monitores que se utilizarana  lso largo del codigo
struct monitor{
    struct visibilidad* Buffer;
    int idM;
    int lleno, vacio;
    pthread_mutex_t mut;
    pthread_cond_t notfull, notEmpty;
};



//--------------------------VARIABLES GLOBALES------------------------------------

int eof=0;
int buffer;
struct visibilidadParcial* listaFinal;
int final;
int cantDiscos;
int casoFinal=0;
struct monitor* listaMonitores;

//Funcion que innicializa la lista  monitores y permite su uso de estos
//Entrada: nada
//Salidad; un cambionen la variable global lista monitores
void Monitores(){
    listaMonitores=(struct monitor*)malloc((sizeof(struct monitor)*cantDiscos));
    for(int x=0; x<cantDiscos;x++){
        pthread_mutex_init(&listaMonitores[x].mut,NULL);
        pthread_cond_init(&listaMonitores[x].notfull,NULL);
        pthread_cond_init(&listaMonitores[x].notEmpty,NULL);
        listaMonitores[x].idM=x;
        listaMonitores[x].lleno=-1;//inicialmente vacio (-1), lleno(1)
        listaMonitores[x].Buffer=(struct visibilidad*)malloc((sizeof(struct visibilidad)*buffer));
    }
}


//Funcionamiento: Funcion que obtiene los parametros de entrada necesarios para el funciojnamiento del programa
//Entrada: parametros ingresados por el usuario siguiendo el sgiguiente patron:
//-i: nombre de las visibilidades
//-o: nombre archivo Salida
//-n: cantidad de disco
//-d: anchho de cada disco
//-s: tamaño buffer de cada monitor
//-d: bandera para mostrar por pantalla las visibilidades
//Salida: No posee retorno, ya que es una función void, pero entrega por puntero los parametros de char*, char* y 4 enteros.
void recibirArgumentos(int argc, char *argv[],char **nombreVisibilidades, char **nombreSalida,int *cantDiscos,int *anchodisco, int *tamBuffer, int *bandera){
	int opt;
    int flags = 0;
	char *aux2;
	char *aux3;
    char *aux4;
	aux2 = malloc(10*sizeof(char));
	aux3 = malloc(10*sizeof(char)); 
    aux4 = malloc(10*sizeof(char));


	if(argc <9){//si se ingresa un numero de argumentos menor a 9, se finaliza la ejecucion del programa
		printf("Se ingreso un numero incorrecto de argumentos\n");
		fprintf(stderr, "Uso correcto: %s [-i nombre de la imagen de entrada] [-o nombre archivo Salida] [-n cantidad de disco] [-d anchho de cada disco] [-s tamaño buffer de cada monitor] [-d bandera para mostrar por pantalla las visibilidades]\n",
				   argv[0]);
		   exit(EXIT_FAILURE);
		}
	int C = -1;
	int A = -1;
    int T = -1;
	int B = -1;
	while((opt = getopt(argc, argv, "i:o:d:n:s:b")) != -1) 
    { 
	   //opt recibe el argumento leido (se hace de forma secuencial) y se ingresa a un switch
	   //En caso de que opt sea -1, se dejaran de buscar entradas y por lo tanto se podrá salir del while
	   switch(opt) 
       {    
            case 'i':
              	(*nombreVisibilidades)=optarg; 

            break;
             case 'o':
              	(*nombreSalida)=optarg; 

            break;
            
	 		case 'n': 
                C = strtol(optarg, &aux2, 10);
                if(optarg!=0 && C==0){
                    fprintf(stderr,  "Uso correcto: %s [-i nombre de la imagen de entrada] [-o nombre archivo Salida] [-n cantidad de disco] [-d anchho de cada disco] [-s tamaño buffer de cada monitor] [-b bandera para mostrar por pantalla las visibilidades]\n",
                        argv[0]); 
                        exit(EXIT_FAILURE);
                }
         
            break;

            case 'd': 
                A = strtol(optarg, &aux3, 10);
                if(optarg!=0 && A==0){
                    fprintf(stderr,"Uso correcto: %s [-i nombre de la imagen de entrada] [-o nombre archivo Salida] [-n cantidad de disco] [-d anchho de cada disco] [-s tamaño buffer de cada monitor] [-b bandera para mostrar por pantalla las visibilidades]\n",
                        argv[0]); 
                        exit(EXIT_FAILURE);
                }               
            break;

            case 's': 
                T = strtol(optarg, &aux4, 10);
                if(optarg!=0 && T==0){
                    fprintf(stderr,"Uso correcto: %s [-i nombre de la imagen de entrada] [-o nombre archivo Salida] [-n cantidad de disco] [-d anchho de cada disco] [-s tamaño buffer de cada monitor] [-b bandera para mostrar por pantalla las visibilidades]\n",
                        argv[0]); 
                        exit(EXIT_FAILURE);
                }               
            break;

            case 'b': 
                flags = 1;              
            break;

            default:
                printf("opcions default\n");
                    fprintf(stderr, "Uso correcto: %s [-i nombre de la imagen de entrada] [-o nombre archivo Salida] [-n cantidad de disco] [-d anchho de cada disco] [-s tamaño buffer de cada monitor] [-b bandera para mostrar por pantalla las visibilidades]\n",
                        argv[0]); 
                exit(EXIT_FAILURE);
        }
	}
    if(C<0){
		fprintf(stderr, "Uso correcto: %s [-i nombre de la imagen de entrada] [-o nombre archivo Salida] [-n cantidad de disco] [-d anchho de cada disco] [-s tamaño buffer de cada monitor] [-b bandera para mostrar por pantalla las visibilidades]\n",
				   argv[0]); 
		exit(EXIT_FAILURE);
		}

    if(flags==1)
    {
		(*bandera) = 1;
    }	
	(*cantDiscos) = C; 
	(*anchodisco) = A;		
    (*tamBuffer) = T; 
	
}

//Funcion que crea un archivo de salida que reune los resultados de los resultados el hebras
//Entrada: nombre crea el archivo de salida basado en el nomrbe ingresa inicialmente
//Salida: archivo que contiene todos los resultados de las hebras y los histogramas
void archivoSalida(char* nombreSalida) {
    FILE* salida;
    salida=fopen(nombreSalida,"w");
    salida=fopen(nombreSalida,"a+");
    int contador=0;
    // Se ve ek tamaño necesario para utilizar el proceso comi char
	
    while (contador<cantDiscos)
    {   
        int disco=contador;
        double real=listaFinal[contador].real;
        double imaginario=listaFinal[contador].imaginaria;
        double potencia=listaFinal[contador].potencia;
        double total=listaFinal[contador].total;

        fprintf(salida,"Disco %d: \nMedia real: %f \nMedia imaginaria: %f \nPotencia: %f \nRuido real: %f \n",disco+1,real,imaginario,potencia,total);
        contador=contador+1;
    }
    
    fclose(salida);
}

//Funcion que mustra por pantalla las hebras y su visibilidades
//Entrada: lista de las hebras y las visibilidades que procesaron
//Salida: por pantalla
void salidaBandera(){
    int contador=0;
    while (contador<cantDiscos){
        printf("Soy la hebra %d, procesé %lf visibilidades\n",contador,listaFinal[contador].N);
        contador=contador+1;
    }
}

//funcion que entrega el numero de lineas totales del archivo
//Entrada: Archivo a leer
//Salidad: entero que contiene el numero de lineas totales del archivo
int maximochar(char* nombreArchivo){
    char caracter;
    //Se declara un archivo
    FILE *archivo;
    //Se declar auna variable caracter que contendra los caracteres que ingresaran a la lista
    int max=0;
      archivo = fopen(nombreArchivo,"r");
       while((caracter = fgetc(archivo)) != EOF)   
        {
            if (caracter=='\n'){
            max=max+1;
            }
        }
        fclose(archivo);
    return max;
}


//funcion, entrega la visibilidad que corresponde a la linea ingresada, por ende entrega 1, en caso e haber leido todo el documento, se entrega una es
//estructura llena de -1
//Entrada: nombre el archivo y la linea obtetivo de la cual se queire obtener información
//Salida: estructura de visibilidad
struct visibilidad visibilidadLeida(char* nombreArchivo, int lineasActual){
   //Se declara un archivo
    FILE *archivo;
    //Se declar auna variable caracter que contendra los caracteres que ingresaran a la lista
    char caracter;
    //Se inicializa una variable para darle contar las lineas
    int lineas=1;
    //Se inicializa una estructura que contendra las visibilidades encontradas por linea
    struct visibilidad visActual;
    //Se inicalizan als variables para en caso de haber llegado al final de archivo la hebra principla tenga una forma de detectarlo
    visActual.u=-1;
    visActual.v=-1;
    visActual.imaginaria=-1;
    visActual.real=-1;
    visActual.ruido=-1;
    //Se inicializar el espacio para los caracteres
    int i=1;
    //Se iniciliza la lista que contendra los caracteres
    char *listaCaracteres=(char*)malloc(sizeof(char)*i);
    //Se iniciliza una double para guardar lso numeros encontrados para poder transofmralos a double
    double numeroActual;
    //Se van contando las columnas llamadas aqui elementos
    int elemento=0;
    //Se abre el archivo en modo lectura
    archivo = fopen(nombreArchivo,"r");
    
    //Se comprueba que el archivo no este vario o corrupto de se asi se le informa al usuario
    if (archivo == NULL)
    {
        //Se le informa al usuario el fallo y se aplica el mismo resultado que si se llegara al final del archivo
      //  printf("\nEl archivo %s no pudo ser leido. \n\n",nombreArchivo);
        //Se retorna un valor diferente al del finl del archivo para informar que hubo un fallo en la lectura del archivo
        visActual.u=-2;
        visActual.v=-2;
        visActual.imaginaria=-2;
        visActual.real=-2;
        visActual.ruido=-2;
    }
    //Si esta correcto se sigue con esta parte
    else{
        //Se entra en un ciclo que no se detiene hasta encontrar el End Of File, es decir el final del archivo
        while((caracter = fgetc(archivo)) != EOF)   
        {
            //Si el caracter es un salto de linea o se no encuentra en el quinto elemento se entra aqui
            if (caracter=='\n'&&(lineas!=lineasActual))
            {
                //Se auemnta la cantiad de lineas leidas
                lineas=lineas+1;
            }
            //se commparan las lienas pediddad con la actual
            if(lineas==lineasActual){
                // se buscar el caracter que indica el cambio de columno o elemento
                if(caracter==','){
                    switch (elemento)
                    {
                    //caso u
                    case 0:
                        //se transforma a double el char*
                        numeroActual=atof(listaCaracteres);
                        //se guarda en la casilla pedida
                        visActual.u=numeroActual;
                        // se libera memoria para poder "reiniciar la lista"
                        free(listaCaracteres);
                        //Se pide memoria nuevamente
                        i=1;
                        listaCaracteres=(char*)malloc(sizeof(char)*i);
                        break;
                    //casi v    
                    case 1:
                        //se transforma a double el char*
                        numeroActual=atof(listaCaracteres);
                         //se guarda en la casilla pedida
                        visActual.v=numeroActual;
                        // se libera memoria para poder "reiniciar la lista"
                        free(listaCaracteres);
                         //Se pide memoria nuevamente
                        i=1;
                        listaCaracteres=(char*)malloc(sizeof(char)*i);
                        break;
                    //caso real    
                    case 2:
                        //se transforma a double el char*
                        numeroActual=atof(listaCaracteres);
                         //se guarda en la casilla pedida
                        visActual.real=numeroActual;
                        // se libera memoria para poder "reiniciar la lista"
                        free(listaCaracteres);
                         //Se pide memoria nuevamente
                        i=1;
                        listaCaracteres=(char*)malloc(sizeof(char)*i);
                        break;
                    //caso imaginaria    
                    case 3:
                        //se transforma a double el char*
                        numeroActual=atof(listaCaracteres);
                         //se guarda en la casilla pedida
                        visActual.imaginaria=numeroActual;
                        // se libera memoria para poder "reiniciar la lista"
                        free(listaCaracteres);
                         //Se pide memoria nuevamente
                        i=1;
                        listaCaracteres=(char*)malloc(sizeof(char)*i);
                        break;      
                    //caso ruido
                    case 4:
                        //se transforma a double el char*
                        numeroActual=atof(listaCaracteres);
                        //se guarda en la casilla pedida
                        visActual.ruido=numeroActual;
                        // se libera memoria para poder "reiniciar la lista"
                        free(listaCaracteres);
                         //Se pide memoria nuevamente
                        i=1;
                        listaCaracteres=(char*)malloc(sizeof(char)*i);
                        break;

                    default:
                        break;
                    }
                    //Se aumenta la colna que se esta leyendo
                    elemento=elemento+1;
                } else
                {
                    listaCaracteres[i-1]=caracter;
                    //Se aumenta el contador
                    i++;
                    //Se cambia el tamaño
                    listaCaracteres=realloc(listaCaracteres,sizeof(char)*i);                
                }
            }
	            
        }
        if((getc(archivo)) != EOF){eof=1;printf("si lo hizo");exit(1);}
           
    }
    //Se cierra el archivo
    fclose(archivo);
    //Se retorna la estructura solicitada
   return visActual;
}

//Funcion que devuelve el radio interno y externo de cada disco
//Entrada: el ancho , la cantidad de discos , y el arreglo de resultados
//Salida: se actualiza el contenido del arreglo radios
void calculosRadiosDisco(int ancho,int cantDiscos,double radios[cantDiscos][2]){
	double aumento = cantDiscos - 1;
	double radioInterno=0;
	double radioExterno=0;
	for(int i=0;i<aumento;i++){
		radioInterno = radioExterno;
		radioExterno = radioExterno + ancho;
		radios[i][0] = radioInterno;
		radios[i][1] = radioExterno;
	}
	radios[cantDiscos-1][0]=radioExterno;
	radios[cantDiscos-1][1]=-1;//indicará valor infinito
}

//Funcion que permite saber a que disco pertenece cada uno de las visibilidades
//Entrada: radio interno, radio externo  el u y el v
//Salida: booleano 
bool pertenenciaDisco(int radioInterno,int radioExterno,double u, double v){
	double distancia = sqrt(((u*u)+(v*v)));
   // printf("distancia: %f\n", distancia);
	if(distancia>radioInterno){
		if(distancia<=radioExterno){
           // printf("1");
			return true;
		}
        else if(radioExterno==-1){
          //  printf("2");
            return true;    
        }
       // printf("3");
        return false;
        
	}
	else{
       // printf("4");
		return false;
        
	}
}

//Funcionq ue devuelve el radio al que pertenece una vizibilidad en especifico
//Entrada : la cantidad de discos, la lista con los radios interno y externos, el u y v de cada visbilidad
//Salida: entero que indica a que readio pertenece 
int radioActual(int numDiscos, double radios[numDiscos][2] , double u, double v){
    int disco=0;
	for(int i=0;i<numDiscos;i++){
		if(pertenenciaDisco(radios[i][0],radios[i][1],u,v)){
            //printf("%d",i+1);
			return disco=i;
		}
	}
    
}

//Funcion que calcula los calculos fianles pedidos 
//Entrada; la cantidad de disco, y los valoes que entregara
//Salida: antualización de las 
void calculos(int disco , double valores[6]){
    double mediaReal=0;
    double mediaImaginaria=0;
    double potencia=0;
    double ruido=0;
    double N=0;
    int termino=0;
    int largoBuffer=buffer;
    for(int i=0;i<largoBuffer;i++){
        if(listaMonitores[disco].Buffer[i].ruido==-2){
            termino=1;
      //      printf("ruido=-2--------------------------------\n");
        }
        else{
            mediaReal = mediaReal + listaMonitores[disco].Buffer[i].real;
            mediaImaginaria = mediaImaginaria + listaMonitores[disco].Buffer[i].imaginaria;
            potencia = potencia + sqrt((pow(listaMonitores[disco].Buffer[i].real,2) + pow(listaMonitores[disco].Buffer[i].imaginaria,2)));
            ruido = ruido + listaMonitores[disco].Buffer[i].ruido;
            N+=1;
    //        printf("ruido no es-2.................................\n");
        }   
    }
    //mediaReal = mediaReal / visibilidades;
    //mediaImaginaria = mediaImaginaria / visibilidades;
    valores[0]=mediaReal;
    valores[1]=mediaImaginaria;
    valores[2]=potencia;
    valores[3]=ruido;
    valores[4]=N;
    if(termino==1){
        valores[5]=-1;
    }

} 

//Funcion que rellena de 0 un buffer en especifico dado por el programa
//Entrada: lista con los buffer de los distintos discos
//Salida: la misma lista con un buffer inicializado en 0
void rellenarBuffer(int disco){
    struct visibilidad* listaBuffer= (struct visibilidad*)malloc((sizeof(struct visibilidad)*buffer));
    struct visibilidad visBuffer1;
//printf("buffer:    %d",buffer);
    for (int j = 0; j < buffer; j++)
    {
        listaMonitores[disco].Buffer[j].u=-2;
        listaMonitores[disco].Buffer[j].v=-2;
        listaMonitores[disco].Buffer[j].imaginaria=-2;
        listaMonitores[disco].Buffer[j].real=-2;
        listaMonitores[disco].Buffer[j].ruido=-2;

    }
    listaMonitores[disco].lleno=-1;
}

//Funcion que encuentre un buffer que se encuentre lleno y retorna en cual de los discos esta dicho buffer
//Entrada: listas de buffers, la cantidad de discos y el tamaño de los buffer
//Salida: entero con el disco que debe ser rellenado
int bufferLleno(int cantDiscos){
    int lleno=-1;
    int contador=0;
    int k;
    for (int j = 0; j < cantDiscos; j++)
    {
        for (k = 0; k < buffer; k++){
            if((listaMonitores[j].Buffer[k].u!=-2)&&(listaMonitores[j].Buffer[k].v!=-2)&&(listaMonitores[j].Buffer[k].imaginaria!=-2)&&(listaMonitores[j].Buffer[k].real!=-2)&&(listaMonitores[j].Buffer[k].ruido!=-2))
            contador++;            
        }
        if(k==buffer){
            if(buffer==contador){
                lleno=j;
                j=cantDiscos;
            }else{
                contador=0;
            }
        }

    }
    if(lleno!=-1){
        listaMonitores[lleno].lleno=1;
    //    printf("lleno = %d\n",listaMonitores[lleno].lleno);
    }
    return lleno;
}

//funcion qeu devuelve al rpimera posicion vacia de un buffer
//Entrada: cantidad de disco y el buffer en especifico al que llaman
//Salida; entero que contiene la primera posicion vacia
int posicionVacia( int cantDiscos, int disco){
    int contador=0;
    for (int k = 0; k < buffer; k++){
        if((listaMonitores[disco].Buffer[k].u==-2)&&(listaMonitores[disco].Buffer[k].v==-2)&&(listaMonitores[disco].Buffer[k].imaginaria==-2)&&(listaMonitores[disco].Buffer[k].real==-2)&&(listaMonitores[disco].Buffer[k].ruido==-2)){
            contador=k;
            k=buffer;
        }
    }
    return contador;
}

//Funcion que ejecuta los proces de las hebras
//Entrada : el id de la hebra apra poder identificar  sobre que herbas estamso trabajando
//Salida : se actualizan las variables globales de listafina y la de monitorires
void* Ejecucion_Hebras(void *idHebra){
//  pthread_mutex_lock();
    //printf("pid:%d",pthread_self());
    int id = *((int*)idHebra);
    //printf("id:%d\n",id);
    double media_real=0;
    double media_imag=0;
    double potencia=0;
    double ruido=0;
    double N=0; 
    //final=0;
  //  printf("final");
    double valores[6];
    valores[0]=0;
    valores[1]=0;
    valores[2]=0;
    valores[3]=0;
    valores[4]=0;
    valores[5]=0;

    while(final==0 || casoFinal==1 || valores[5]==0 ) { //por cada vez que se le entregue un buffer
        
       // printf("%d",id);
       // printf("buffer lleno :%d\n",listaMonitores[id].lleno);

        pthread_mutex_lock(&listaMonitores[id].mut);
       // printf("----while\n");
        //printf("lenooooo???%d",listaMonitores[id].lleno);
        if(listaMonitores[id].lleno==-1 && final==0){
            pthread_cond_wait(&listaMonitores[id].notEmpty,&listaMonitores[id].mut);
        }
      //  printf("calculos1");
        
        calculos(id,valores);
        
        media_real=media_real + valores[0];
        media_imag=media_imag + valores[1];
        potencia=potencia + valores[2];
        ruido=ruido + valores[3];
        N = N + valores[4]; //valores[4]=largo del buffer 
    //    printf("caclulos2");
        
        if(valores[5]==-1 || final==1 || casoFinal==1){
  //          printf("guardo los datos...........................................................................................................");
            final=1;
            int hebraActual=id;

            listaFinal[hebraActual].real =media_real/N;
            listaFinal[hebraActual].imaginaria=media_imag/N;
            listaFinal[hebraActual].potencia=potencia;
            listaFinal[hebraActual].total=ruido;
            listaFinal[hebraActual].N=N;
        
 //           printf("aqui lo imprimio%d",listaFinal[hebraActual].N);
             //pthread_exit(NULL);
        }
        //SE VACIA NUEVAMENTE EL BUFFER Y SE INICIA MONITOR.FULL=-1
        rellenarBuffer(id);
       // listaMonitores[id].lleno=-1;
        pthread_cond_signal(&listaMonitores[id].notfull);
        pthread_mutex_unlock(&listaMonitores[id].mut);

    }

    pthread_exit(NULL);
}

int main (int argc, char* argv[]) {
  
    //Declaracion de las variables a utilizar
    int prueba;
    struct visibilidad visActual;
    int contadorLineas=1;
    char* nombreArchivo;
    char* nombreSalida;
    int bandera=0;
    int ancho;
    //Se reciben argumentos por pantalla
    recibirArgumentos(argc, argv,&nombreArchivo, &nombreSalida,&cantDiscos,&ancho,&buffer,&bandera);
    //Se pide memoria para el arreglo final
    listaFinal=(struct visibilidadParcial*)malloc((sizeof(struct visibilidadParcial)*cantDiscos));
    //Se guardan los radios interno y externos de los doscos
    double radios[cantDiscos][2];
    calculosRadiosDisco(ancho,cantDiscos,radios);
    for (int t = 0; t< cantDiscos; t++)
    {
        printf("  %f  \n", radios[t][1]);
    }
    //Variables a utilizar en el while sigueinte
    int discoActual;
    int finalizar=0;
    int pos;

    //se crean monitores
    Monitores();
    //se rellena  lo buffers de los monitores
    for (int j = 0; j < cantDiscos; j++)
    {
        rellenarBuffer(j);
    }  

    //elementos precvios al uso de hebras
    pthread_attr_t atributos;
    pthread_attr_init (&atributos);
    pthread_attr_setdetachstate (&atributos, PTHREAD_CREATE_JOINABLE);
    int i=0;
    //Creacion de estrcutura de id de herbas
    pthread_t idHebra[cantDiscos];
    int id[cantDiscos];
    for(int f=0;f<cantDiscos;f++){
        id[f]=f;
    }
    //Creacion de las hebras
    for(int i=0;i<(cantDiscos);i++){
        int error= pthread_create(&idHebra[cantDiscos], &atributos, Ejecucion_Hebras, &id[i]);
        //i=i+1;
    }
    //Se declara la estructura que llenaremos  a medida que recorramos el archivo
   struct visibilidad visBuffer;
   visActual=visibilidadLeida(nombreArchivo, contadorLineas);
   //Se le el archivo
   while (visActual.ruido!=-1){
        while (bufferLleno(cantDiscos)==-1 && contadorLineas<=maximochar(nombreArchivo))
        {
            //printf("entro visibilidades\n");
            visActual=visibilidadLeida(nombreArchivo, contadorLineas);
            visBuffer.u=visActual.u;
            visBuffer.v=visActual.v;
            visBuffer.imaginaria=visActual.imaginaria;
            visBuffer.real=visActual.real;
            visBuffer.ruido=visActual.ruido;
            discoActual= radioActual(cantDiscos,radios ,visActual.u,visActual.v);
            pos=posicionVacia(cantDiscos, discoActual);
            listaMonitores[discoActual].Buffer[pos]=visActual;
           // printf("\nbuff:%d\n",bufferLleno(cantDiscos));
            contadorLineas++;
            if(contadorLineas>=maximochar(nombreArchivo)){final=1;}
        }

        if(bufferLleno(cantDiscos)!=-1){
             pthread_mutex_lock(&listaMonitores[id[bufferLleno(cantDiscos)]].mut);
             prueba=bufferLleno(cantDiscos);

            if(bufferLleno(cantDiscos)!=-1){
                pthread_cond_signal(&listaMonitores[id[bufferLleno(cantDiscos)]].notEmpty);
                pthread_cond_wait(&listaMonitores[id[bufferLleno(cantDiscos)]].notfull,&listaMonitores[id[bufferLleno(cantDiscos)]].mut);
               
               }
               
               pthread_mutex_unlock(&listaMonitores[id[prueba]].mut);
        }
    
     }

    casoFinal=1;     
    //caso final
    //final=0;
    //printf("---------------------------------------------------------------------------------------------------------------------------------");
        //for(int j=0;j<cantDiscos;j++){
            //listaMonitores[j].lleno=1;
        //}
    for(int j=0;j<cantDiscos;j++){
        listaMonitores[j].lleno=1;
         while(listaMonitores[j].lleno==1){
            pthread_cond_signal(&listaMonitores[j].notEmpty);
            pthread_cond_wait(&listaMonitores[j].notfull,&listaMonitores[j].mut);
        } 
         pthread_mutex_unlock(&listaMonitores[j].mut);           

     }
    for(int d=0;d<cantDiscos;d++){
        printf("%lf",listaFinal[d].N);
    }
  
    //volver a llamar
    i=0;
    while(i<cantDiscos){
        pthread_join (idHebra[i],NULL);
        i+=1;
    }
    archivoSalida(nombreSalida);
    if(bandera==1){
        salidaBandera();
    }
    
    return 0;
}