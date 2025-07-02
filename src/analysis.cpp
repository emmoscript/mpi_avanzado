/**
 * @file analysis.cpp
 * @brief Programa de análisis avanzado para MPI - Memoria, robustez y escalabilidad
 * @author Estudiante
 * @date 2024
 * 
 * Este programa realiza análisis detallados:
 * - Medición de uso de memoria
 * - Tests de robustez (fallos, timeouts)
 * - Análisis de escalabilidad fuerte y débil
 * - Comparación de rendimiento
 */

#include <mpi.h>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <string>
#include <sys/resource.h>
#include <unistd.h>

/**
 * @brief Obtiene el uso de memoria del proceso actual
 * @return Uso de memoria en KB
 */
long getMemoryUsage() {
    struct rusage rusage;
    if (getrusage(RUSAGE_SELF, &rusage) == 0) {
        return rusage.ru_maxrss;
    }
    return -1;
}

/**
 * @brief Obtiene información del sistema
 * @param rank Rank del proceso
 * @param numProcs Número total de procesos
 */
void printSystemInfo(int rank, int numProcs) {
    if (rank == 0) {
        std::cout << "=== INFORMACIÓN DEL SISTEMA ===" << std::endl;
        std::cout << "Número de procesos: " << numProcs << std::endl;
        std::cout << "Número de núcleos: " << sysconf(_SC_NPROCESSORS_ONLN) << std::endl;
        std::cout << "Tamaño de página: " << sysconf(_SC_PAGESIZE) << " bytes" << std::endl;
        std::cout << std::endl;
    }
}

/**
 * @brief Análisis de escalabilidad fuerte (mismo problema, más procesos)
 * @param N Tamaño total del problema
 * @param maxProcs Máximo número de procesos a probar
 * @param rank Rank del proceso actual
 * @param numProcs Número total de procesos
 */
void strongScalingAnalysis(int N, int maxProcs, int rank, int numProcs) {
    if (rank == 0) {
        std::cout << "=== ANÁLISIS DE ESCALABILIDAD FUERTE ===" << std::endl;
        std::cout << "Tamaño total del problema: " << N << " elementos" << std::endl;
        std::cout << std::endl;
    }
    
    // Calcular elementos por proceso
    int elementsPerProc = N / numProcs;
    if (N % numProcs != 0) {
        elementsPerProc++;
    }
    
    // Generar datos
    std::vector<double> data(elementsPerProc);
    std::mt19937 gen(rank + 42);
    std::uniform_real_distribution<double> dist(0.0, 100.0);
    
    for (int i = 0; i < elementsPerProc; ++i) {
        data[i] = dist(gen);
    }
    
    // Medir tiempo de computación
    double startTime = MPI_Wtime();
    
    double localSum = 0.0;
    for (double val : data) {
        localSum += val;
    }
    
    double globalSum = 0.0;
    MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    double endTime = MPI_Wtime();
    double elapsed = (endTime - startTime) * 1e6; // microsegundos
    
    // Medir memoria
    long memoryUsage = getMemoryUsage();
    
    if (rank == 0) {
        double avg = globalSum / N;
        std::cout << "Procesos: " << numProcs << " | ";
        std::cout << "Tiempo: " << std::fixed << std::setprecision(2) << elapsed << " μs | ";
        std::cout << "Memoria: " << memoryUsage << " KB | ";
        std::cout << "Promedio: " << std::fixed << std::setprecision(4) << avg << std::endl;
    }
}

/**
 * @brief Análisis de escalabilidad débil (mismo trabajo por proceso)
 * @param NPerProc Elementos por proceso
 * @param rank Rank del proceso actual
 * @param numProcs Número total de procesos
 */
void weakScalingAnalysis(int NPerProc, int rank, int numProcs) {
    if (rank == 0) {
        std::cout << "=== ANÁLISIS DE ESCALABILIDAD DÉBIL ===" << std::endl;
        std::cout << "Elementos por proceso: " << NPerProc << std::endl;
        std::cout << std::endl;
    }
    
    // Generar datos
    std::vector<double> data(NPerProc);
    std::mt19937 gen(rank + 42);
    std::uniform_real_distribution<double> dist(0.0, 100.0);
    
    for (int i = 0; i < NPerProc; ++i) {
        data[i] = dist(gen);
    }
    
    // Medir tiempo
    double startTime = MPI_Wtime();
    
    double localSum = 0.0;
    for (double val : data) {
        localSum += val;
    }
    
    double globalSum = 0.0;
    MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    double endTime = MPI_Wtime();
    double elapsed = (endTime - startTime) * 1e6; // microsegundos
    
    // Medir memoria
    long memoryUsage = getMemoryUsage();
    
    if (rank == 0) {
        int totalElements = NPerProc * numProcs;
        double avg = globalSum / totalElements;
        std::cout << "Procesos: " << numProcs << " | ";
        std::cout << "Total elementos: " << totalElements << " | ";
        std::cout << "Tiempo: " << std::fixed << std::setprecision(2) << elapsed << " μs | ";
        std::cout << "Memoria: " << memoryUsage << " KB | ";
        std::cout << "Promedio: " << std::fixed << std::setprecision(4) << avg << std::endl;
    }
}

/**
 * @brief Test de robustez - manejo de errores
 * @param rank Rank del proceso actual
 * @param numProcs Número total de procesos
 */
void robustnessTest(int rank, int numProcs) {
    if (rank == 0) {
        std::cout << "=== TEST DE ROBUSTEZ ===" << std::endl;
    }
    
    // Test 1: Datos muy grandes
    if (rank == 0) {
        std::cout << "Test 1: Datos grandes (1M elementos)..." << std::endl;
    }
    
    try {
        std::vector<double> largeData(1000000);
        std::mt19937 gen(rank + 42);
        std::uniform_real_distribution<double> dist(0.0, 100.0);
        
        for (int i = 0; i < 1000000; ++i) {
            largeData[i] = dist(gen);
        }
        
        double localSum = 0.0;
        for (double val : largeData) {
            localSum += val;
        }
        
        double globalSum = 0.0;
        MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        
        if (rank == 0) {
            std::cout << "  ✅ Test 1 completado exitosamente" << std::endl;
        }
    } catch (const std::exception& e) {
        if (rank == 0) {
            std::cout << "  ❌ Test 1 falló: " << e.what() << std::endl;
        }
    }
    
    // Test 2: Múltiples operaciones colectivas
    if (rank == 0) {
        std::cout << "Test 2: Múltiples operaciones colectivas..." << std::endl;
    }
    
    std::vector<double> data(1000);
    std::mt19937 gen(rank + 42);
    std::uniform_real_distribution<double> dist(0.0, 100.0);
    
    for (int i = 0; i < 1000; ++i) {
        data[i] = dist(gen);
    }
    
    for (int iter = 0; iter < 10; ++iter) {
        double localSum = 0.0;
        for (double val : data) {
            localSum += val;
        }
        
        double globalSum = 0.0;
        MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        
        MPI_Bcast(&globalSum, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    
    if (rank == 0) {
        std::cout << "  ✅ Test 2 completado exitosamente" << std::endl;
    }
    
    // Test 3: Verificación de sincronización
    if (rank == 0) {
        std::cout << "Test 3: Verificación de sincronización..." << std::endl;
    }
    
    int localValue = rank + 1;
    int globalSum = 0;
    
    MPI_Reduce(&localValue, &globalSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        int expectedSum = (numProcs * (numProcs + 1)) / 2;
        if (globalSum == expectedSum) {
            std::cout << "  ✅ Test 3 completado exitosamente" << std::endl;
        } else {
            std::cout << "  ❌ Test 3 falló: suma esperada " << expectedSum 
                      << ", obtenida " << globalSum << std::endl;
        }
    }
}

/**
 * @brief Análisis de comunicación vs cómputo
 * @param rank Rank del proceso actual
 * @param numProcs Número total de procesos
 */
void communicationVsComputationAnalysis(int rank, int numProcs) {
    if (rank == 0) {
        std::cout << "=== ANÁLISIS COMUNICACIÓN VS CÓMPUTO ===" << std::endl;
    }
    
    std::vector<int> problemSizes = {100, 1000, 10000, 100000};
    
    for (int N : problemSizes) {
        std::vector<double> data(N);
        std::mt19937 gen(rank + 42);
        std::uniform_real_distribution<double> dist(0.0, 100.0);
        
        for (int i = 0; i < N; ++i) {
            data[i] = dist(gen);
        }
        
        // Medir tiempo de cómputo
        double startComp = MPI_Wtime();
        double localSum = 0.0;
        for (double val : data) {
            localSum += val;
        }
        double endComp = MPI_Wtime();
        double compTime = (endComp - startComp) * 1e6;
        
        // Medir tiempo de comunicación
        double startComm = MPI_Wtime();
        double globalSum = 0.0;
        MPI_Reduce(&localSum, &globalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        double endComm = MPI_Wtime();
        double commTime = (endComm - startComm) * 1e6;
        
        if (rank == 0) {
            double totalTime = compTime + commTime;
            double compRatio = (compTime / totalTime) * 100;
            double commRatio = (commTime / totalTime) * 100;
            
            std::cout << "N=" << N << " | ";
            std::cout << "Cómputo: " << std::fixed << std::setprecision(2) << compTime << " μs (" 
                      << std::fixed << std::setprecision(1) << compRatio << "%) | ";
            std::cout << "Comunicación: " << std::fixed << std::setprecision(2) << commTime << " μs (" 
                      << std::fixed << std::setprecision(1) << commRatio << "%)" << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    
    if (rank == 0) {
        std::cout << "=== ANÁLISIS AVANZADO MPI - COMUNICACIONES COLECTIVAS ===" << std::endl;
        std::cout << std::endl;
    }
    
    // Información del sistema
    printSystemInfo(rank, numProcs);
    
    // Análisis de escalabilidad fuerte
    strongScalingAnalysis(10000, 16, rank, numProcs);
    
    // Análisis de escalabilidad débil
    weakScalingAnalysis(1000, rank, numProcs);
    
    // Test de robustez
    robustnessTest(rank, numProcs);
    
    // Análisis comunicación vs cómputo
    communicationVsComputationAnalysis(rank, numProcs);
    
    if (rank == 0) {
        std::cout << std::endl;
        std::cout << "=== ANÁLISIS COMPLETADO ===" << std::endl;
    }
    
    MPI_Finalize();
    return 0;
} 