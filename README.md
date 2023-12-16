# Proyecto-Final-EDA
# Integrantes del Proyecto
- Marco Antonio Guillen Davila
- Anthony Wilfredo Mamani Mamani

# Instrucciones para Compilar y Ejecutar
### Requisitos previos
- Tener un compilador de C++ instalado en tu sistema.
- Tener python instalado para la visualizacion de los clusters y tener instalado matplotlib.
```bash
pip install matplotlib
```

### Compilación
Abrir una terminal y navegar al directorio que contiene el archivo `programa.cpp`. Luego, ejecutar el siguiente comando:

```bash
g++ -o programa programa.cpp
```
### Ejecucion
Para ejecutar y obtener los resultados via terminal ejecutar lo siguiente:
```bash
./programa
```
- El programa primero preprocesara los puntos, una vez termine pedira los puntos extremos de el area rectangular que se quiere consultar los clusters, solo ingreselos uno por uno y el programa deberia mostrarle inmediatamente los clusters.
- Terminando esto el programa preguntara si quiere continuar, solo escribir "si" para realice otra consulta de area y si se quiere salir escribir "no".
- Si se quiere ver de manera visual los resultados ejecutar el siguiente comando:
```bash
python visual.py
```
