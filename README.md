# P1-SS
Proyecto 1 - Computación Paralela y Distribuida

## Descripción
Este proyecto es un simulador espacial visual implementado en C++ utilizando la librería SDL2 para la renderización gráfica. El programa genera y anima varios elementos como planetas, asteroides, estrellas, nebulosas y explosiones, en un entorno visual dinámico. El objetivo principal del proyecto es mostrar la evolución de la paralelización del programa mediante la librería OpenMP para mejorar el rendimiento al distribuir las tareas en múltiples núcleos de CPU.

El simulador incluye varias versiones paralelizadas que muestran mejoras progresivas en el uso de los recursos y en la eficiencia computacional.

## Requisitos del Sistema
Este programa está diseñado para ejecutarse en una máquina virtual configurada con los siguientes componentes:

Software:
- Sistema operativo: Linux 
- Compilador: g++ con soporte para OpenMP
- SDL2: Librería de gráficos SDL2 y sus dependencias

Hardware:
- CPU: Múltiples núcleos (mínimo 2 núcleos)
- Memoria RAM: 4 GB o más
- GPU: No se requiere GPU dedicada, aunque podría mejorar el rendimiento.

## :notebook_with_decorative_cover: Instalación

1. Clonar el repositorio:
```
git clone git@github.com:FabianJuarez182/P1-SS.git
```
2. Instalar dependencias en la Máquina Virtual:
```shell
sudo apt-get update && sudo apt-get upgrade
```

```shell
sudo apt install g++ libomp-dev
```

```shell
sudo apt install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-mixer-dev
```

## :card_file_box: Uso

### Programa secuencial
Como primer paso, debemos compilar el programa en la terminal:

```shell
g++ -o SpaceScreensaver SpaceScreensaverS.cpp -lSDL2 
```

Luego, ejecutamos el programa en la Máquina Virtual:
```shell
./SpaceScreensaver <num_nebulas> <num_stars> <num_planets> <num_asteroids>
```

Ejemplo de ejecución del programa:
```shell
./SpaceScreensaver 500 100 5 10
```

### Programa Paralelo

```shell
g++ -o SpaceScreensaver SpaceScreensaverP.cpp -lSDL2  -fopenmp
```
Luego, ejecutamos el programa en la Máquina Virtual:
```shell
./SpaceScreensaver <num_nebulas> <num_stars> <num_planets> <num_asteroids>
```

Ejemplo de ejecución del programa:
```shell
./SpaceScreensaver 500 100 5 10
```
