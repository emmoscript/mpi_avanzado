# MPI Avanzado - Comunicaciones Colectivas

## Descripción del Proyecto

Este proyecto implementa un programa paralelo en C++ que utiliza las comunicaciones colectivas de MPI para calcular el promedio de valores generados aleatoriamente por cada proceso. El programa demuestra el uso efectivo de `MPI_Bcast`, `MPI_Reduce` y otros patrones de comunicación colectiva.

## Características Principales

- **Comunicaciones Colectivas**: Implementación completa de `MPI_Bcast` y `MPI_Reduce`
- **Cálculo Distribuido**: Cálculo eficiente del promedio usando paralelismo
- **Sincronización**: Puntos de sincronización bien definidos entre procesos
- **Benchmarks**: Medición de rendimiento de las operaciones colectivas
- **Pruebas Unitarias**: Validación de la corrección del programa
- **Documentación Completa**: Explicación detallada del código y algoritmos

## Estructura del Proyecto

```
mpi-avanzado/
├── CMakeLists.txt          # Configuración de CMake
├── README.md               # Este archivo
├── INSTALL.md              # Instrucciones de instalación
├── src/
│   ├── main.cpp            # Programa principal
│   ├── benchmark.cpp       # Programa de benchmarks
│   └── test.cpp           # Programa de pruebas
├── docs/
│   ├── informe.pdf         # Informe técnico detallado
│   └── presentacion.pdf    # Presentación del proyecto
└── scripts/
    ├── build.sh            # Script de compilación
    ├── run_tests.sh        # Script de ejecución de pruebas
    └── benchmark.sh        # Script de benchmarks
```

## Requisitos del Sistema

- **MPI**: OpenMPI o MPICH instalado
- **Compilador**: GCC 7+ o Clang 5+ con soporte para C++17
- **CMake**: Versión 3.16 o superior
- **Sistema Operativo**: Linux, macOS, o Windows con WSL

## Instalación

### 1. Clonar el Repositorio

```bash
git clone https://github.com/tu-usuario/mpi-avanzado.git
cd mpi-avanzado
```

### 2. Compilar el Proyecto

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### 3. Verificar la Instalación

```bash
# Ejecutar pruebas
mpirun -np 4 ./mpi_test

# Ejecutar benchmark
mpirun -np 4 ./mpi_benchmark
```

## Uso

### Programa Principal

```bash
# Ejecutar con 4 procesos
mpirun -np 4 ./mpi_promedio
```

El programa solicitará el número de valores por proceso (N) y mostrará:
- Valores generados por cada proceso
- Sumas parciales calculadas
- Promedio final calculado
- Tiempos de ejecución de cada operación

### Ejemplo de Salida

```
=== PROGRAMA MPI: CÁLCULO DE PROMEDIO CON COMUNICACIONES COLECTIVAS ===
Número total de procesos: 4
Ingrese el número de valores por proceso (N): 1000

Proceso raíz (rank 0) distribuyendo N = 1000 a todos los procesos...

Proceso 0:
  - Valores generados: 45.23, 67.89, 23.45, 89.12, 34.67, ... (y 995 más)
  - Suma parcial: 49876.34

Proceso 1:
  - Valores generados: 56.78, 12.34, 78.90, 45.67, 90.12, ... (y 995 más)
  - Suma parcial: 50123.45

...

=== RESULTADOS EN EL PROCESO RAÍZ ===
Suma total de todos los procesos: 200123.45
Número total de valores: 4000
Promedio calculado: 50.0309
Tiempo de reducción: 45 microsegundos

Proceso 0 recibió el promedio final: 50.0309
Proceso 1 recibió el promedio final: 50.0309
...

=== RESUMEN DE TIEMPOS ===
Tiempo de generación de datos: 1234 microsegundos
Tiempo de reducción: 45 microsegundos
Tiempo de broadcast final: 12 microsegundos

=== PROGRAMA COMPLETADO EXITOSAMENTE ===
```

### Benchmarks

```bash
# Ejecutar benchmarks con diferentes configuraciones
mpirun -np 2 ./mpi_benchmark
mpirun -np 4 ./mpi_benchmark
mpirun -np 8 ./mpi_benchmark
```

Los resultados se guardan en archivos CSV para análisis posterior.

### Pruebas

```bash
# Ejecutar todas las pruebas
mpirun -np 4 ./mpi_test
```

## Operaciones Colectivas Implementadas

### 1. MPI_Bcast
- **Propósito**: Distribuir el valor N desde el proceso raíz a todos los procesos
- **Uso**: Sincronización inicial y distribución del promedio final
- **Punto de Sincronización**: Todos los procesos deben participar

### 2. MPI_Reduce
- **Propósito**: Sumar todas las contribuciones parciales en el proceso raíz
- **Operación**: MPI_SUM para agregar las sumas locales
- **Punto de Sincronización**: Todos los procesos deben participar

### 3. MPI_Barrier
- **Propósito**: Sincronización explícita entre fases del programa
- **Uso**: Garantizar que todos los procesos lleguen al mismo punto

## Análisis de Rendimiento

El programa incluye mediciones de tiempo para:
- Generación de datos locales
- Operaciones de comunicación colectiva
- Cálculos computacionales

Los benchmarks permiten analizar:
- Escalabilidad con diferentes números de procesos
- Impacto del tamaño de datos en el rendimiento
- Overhead de las comunicaciones colectivas

## Puntos Críticos y Sincronización

### Puntos de Sincronización Identificados:

1. **Inicialización**: `MPI_Barrier` antes de comenzar
2. **Broadcast de N**: `MPI_Bcast` para distribuir parámetros
3. **Antes de Reducción**: `MPI_Barrier` para sincronizar
4. **Reducción**: `MPI_Reduce` para agregar resultados
5. **Broadcast Final**: `MPI_Bcast` para distribuir resultado
6. **Finalización**: `MPI_Barrier` antes de terminar

### Posibles Puntos Críticos:

- **Deadlock**: Si no todos los procesos participan en operaciones colectivas
- **Desincronización**: Si los procesos no llegan a los puntos de sincronización
- **Overhead**: Comunicaciones colectivas pueden ser costosas con muchos procesos

## Contribución

1. Fork el proyecto
2. Crea una rama para tu feature (`git checkout -b feature/AmazingFeature`)
3. Commit tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abre un Pull Request

## Licencia

Este proyecto está bajo la Licencia MIT. Ver el archivo `LICENSE` para más detalles.
 
