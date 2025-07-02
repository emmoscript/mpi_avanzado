#!/bin/bash

# Script de ejecución de pruebas para MPI Avanzado

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
    echo "Script de ejecución de pruebas para MPI Avanzado"
    echo ""
    echo "Uso: $0 [OPCIONES]"
    echo ""
    echo "Opciones:"
    echo "  -h, --help          Mostrar esta ayuda"
    echo "  -p, --processes N   Número de procesos (por defecto: 4)"
    echo "  -a, --all           Ejecutar todas las configuraciones de prueba"
    echo "  -v, --verbose       Mostrar output detallado"
    echo "  -o, --output FILE   Guardar output en archivo"
    echo "  -t, --timeout SEC   Timeout en segundos (por defecto: 60)"
    echo ""
    echo "Ejemplos:"
    echo "  $0                  # Ejecutar pruebas con 4 procesos"
    echo "  $0 -p 8             # Ejecutar pruebas con 8 procesos"
    echo "  $0 -a               # Ejecutar todas las configuraciones"
    echo "  $0 -v -o test.log   # Ejecutar con output detallado y guardar en archivo"
}

# Variables por defecto
NUM_PROCESSES=4
RUN_ALL=false
VERBOSE=false
OUTPUT_FILE=""
TIMEOUT=60

# Parsear argumentos
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -p|--processes)
            NUM_PROCESSES="$2"
            shift 2
            ;;
        -a|--all)
            RUN_ALL=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -o|--output)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        -t|--timeout)
            TIMEOUT="$2"
            shift 2
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

# Verificar que el ejecutable existe
if [[ ! -f "build/mpi_test" ]]; then
    print_error "Ejecutable mpi_test no encontrado. Compilar primero con ./scripts/build.sh"
    exit 1
fi

print_info "Iniciando ejecución de pruebas..."
print_info "Número de procesos: $NUM_PROCESSES"
print_info "Timeout: $TIMEOUT segundos"

# Función para ejecutar prueba con timeout
run_test_with_timeout() {
    local num_procs=$1
    local test_name=$2
    
    print_info "Ejecutando $test_name con $num_procs procesos..."
    
    local start_time=$(date +%s)
    
    if [[ -n "$OUTPUT_FILE" ]]; then
        echo "=== $test_name con $num_procs procesos ===" >> "$OUTPUT_FILE"
        echo "Fecha: $(date)" >> "$OUTPUT_FILE"
        echo "" >> "$OUTPUT_FILE"
    fi
    
    if timeout $TIMEOUT mpirun -np $num_procs ./build/mpi_test 2>&1 | tee -a "$OUTPUT_FILE"; then
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        print_success "$test_name completado exitosamente en ${duration}s"
        return 0
    else
        local exit_code=$?
        if [[ $exit_code -eq 124 ]]; then
            print_error "$test_name falló por timeout después de ${TIMEOUT}s"
        else
            print_error "$test_name falló con código de salida $exit_code"
        fi
        return 1
    fi
}

# Función para verificar resultado de prueba
check_test_result() {
    local output="$1"
    if echo "$output" | grep -q "TODAS LAS PRUEBAS PASARON EXITOSAMENTE"; then
        return 0
    else
        return 1
    fi
}

# Configuraciones de prueba
TEST_CONFIGS=(2 4 8 16)

# Contadores
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Ejecutar pruebas
if [[ "$RUN_ALL" == true ]]; then
    print_info "Ejecutando todas las configuraciones de prueba..."
    
    for num_procs in "${TEST_CONFIGS[@]}"; do
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        
        if run_test_with_timeout $num_procs "Prueba con $num_procs procesos"; then
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
        
        # Pausa entre pruebas
        sleep 2
    done
else
    TOTAL_TESTS=1
    if run_test_with_timeout $NUM_PROCESSES "Prueba con $NUM_PROCESSES procesos"; then
        PASSED_TESTS=1
    else
        FAILED_TESTS=1
    fi
fi

# Mostrar resumen
print_info ""
print_info "=== RESUMEN DE PRUEBAS ==="
print_info "Total de pruebas: $TOTAL_TESTS"
print_info "Pruebas exitosas: $PASSED_TESTS"
print_info "Pruebas fallidas: $FAILED_TESTS"

if [[ $FAILED_TESTS -eq 0 ]]; then
    print_success "¡Todas las pruebas pasaron exitosamente!"
    exit 0
else
    print_error "Algunas pruebas fallaron. Revisar el output para más detalles."
    exit 1
fi 