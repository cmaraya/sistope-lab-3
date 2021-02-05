# sistope-lab-3
Laboratorio 3 para el ramo sistope

Autores: Catalina Araya y Ekaterina Cornejo.

Sistope lab 3, lavoratorio no logrado en su totalidad.

Enunciado:

IV.A. Hoyos negros, discos proto planetarios y ALMA
Hace un a ̃no aproximadamente los cient ́ıficos han logrado reconstruir la primera imagen del horizonte de
eventos de un hoyo negro. Esta frontera marca el lugar desde donde el cual ya nada puede salir del hoyo
negro, ni materia ni luz. Es por eso que la imagen muestra la luz, radiaci ́on fuera de la frontera y dentro de
la frontera s ́olo se ve oscuridad. La figura 1 muestra la primera imagen de un hoyo negro:

Figure 1. Primera imagen de un hoyo negro.

Esta imagen es el resultado de un proceso computacional llamado s ́ıntesis o reconstrucci ́on de im ́agenes
interferom ́etricas. Es decir, las antenas no capturan la imagen que nosotros vemos, sino detectan se ̃nales de
radio, las cuales son transformadas por un programa computacional en la imagen visual.
En estricto rigor los datos recolectados por las antenas corresponden a muestras del espacio Fourier de
la imagen. Estas muestras est ́an repartidas en forma no uniforme e irregular en el plano Fourier. La figura
2 muestra un t ́ıpico patr ́on de muestreo del plano Fourier. La imagen de la derecha muestra la imagen
reconstruida a partir de los datos. Esta imagen corresponde a un disco protoplanetario llamado HLTau.

Figure 2. Plano (u, v) y imagen sintetizada de HLTau.

En cada punto (u, v) se mide una se ̃nal llamada Visibilidad. Cada visibilidad es un n ́umero complejo, es
decir posee una parte real y otra imaginaria. La imagen por otro lado es solo real.
Usaremos la siguiente notaci ́on para describir los datos:
• V (u, v) es la visibilidad en la coordenada (u, v) del plano Fourier
• V (u, v).r es la parte real de la visibilidad
• V (u, v).i es la parte imaginaria de la visibilidad
• V (u, v).w es el ruido en la coordenada (u, v)
La siguiente lista es un ejemplo de una peque ̃na muestra de visibilidades
46.75563,-160.447845,-0.014992,0.005196,0.005011
119.08387,-30.927526,0.016286,-0.001052,0.005888
-132.906616,58.374054,-0.009442,-0.001208,0.003696
-180.271179,-43.749226,-0.011654,0.001075,0.003039
-30.299767,-126.668739,0.015222,-0.004145,0.007097
-18.289482,28.76403,0.025931,0.001565,0.004362
La primera columna es la posici ́on en el eje u; la segunda es la posici ́on en el eje v. la tercera columna es
el valor real de la visibilidad, la cuarta, el valor imaginario y finalmente, la quinta es el ruido. Estos datos
corresponden a datos reales de un disco proto planetario captadas por el observatorio ALMA.
El n ́umero de visibilidades depende de cada captura de datos y del telescopio usado, pero puede variar
entre varios cientos de miles de puntos hasta cientos de millones. Por lo tanto, el procesamiento de las
visibilidades es una tarea computacionalmente costosa y generalmente se usa paralelismo para acelerar los
procesos. En este lab, simularemos este proceso usando varias hebras que cooperan para hacer c ́alculo a las
visibilidades. Por ahora, no haremos s ́ıntesis de im ́agenes.

IV.B. C ́alculo de propiedades
En ciertas aplicaciones, antes de sintetizar la imagen, es necesario y  ́util extraer caracter ́ısticas del plano
Fourier. Uno de estos preprocesamiento, agrupa las visibilidades en anillos conc ́entricos con centro en el
(0, 0) y radios crecientes. Por ejemplo, suponga que definimos un radio r, el cual divide las visibilidades en
dos anillos, aquellas visibilidades cuya distancia al centro es menor o igual a R, y aquellas cuya distancia al
centro es mayor a R. La distancia de una visibilidad al centro es simplemente:

d(u, v) = (u^2 + v^2)^1/2

Si definieramos dos radios R1 y R2 (R1 < R2), las visibilidades se dividir ́ıan en tres discos: 0 ≤ d(u, v) <
R1, R1 ≤ d(u, v) < R2, y R2 ≤ d(u, v).
Para este lab, las propiedades que se calcular ́an por disco son
1. Media real : 1/N* sumaria* (u, v).r, donde N es el n ́umero de visibilidades en el disco

2. Media imaginaria : 1/N sumatoria (u, v).i, donde N es el n ́umero de visibilidades en el disco

3. Potencia: sumatoria((V (u, v).r)^2 + (V (u, v).i)^2)^1/2

4. Ruido total: sumatoria (u, v).w

V. El programa del lab

V.A. L ́ogica de solucion
En este laboratorio crearemos un conjunto de procesos que calculen las propiedades antes descritas. Usted
debe organizar su soluci ́on de la siguiente manera.
1. El proceso principal recibir ́a argumentos por l ́ınea de comando el n ́umero de radios en los que se debe
dividir el plano de Fourier. Adem ́as recibir ́a el ancho ∆r de cada intervalo, tal que R1 = ∆r, R2 = 2∆r,
etc. Este proceso tambi ́en recibe como argumento el nombre del archivo de entrada, el nombre del
archivo de salida y el tama ̃no del b ́uffer de cada monitor.

2. El proceso principal debe crear tantos monitores y hebras como n ́umero de discos hayan sido especifi-
cados (una hebra por cada disco). Adem ́as de mutex, variables de condici ́on y cualquier herramienta

de pthreads que sea necesaria para proveer exclusi ́on mutua y sincronizaci ́on.
3. El proceso principal (hebra padre) debe instanciar una estructura com ́un en la cual todas las hebras
deber ́an escribir los resultados de las propiedades.
4. El proceso principal leer ́a l ́ınea a l ́ınea el archivo de entrada con formato .csv y determinar ́a el disco
al que pertenece la visibilidad, y utilizando un monitor, pondr ́a a disposici ́on de cada hebra los datos
que le correspondan, en funci ́on del tama ̃no de b ́uffer ingresado como par ́ametro.
5. Cada hebra deber ́a recoger los datos que le correspondan cuando el b ́uffer se encuentre lleno, adem ́as
debe calcular parcialmente las propiedades.
6. Cuando el proceso principal termine de leer y enviar todas las visibilidades, debe indicar que en caso
de no encontrarse llenos los b ́uffer, como ya se acabaron los datos, entonces las hebras de todas formas
pueden recoger lo que haya en los b ́uffer (evita deadlock).
7. Una vez que se hayan procesado todos los datos, cada hebra debe escribir en la estructura com ́un los
resultados finales de las propiedades.
8. El proceso principal lee los resultados de la estructura com ́un y escribe el archivo de salida final.
Para implementar esta l ́ogica, se debe tener un programa. Este ser ́a lab3.c el cual ser ́a el proceso
padre y por lo tanto se encargar ́a de las funciones que le corresponde (previamente descritas), como por
ejemplo leer el archivo y crear hebras, para luego distribuir los datos seg ́un el radio entre cada hebra.
Esto se har ́a implementando un monitor por cada disco y llenando un b ́uffer tambi ́en por cada disco con

todos los datos correspondientes. Por ejemplo, si el b ́uffer es igual a 10, entonces se deben recolectar 10
datos correspondientes al disco 1 para que reci ́en en ese momento la hebra correspondiente al disco 1 pueda
recoger dichos datos y calcular parcialmente las propiedades pedidas.
Por otro lado, las hebras deben recoger los datos correspondientes a su disco seg ́un el tama ̃no del b ́uffer y
mediante acumuladores calcular parcialmente las propiedades. Las hebras adem ́as esperar ́an un mensaje de
t ́ermino, para que no se produzca deadlock en caso de que nunca se llegue a completar el b ́uffer del monitor,
el cual indicar ́a que ya pueden calcular las propiedades finales. Las propiedades finales deben ser guardadas
en una estructura com ́un (por lo tanto debe protegerse con exclusi ́on mutua) y finalmente el proceso principal
debe leer los resultados en esta estructura com ́un y escribirlos en un archivo de salida.
El archivo que debe escribir el proceso principal con las propiedades calculadas por las hebras, debe tener
el siguiente formato:

Figure 3. Ejemplo archivo de salida.

Cabe destacar que se espera un resultado en cuanto al c ́alculo de propiedades igual a la experiencia
anterior.
En donde el nombre del archivo es el nombre entregado como argumento (en este caso: propiedades.txt),
adem ́as se incluyen todas las propiedades que cada hijo debe calcular en su disco asignado seg ́un el ancho y
el n ́umero de discos tambi ́en ingresados como argumentos (en este caso: 3 discos). El formato debe indicar
cada disco de forma ordenada y luego mostrar sus propiedades indicando el nombre de estas y su resultado.
El programa se ejecutar ́a usando los siguientes argumentos (ejemplo):
$ ./lab3 -i visibilidades.csv -o propiedades.txt -d ancho -n ndiscos -s tama~nobuffer -b
• -i: nombre de archivo con visibilidades, el cual tiene como formato uv values i.csv con i = 0,1,2,....
• -o: nombre de archivo de salida
• -n: cantidad de discos
• -d: ancho de cada disco
• -s: tama ̃no del buffer de cada monitor

• -b: bandera o flag que permite indicar si se quiere ver por consola la cantidad de visibilidades encon-
tradas por cada proceso hijo. Es decir, si se indica el flag, entonces se muestra la salida por consola.

Ejemplo de compilaci ́on y ejecuci ́on:
>> gcc lab3.c -o lab3
>> ./lab3 -i uvplaneco65.csv -o propiedades.txt -d 100 -n 4 -s 10 -b
En el primer caso, se ejecuta el programa entregando como nombre de archivo de entrada uvplaneco65.csv,
adem ́as el proceso principal deber ́a crear 4 hebras y cada hebra recoger ́a los datos que le corresponden y le
entrega el proceso principal cuando se llene el b ́uffer de tama ̃no 10 (es decir, puede guardar 10 filas de datos).

Las visibilidades se repartir ́an entre los siguientes rangos: [0, 100), [100, 200), [200, 300), [300,). Finalmente,
debido a que el primer caso considera el flag -b, cada hebra debe identificarse y mostrar la cantidad de
visibilidades que proces ́o. Ejemplo:
Soy la hebra 1, proces ́e 10000 visibilidades
Soy la hebra 2, proces ́e 14000 visibilidades
...
El segundo caso, funciona igual al anterior, siendo la  ́unica diferencia que las hebras no deben mostrar
ning ́un tipo de salida por consola.
Como requerimientos no funcionales, se exige lo siguiente:
• Debe funcionar en sistemas operativos con kernel Linux.
• Debe ser implementado en lenguaje de programaci ́on C.
• Se debe utilizar un archivo Makefile para su compilaci ́on.
• Realizar el programa utilizando buenas pr ́acticas, dado que este laboratorio no contiene manual de
usuario ni informe, es necesario que todo est ́e debidamente comentado.
• Que el programa principal est ́e desacoplado, es decir, que se desarrollen las funciones correspondientes
en otro archivo .c para mayor entendimiento de la ejecuci ́on.
• Se deben verificar y validar los par ́ametros de entrada.
