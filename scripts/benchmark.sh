#!/bin/bash

# Script de benchmarks para MPI Avanzado

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
    echo "Script de benchmarks para MPI Avanzado"
    echo ""
    echo "Uso: $0 [OPCIONES]"
    echo ""
    echo "Opciones:"
    echo "  -h, --help          Mostrar esta ayuda"
    echo "  -p, --processes N   Número de procesos (por defecto: 4)"
    echo "  -a, --all           Ejecutar todas las configuraciones de benchmark"
    echo "  -v, --verbose       Mostrar output detallado"
    echo "  -o, --output DIR    Directorio de salida (por defecto: benchmarks/)"
    echo "  -i, --iterations N  Número de iteraciones (por defecto: 100)"
    echo "  -t, --timeout SEC   Timeout en segundos (por defecto: 300)"
    echo ""
    echo "Ejemplos:"
    echo "  $0                  # Ejecutar benchmark con 4 procesos"
    echo "  $0 -p 8             # Ejecutar benchmark con 8 procesos"
    echo "  $0 -a               # Ejecutar todas las configuraciones"
    echo "  $0 -v -o results/   # Ejecutar con output detallado y directorio personalizado"
}

# Variables por defecto
NUM_PROCESSES=4
RUN_ALL=false
VERBOSE=false
OUTPUT_DIR="benchmarks"
ITERATIONS=100
TIMEOUT=300

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
            OUTPUT_DIR="$2"
            shift 2
            ;;
        -i|--iterations)
            ITERATIONS="$2"
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
if [[ ! -f "build/mpi_benchmark" ]]; then
    print_error "Ejecutable mpi_benchmark no encontrado. Compilar primero con ./scripts/build.sh"
    exit 1
fi

print_info "Iniciando ejecución de benchmarks..."
print_info "Número de procesos: $NUM_PROCESSES"
print_info "Iteraciones: $ITERATIONS"
print_info "Timeout: $TIMEOUT segundos"
print_info "Directorio de salida: $OUTPUT_DIR"

# Crear directorio de salida si no existe
if [[ ! -d "$OUTPUT_DIR" ]]; then
    print_info "Creando directorio de salida: $OUTPUT_DIR"
    mkdir -p "$OUTPUT_DIR"
fi

# Función para ejecutar benchmark con timeout
run_benchmark_with_timeout() {
    local num_procs=$1
    local benchmark_name=$2
    
    print_info "Ejecutando $benchmark_name con $num_procs procesos..."
    
    local start_time=$(date +%s)
    local output_file="$OUTPUT_DIR/benchmark_${num_procs}procs_$(date +%Y%m%d_%H%M%S).log"
    
    # Configurar variables de entorno para el benchmark
    export BENCHMARK_ITERATIONS=$ITERATIONS
    
    if timeout $TIMEOUT mpirun -np $num_procs ./build/mpi_benchmark 2>&1 | tee "$output_file"; then
        local end_time=$(date +%s)
        local duration=$((end_time - start_time))
        print_success "$benchmark_name completado exitosamente en ${duration}s"
        print_info "Resultados guardados en: $output_file"
        return 0
    else
        local exit_code=$?
        if [[ $exit_code -eq 124 ]]; then
            print_error "$benchmark_name falló por timeout después de ${TIMEOUT}s"
        else
            print_error "$benchmark_name falló con código de salida $exit_code"
        fi
        return 1
    fi
}

# Función para generar reporte de benchmarks
generate_benchmark_report() {
    local report_file="$OUTPUT_DIR/benchmark_report_$(date +%Y%m%d_%H%M%S).md"
    
    print_info "Generando reporte de benchmarks..."
    
    cat > "$report_file" << EOF
# Reporte de Benchmarks - MPI Avanzado

**Fecha:** $(date)
**Sistema:** $(uname -a)
**CPU:** $(lscpu | grep "Model name" | cut -d: -f2 | xargs)
**Núcleos:** $(nproc)

## Configuración de Benchmarks

- **Iteraciones por prueba:** $ITERATIONS
- **Timeout por prueba:** $TIMEOUT segundos
- **Configuraciones probadas:** ${TEST_CONFIGS[*]}

## Resultados

EOF

    # Buscar archivos CSV de resultados
    for csv_file in "$OUTPUT_DIR"/*.csv; do
        if [[ -f "$csv_file" ]]; then
            echo "### Resultados de $(basename "$csv_file")" >> "$report_file"
            echo "" >> "$report_file"
            echo "\`\`\`csv" >> "$report_file"
            cat "$csv_file" >> "$report_file"
            echo "\`\`\`" >> "$report_file"
            echo "" >> "$report_file"
        fi
    done

    # Agregar análisis de rendimiento
    cat >> "$report_file" << EOF

## Análisis de Rendimiento

### Escalabilidad

Los benchmarks muestran cómo el rendimiento varía con diferentes números de procesos:

- **MPI_Bcast**: Tiempo de comunicación para distribuir datos
- **MPI_Reduce**: Tiempo de agregación de resultados
- **Programa Completo**: Tiempo total del algoritmo

### Optimizaciones Identificadas

1. **Tamaño de datos**: El rendimiento se ve afectado por el tamaño de los datos transmitidos
2. **Número de procesos**: La escalabilidad depende del número de procesos utilizados
3. **Overhead de comunicación**: Las operaciones colectivas tienen overhead que aumenta con el número de procesos

### Recomendaciones

1. **Tamaño óptimo de datos**: Para mejor rendimiento, usar tamaños de datos que equilibren computación y comunicación
2. **Número de procesos**: Evaluar el punto de saturación donde agregar más procesos no mejora el rendimiento
3. **Optimización de red**: En clusters, considerar la topología de red para optimizar comunicaciones

## Archivos de Resultados

EOF

    # Listar archivos generados
    for file in "$OUTPUT_DIR"/*; do
        if [[ -f "$file" ]]; then
            echo "- \`$(basename "$file")\`" >> "$report_file"
        fi
    done

    print_success "Reporte generado en: $report_file"
}

# Configuraciones de benchmark
TEST_CONFIGS=(2 4 8 16)

# Contadores
TOTAL_BENCHMARKS=0
PASSED_BENCHMARKS=0
FAILED_BENCHMARKS=0

# Ejecutar benchmarks
if [[ "$RUN_ALL" == true ]]; then
    print_info "Ejecutando todas las configuraciones de benchmark..."
    
    for num_procs in "${TEST_CONFIGS[@]}"; do
        TOTAL_BENCHMARKS=$((TOTAL_BENCHMARKS + 1))
        
        if run_benchmark_with_timeout $num_procs "Benchmark con $num_procs procesos"; then
            PASSED_BENCHMARKS=$((PASSED_BENCHMARKS + 1))
        else
            FAILED_BENCHMARKS=$((FAILED_BENCHMARKS + 1))
        fi
        
        # Pausa entre benchmarks
        sleep 5
    done
else
    TOTAL_BENCHMARKS=1
    if run_benchmark_with_timeout $NUM_PROCESSES "Benchmark con $NUM_PROCESSES procesos"; then
        PASSED_BENCHMARKS=1
    else
        FAILED_BENCHMARKS=1
    fi
fi

# Generar reporte
if [[ $PASSED_BENCHMARKS -gt 0 ]]; then
    generate_benchmark_report
fi

# Mostrar resumen
print_info ""
print_info "=== RESUMEN DE BENCHMARKS ==="
print_info "Total de benchmarks: $TOTAL_BENCHMARKS"
print_info "Benchmarks exitosos: $PASSED_BENCHMARKS"
print_info "Benchmarks fallidos: $FAILED_BENCHMARKS"
print_info "Resultados guardados en: $OUTPUT_DIR"

if [[ $FAILED_BENCHMARKS -eq 0 ]]; then
    print_success "¡Todos los benchmarks se completaron exitosamente!"
    print_info ""
    print_info "Para analizar los resultados:"
    print_info "  - Revisar archivos CSV en $OUTPUT_DIR"
    print_info "  - Ver reporte generado: $OUTPUT_DIR/benchmark_report_*.md"
    print_info "  - Usar herramientas como gnuplot o matplotlib para visualizar datos"
    exit 0
else
    print_error "Algunos benchmarks fallaron. Revisar los logs para más detalles."
    exit 1
fi 