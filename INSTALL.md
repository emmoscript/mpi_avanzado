# Guía de Instalación - MPI Avanzado

## Índice

1. [Requisitos Previos](#requisitos-previos)
2. [Instalación en Linux](#instalación-en-linux)
3. [Instalación en macOS](#instalación-en-macos)
4. [Instalación en Windows](#instalación-en-windows)
5. [Verificación de la Instalación](#verificación-de-la-instalación)
6. [Solución de Problemas](#solución-de-problemas)

## Requisitos Previos

### Software Necesario

- **MPI**: OpenMPI 4.0+ o MPICH 3.3+
- **Compilador C++**: GCC 7+, Clang 5+, o MSVC 2017+
- **CMake**: Versión 3.16 o superior
- **Make**: GNU Make o equivalente
- **Git**: Para clonar el repositorio

### Hardware Recomendado

- **CPU**: Múltiples núcleos (4+ recomendado)
- **RAM**: 4GB mínimo, 8GB recomendado
- **Almacenamiento**: 1GB de espacio libre

## Instalación en Linux

### Ubuntu/Debian

```bash
# Actualizar repositorios
sudo apt update

# Instalar dependencias básicas
sudo apt install build-essential cmake git

# Instalar OpenMPI
sudo apt install openmpi-bin libopenmpi-dev

# Verificar instalación
mpicc --version
mpirun --version
```

### CentOS/RHEL/Fedora

```bash
# Instalar dependencias básicas
sudo yum groupinstall "Development Tools"
sudo yum install cmake git

# Instalar OpenMPI
sudo yum install openmpi openmpi-devel

# Cargar módulo MPI
module load mpi/openmpi-x86_64

# Verificar instalación
mpicc --version
mpirun --version
```

### Arch Linux

```bash
# Instalar dependencias
sudo pacman -S base-devel cmake git

# Instalar OpenMPI
sudo pacman -S openmpi

# Verificar instalación
mpicc --version
mpirun --version
```

## Instalación en macOS

### Usando Homebrew

```bash
# Instalar Homebrew (si no está instalado)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Instalar dependencias
brew install cmake git

# Instalar OpenMPI
brew install open-mpi

# Verificar instalación
mpicc --version
mpirun --version
```

### Usando MacPorts

```bash
# Instalar MacPorts (si no está instalado)
# Descargar desde https://www.macports.org/

# Instalar dependencias
sudo port install cmake git

# Instalar OpenMPI
sudo port install openmpi

# Verificar instalación
mpicc --version
mpirun --version
```

## Instalación en Windows

### Usando WSL (Recomendado)

```bash
# Instalar WSL2
wsl --install

# En WSL, seguir las instrucciones de Ubuntu
sudo apt update
sudo apt install build-essential cmake git openmpi-bin libopenmpi-dev
```

### Usando MSYS2

```bash
# Instalar MSYS2 desde https://www.msys2.org/

# Abrir MSYS2 MinGW 64-bit
pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-openmpi
pacman -S mingw-w64-x86_64-git

# Agregar al PATH
export PATH="/mingw64/bin:$PATH"
```

### Usando Visual Studio

1. Instalar Visual Studio 2019/2022 con C++ workload
2. Instalar Microsoft MPI (MS-MPI)
3. Configurar variables de entorno
4. Usar CMake con generador Visual Studio

## Compilación del Proyecto

### 1. Clonar el Repositorio

```bash
git clone https://github.com/tu-usuario/mpi-avanzado.git
cd mpi-avanzado
```

### 2. Crear Directorio de Build

```bash
mkdir build
cd build
```

### 3. Configurar con CMake

```bash
# Configuración básica
cmake ..

# O con configuración específica
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3" ..
```

### 4. Compilar

```bash
# Compilación básica
make

# Compilación paralela
make -j$(nproc)

# O en Windows con Visual Studio
cmake --build . --config Release
```

## Verificación de la Instalación

### 1. Verificar MPI

```bash
# Verificar que MPI está disponible
mpicc --version
mpirun --version

# Ejecutar programa de prueba MPI
echo '#include <mpi.h>
#include <stdio.h>
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("Proceso %d de %d\n", rank, size);
    MPI_Finalize();
    return 0;
}' > test_mpi.c

mpicc -o test_mpi test_mpi.c
mpirun -np 4 ./test_mpi
```

### 2. Verificar el Proyecto

```bash
# Ejecutar pruebas
mpirun -np 4 ./mpi_test

# Ejecutar benchmark
mpirun -np 4 ./mpi_benchmark

# Ejecutar programa principal
mpirun -np 4 ./mpi_promedio
```

## Configuración Avanzada

### Variables de Entorno

```bash
# Configurar MPI
export OMPI_CC=gcc
export OMPI_CXX=g++
export MPICC=mpicc
export MPICXX=mpicxx

# Configurar CMake
export CMAKE_PREFIX_PATH=/usr/local/lib:$CMAKE_PREFIX_PATH
```

### Configuración de CMake

```cmake
# CMakeLists.txt personalizado
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar MPI
find_package(MPI REQUIRED)

# Configurar flags de optimización
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra")
```

## Solución de Problemas

### Error: MPI no encontrado

```bash
# Verificar instalación de MPI
which mpicc
which mpirun

# Reinstalar MPI si es necesario
sudo apt remove --purge openmpi-bin libopenmpi-dev
sudo apt install openmpi-bin libopenmpi-dev
```

### Error: CMake no encuentra MPI

```bash
# Especificar ruta de MPI manualmente
cmake -DMPI_C_COMPILER=/usr/bin/mpicc -DMPI_CXX_COMPILER=/usr/bin/mpicxx ..
```

### Error: Compilación falla

```bash
# Verificar versión del compilador
gcc --version
g++ --version

# Verificar flags de compilación
echo $CXXFLAGS
echo $LDFLAGS
```

### Error: Ejecución falla

```bash
# Verificar que MPI puede ejecutar procesos
mpirun -np 1 hostname

# Verificar configuración de red
ompi_info | grep -i network

# Ejecutar con debug
mpirun -np 4 --debug ./mpi_promedio
```

### Problemas de Rendimiento

```bash
# Verificar configuración de procesos
nproc
lscpu

# Optimizar para el hardware específico
export OMPI_MCA_btl_base_warn_component_unused=0
export OMPI_MCA_btl="^openib"
```

## Configuración de Entorno de Desarrollo

### Visual Studio Code

1. Instalar extensión C/C++
2. Instalar extensión CMake Tools
3. Configurar `c_cpp_properties.json`:

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "/usr/include/openmpi-x86_64"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
```

### CLion

1. Configurar toolchain con MPI
2. Configurar CMake con MPI
3. Configurar run configurations para MPI

## Próximos Pasos

1. **Leer la documentación**: Revisar `README.md` y `docs/`
2. **Ejecutar ejemplos**: Probar diferentes configuraciones
3. **Experimentar**: Modificar parámetros y analizar resultados
4. **Contribuir**: Reportar bugs o sugerir mejoras

## Soporte

- **Issues**: Crear issue en GitHub
- **Documentación**: Revisar archivos en `docs/`
- **Comunidad**: Foros de MPI y OpenMPI
- **Email**: [tu.email@ejemplo.com]

## Referencias Adicionales

- [OpenMPI Installation Guide](https://www.open-mpi.org/faq/?category=building)
- [MPICH Installation Guide](https://www.mpich.org/documentation/guides/)
- [CMake MPI Documentation](https://cmake.org/cmake/help/latest/module/FindMPI.html)
- [MPI Forum Standards](https://www.mpi-forum.org/docs/) 