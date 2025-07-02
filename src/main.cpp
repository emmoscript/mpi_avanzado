/**
 * @file main.cpp
 * @brief Programa MPI para calcular el promedio de valores aleatorios usando comunicaciones colectivas
 * @author Emil M
 * @date 2025
 * 
 * Este programa demuestra el uso de las siguientes operaciones colectivas de MPI:
 * - MPI_Bcast: Para distribuir el tamaño N a todos los procesos
 * - MPI_Reduce: Para sumar todas las contribuciones parciales
 * - MPI_Bcast: Para distribuir el promedio final a todos los procesos
 */

#include <mpi.h>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>

/**
 * @brief Genera N valores aleatorios y calcula su suma
 * @param N Número de valores a generar
 * @param rank Rank del proceso actual
 * @return Vector con los valores generados
 */
std::vector<double> generarValoresAleatorios(int N, int rank) {
    std::vector<double> valores(N);
    
    // Usar una semilla diferente para cada proceso para evitar correlación
    std::mt19937 generador(rank + 42);
    std::uniform_real_distribution<double> distribucion(0.0, 100.0);
    
    for (int i = 0; i < N; ++i) {
        valores[i] = distribucion(generador);
    }
    
    return valores;
}

/**
 * @brief Calcula la suma de los valores en un vector
 * @param valores Vector de valores
 * @return Suma de todos los valores
 */
double calcularSumaParcial(const std::vector<double>& valores) {
    double suma = 0.0;
    for (double valor : valores) {
        suma += valor;
    }
    return suma;
}

/**
 * @brief Imprime información del proceso
 * @param rank Rank del proceso
 * @param valores Vector de valores generados
 * @param sumaParcial Suma parcial calculada
 */
void imprimirInfoProceso(int rank, const std::vector<double>& valores, double sumaParcial) {
    std::cout << "Proceso " << rank << ":" << std::endl;
    std::cout << "  - Valores generados: ";
    
    // Mostrar solo los primeros 5 valores para no saturar la salida
    int maxMostrar = std::min(5, static_cast<int>(valores.size()));
    for (int i = 0; i < maxMostrar; ++i) {
        std::cout << std::fixed << std::setprecision(2) << valores[i];
        if (i < maxMostrar - 1) std::cout << ", ";
    }
    if (valores.size() > maxMostrar) {
        std::cout << ", ... (y " << (valores.size() - maxMostrar) << " más)";
    }
    std::cout << std::endl;
    std::cout << "  - Suma parcial: " << std::fixed << std::setprecision(2) << sumaParcial << std::endl;
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    // Inicialización MPI
    MPI_Init(&argc, &argv);
    
    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    
    // Variables para el cálculo
    int N = 0;
    double sumaParcial = 0.0;
    double sumaTotal = 0.0;
    double promedioFinal = 0.0;
    
    // Punto de sincronización 1: Todos los procesos esperan aquí
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Paso 1: El proceso raíz solicita N al usuario y lo distribuye con MPI_Bcast
    if (rank == 0) {
        std::cout << "=== PROGRAMA MPI: CÁLCULO DE PROMEDIO CON COMUNICACIONES COLECTIVAS ===" << std::endl;
        std::cout << "Número total de procesos: " << numProcs << std::endl;
        std::cout << "Ingrese el número de valores por proceso (N): ";
        std::cin >> N;
        
        if (N <= 0) {
            std::cerr << "Error: N debe ser un número positivo." << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        std::cout << "Proceso raíz (rank 0) distribuyendo N = " << N << " a todos los procesos..." << std::endl;
    }
    
    // MPI_Bcast: Distribuye N desde el proceso raíz a todos los procesos
    // Punto de sincronización 2: Todos los procesos deben llegar aquí antes de continuar
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Paso 2: Cada proceso genera N valores aleatorios y calcula su suma parcial
    double inicioGeneracion = MPI_Wtime();
    
    std::vector<double> valores = generarValoresAleatorios(N, rank);
    sumaParcial = calcularSumaParcial(valores);
    
    double finGeneracion = MPI_Wtime();
    double duracionGeneracion = (finGeneracion - inicioGeneracion) * 1e6; // microsegundos
    
    // Imprimir información de cada proceso en orden
    for (int i = 0; i < numProcs; ++i) {
        if (rank == i) {
            imprimirInfoProceso(rank, valores, sumaParcial);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    // Punto de sincronización 3: Todos los procesos esperan aquí antes de la reducción
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Paso 3: MPI_Reduce para sumar todas las contribuciones parciales en el proceso raíz
    double inicioReduccion = MPI_Wtime();
    
    // MPI_Reduce: Suma todas las contribuciones parciales en el proceso raíz
    // Punto de sincronización 4: Todos los procesos deben participar en la reducción
    MPI_Reduce(&sumaParcial, &sumaTotal, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    double finReduccion = MPI_Wtime();
    double duracionReduccion = (finReduccion - inicioReduccion) * 1e6; // microsegundos
    
    // Paso 4: El proceso raíz calcula el promedio total
    if (rank == 0) {
        promedioFinal = sumaTotal / (N * numProcs);
        std::cout << "=== RESULTADOS EN EL PROCESO RAÍZ ===" << std::endl;
        std::cout << "Suma total de todos los procesos: " << std::fixed << std::setprecision(2) << sumaTotal << std::endl;
        std::cout << "Número total de valores: " << (N * numProcs) << std::endl;
        std::cout << "Promedio calculado: " << std::fixed << std::setprecision(4) << promedioFinal << std::endl;
        std::cout << "Tiempo de reducción: " << std::fixed << std::setprecision(2) << duracionReduccion << " microsegundos" << std::endl;
        std::cout << std::endl;
    }
    
    // Paso 5: MPI_Bcast para distribuir el promedio a todos los procesos
    double inicioBroadcast = MPI_Wtime();
    
    // MPI_Bcast: Distribuye el promedio desde el proceso raíz a todos los procesos
    // Punto de sincronización 5: Todos los procesos deben participar en el broadcast
    MPI_Bcast(&promedioFinal, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    double finBroadcast = MPI_Wtime();
    double duracionBroadcast = (finBroadcast - inicioBroadcast) * 1e6; // microsegundos
    
    // Paso 6: Cada proceso imprime el promedio recibido en orden
    for (int i = 0; i < numProcs; ++i) {
        if (rank == i) {
            std::cout << "Proceso " << rank << " recibió el promedio final: " 
                      << std::fixed << std::setprecision(4) << promedioFinal << std::endl;
            std::cout << "Tiempo de broadcast: " << std::fixed << std::setprecision(2) 
                      << duracionBroadcast << " microsegundos" << std::endl;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    // Punto de sincronización final: Todos los procesos esperan aquí antes de finalizar
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (rank == 0) {
        std::cout << std::endl;
        std::cout << "=== RESUMEN DE TIEMPOS ===" << std::endl;
        std::cout << "Tiempo de generación de datos: " << std::fixed << std::setprecision(2) << duracionGeneracion << " microsegundos" << std::endl;
        std::cout << "Tiempo de reducción: " << std::fixed << std::setprecision(2) << duracionReduccion << " microsegundos" << std::endl;
        std::cout << "Tiempo de broadcast final: " << std::fixed << std::setprecision(2) << duracionBroadcast << " microsegundos" << std::endl;
        std::cout << std::endl;
        std::cout << "=== PROGRAMA COMPLETADO EXITOSAMENTE ===" << std::endl;
    }
    
    // Finalización MPI
    MPI_Finalize();
    
    return 0;
} 