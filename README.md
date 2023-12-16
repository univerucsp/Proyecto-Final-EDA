# Proyecto-Final-EDA
# Integrantes del Proyecto
- Marco Antonio Guillen Davila
- Anthony Wilfredo Mamani Mamani

# Instrucciones para Compilar y Ejecutar
### Requisitos previos
- Asegúrate de tener un compilador de C++ instalado en tu sistema.
- Tener python instalado para la visualizacion de los clusters y matplotlib.
```bash
pip install matplotlib
```

### Compilación
Abre una terminal y navega al directorio que contiene el archivo `programa.cpp`. Luego, ejecuta el siguiente comando:

```bash
g++ -o programa programa.cpp
```
### Ejecucion
Para ejecutar y obtener los resultados via terminal ejecutar lo siguiente:
```bash
./programa
```
- El programa primero preprocesara los puntos, una vez termine pedira los puntos extremos de el area rectangular que se quiere consultar los clusters, solo ingreselos y el programa deberia mostrarle inmediatamente los clusters.
- Si se quiere ver de manera visual los resultados ejecutar el siguiente comando:
```bash
python visual.py
```
