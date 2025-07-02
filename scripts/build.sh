#!/bin/bash

# Script de compilación para MPI Avanzado

set -e  # Salir en caso de error

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Función para imprimir mensajes con colores
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Función para mostrar ayuda
show_help() {
    echo "Script de compilación para MPI Avanzado"
    echo ""
    echo "Uso: $0 [OPCIONES]"
    echo ""
    echo "Opciones:"
    echo "  -h, --help          Mostrar esta ayuda"
    echo "  -c, --clean         Limpiar directorio build"
    echo "  -d, --debug         Compilar en modo debug"
    echo "  -r, --release       Compilar en modo release (por defecto)"
    echo "  -j, --jobs N        Usar N jobs para compilación paralela"
    echo "  -v, --verbose       Mostrar output detallado"
    echo ""
    echo "Ejemplos:"
    echo "  $0                  # Compilación básica en release"
    echo "  $0 -d               # Compilación en modo debug"
    echo "  $0 -j 8             # Compilación con 8 jobs"
    echo "  $0 -c -d -j 4       # Limpiar y compilar en debug con 4 jobs"
}

# Variables por defecto
BUILD_TYPE="Release"
CLEAN_BUILD=false
VERBOSE=false
JOBS=$(nproc)

# Parsear argumentos
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        *)
            print_error "Opción desconocida: $1"
            show_help
            exit 1
            ;;
    esac
done

# Verificar que estamos en el directorio correcto
if [[ ! -f "CMakeLists.txt" ]]; then
    print_error "No se encontró CMakeLists.txt. Ejecutar desde el directorio raíz del proyecto."
    exit 1
fi

print_info "Iniciando compilación de MPI Avanzado..."
print_info "Tipo de build: $BUILD_TYPE"
print_info "Jobs: $JOBS"

# Verificar dependencias
print_info "Verificando dependencias..."

# Verificar MPI
if ! command -v mpicc &> /dev/null; then
    print_error "MPI no está instalado o no está en el PATH"
    print_info "Instalar MPI con: sudo apt install openmpi-bin libopenmpi-dev"
    exit 1
fi

# Verificar CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake no está instalado"
    print_info "Instalar CMake con: sudo apt install cmake"
    exit 1
fi

# Verificar Make
if ! command -v make &> /dev/null; then
    print_error "Make no está instalado"
    print_info "Instalar Make con: sudo apt install build-essential"
    exit 1
fi

print_success "Todas las dependencias están instaladas"

# Crear directorio build si no existe
if [[ ! -d "build" ]]; then
    print_info "Creando directorio build..."
    mkdir build
fi

cd build

# Limpiar build si se solicita
if [[ "$CLEAN_BUILD" == true ]]; then
    print_info "Limpiando directorio build..."
    rm -rf *
fi

# Configurar CMake
print_info "Configurando CMake..."
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

if [[ "$BUILD_TYPE" == "Release" ]]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_CXX_FLAGS_RELEASE='-O3 -march=native -DNDEBUG'"
elif [[ "$BUILD_TYPE" == "Debug" ]]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_CXX_FLAGS_DEBUG='-O0 -g -Wall -Wextra'"
fi

if [[ "$VERBOSE" == true ]]; then
    CMAKE_ARGS="$CMAKE_ARGS --debug-output"
fi

cmake $CMAKE_ARGS ..

# Compilar
print_info "Compilando con $JOBS jobs..."
if [[ "$VERBOSE" == true ]]; then
    make -j$JOBS VERBOSE=1
else
    make -j$JOBS
fi

# Verificar que los ejecutables se crearon
print_info "Verificando ejecutables..."

EXECUTABLES=("mpi_promedio" "mpi_benchmark" "mpi_test")
MISSING_EXECUTABLES=()

for exec in "${EXECUTABLES[@]}"; do
    if [[ -f "$exec" ]]; then
        print_success "$exec compilado exitosamente"
    else
        MISSING_EXECUTABLES+=("$exec")
    fi
done

if [[ ${#MISSING_EXECUTABLES[@]} -gt 0 ]]; then
    print_error "Los siguientes ejecutables no se crearon:"
    for exec in "${MISSING_EXECUTABLES[@]}"; do
        print_error "  - $exec"
    done
    exit 1
fi

# Mostrar información de los ejecutables
print_info "Información de los ejecutables:"
for exec in "${EXECUTABLES[@]}"; do
    if [[ -f "$exec" ]]; then
        SIZE=$(du -h "$exec" | cut -f1)
        print_info "  - $exec: $SIZE"
    fi
done

print_success "Compilación completada exitosamente!"
print_info ""
print_info "Para ejecutar el programa principal:"
print_info "  mpirun -np 4 ./mpi_promedio"
print_info ""
print_info "Para ejecutar las pruebas:"
print_info "  mpirun -np 4 ./mpi_test"
print_info ""
print_info "Para ejecutar benchmarks:"
print_info "  mpirun -np 4 ./mpi_benchmark" 