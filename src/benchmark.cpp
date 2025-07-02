/**
 * @file benchmark.cpp
 * @brief Programa de benchmark para medir el rendimiento de las comunicaciones colectivas
 * @author Emil M
 * @date 2025
 * 
 * Este programa realiza mediciones de rendimiento para:
 * - MPI_Bcast con diferentes tamaños de datos
 * - MPI_Reduce con diferentes tamaños de datos
 * - Escalabilidad con diferentes números de procesos
 */

#include <mpi.h>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <string>

/**
 * @brief Ejecuta un benchmark de MPI_Bcast
 * @param dataSize Tamaño de los datos a transmitir
 * @param numIterations Número de iteraciones para el benchmark
 * @param rank Rank del proceso actual
 * @param numProcs Número total de procesos
 * @return Tiempo promedio en microsegundos
 */
double benchmarkBroadcast(int dataSize, int numIterations, int rank, int numProcs) {
    std::vector<double> data(dataSize);
    
    // Inicializar datos en el proceso raíz
    if (rank == 0) {
        std::mt19937 gen(42);
        std::uniform_real_distribution<double> dist(0.0, 100.0);
        for (int i = 0; i < dataSize; ++i) {
            data[i] = dist(gen);
        }
    }
    
    // Sincronizar antes del benchmark
    MPI_Barrier(MPI_COMM_WORLD);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < numIterations; ++iter) {
        MPI_Bcast(data.data(), dataSize, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    return static_cast<double>(duration.count()) / numIterations;
}

/**
 * @brief Ejecuta un benchmark de MPI_Reduce
 * @param dataSize Tamaño de los datos a reducir
 * @param numIterations Número de iteraciones para el benchmark
 * @param rank Rank del proceso actual
 * @param numProcs Número total de procesos
 * @return Tiempo promedio en microsegundos
 */
double benchmarkReduce(int dataSize, int numIterations, int rank, int numProcs) {
    std::vector<double> localData(dataSize);
    std::vector<double> globalData(dataSize);
    
    // Inicializar datos locales
    std::mt19937 gen(rank + 42);
    std::uniform_real_distribution<double> dist(0.0, 100.0);
    for (int i = 0; i < dataSize; ++i) {
        localData[i] = dist(gen);
    }
    
    // Sincronizar antes del benchmark
    MPI_Barrier(MPI_COMM_WORLD);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < numIterations; ++iter) {
        MPI_Reduce(localData.data(), globalData.data(), dataSize, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    return static_cast<double>(duration.count()) / numIterations;
}

/**
 * @brief Ejecuta un benchmark completo del programa principal
 * @param N Número de valores por proceso
 * @param numIterations Número de iteraciones para el benchmark
 * @param rank Rank del proceso actual
 * @param numProcs Número total de procesos
 * @return Tiempo promedio en microsegundos
 */
double benchmarkCompleto(int N, int numIterations, int rank, int numProcs) {
    std::vector<double> valores(N);
    double sumaParcial = 0.0;
    double sumaTotal = 0.0;
    double promedioFinal = 0.0;
    
    // Sincronizar antes del benchmark
    MPI_Barrier(MPI_COMM_WORLD);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int iter = 0; iter < numIterations; ++iter) {
        // Generar valores aleatorios
        std::mt19937 generador(rank + 42 + iter);
        std::uniform_real_distribution<double> distribucion(0.0, 100.0);
        
        for (int i = 0; i < N; ++i) {
            valores[i] = distribucion(generador);
        }
        
        // Calcular suma parcial
        sumaParcial = 0.0;
        for (double valor : valores) {
            sumaParcial += valor;
        }
        
        // MPI_Reduce
        MPI_Reduce(&sumaParcial, &sumaTotal, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        
        // Calcular promedio en proceso raíz
        if (rank == 0) {
            promedioFinal = sumaTotal / (N * numProcs);
        }
        
        // MPI_Bcast del promedio
        MPI_Bcast(&promedioFinal, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    return static_cast<double>(duration.count()) / numIterations;
}

/**
 * @brief Guarda los resultados del benchmark en un archivo CSV
 * @param filename Nombre del archivo
 * @param results Vector de resultados
 */
void guardarResultados(const std::string& filename, const std::vector<std::string>& results) {
    if (MPI::COMM_WORLD.Get_rank() == 0) {
        std::ofstream file(filename);
        if (file.is_open()) {
            for (const auto& result : results) {
                file << result << std::endl;
            }
            file.close();
            std::cout << "Resultados guardados en: " << filename << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    
    if (rank == 0) {
        std::cout << "=== BENCHMARK DE COMUNICACIONES COLECTIVAS MPI ===" << std::endl;
        std::cout << "Número de procesos: " << numProcs << std::endl;
        std::cout << std::endl;
    }
    
    std::vector<std::string> resultados;
    resultados.push_back("Operacion,TamañoDatos,NumProcesos,TiempoPromedio(microsegundos)");
    
    // Configuración del benchmark
    std::vector<int> dataSizes = {1, 10, 100, 1000, 10000};
    std::vector<int> NValues = {100, 1000, 10000};
    int numIterations = 100;
    
    // Benchmark de MPI_Bcast
    if (rank == 0) {
        std::cout << "Ejecutando benchmark de MPI_Bcast..." << std::endl;
    }
    
    for (int dataSize : dataSizes) {
        double tiempoPromedio = benchmarkBroadcast(dataSize, numIterations, rank, numProcs);
        
        if (rank == 0) {
            std::string resultado = "MPI_Bcast," + std::to_string(dataSize) + "," + 
                                   std::to_string(numProcs) + "," + 
                                   std::to_string(tiempoPromedio);
            resultados.push_back(resultado);
            
            std::cout << "  MPI_Bcast con " << dataSize << " elementos: " 
                      << std::fixed << std::setprecision(2) << tiempoPromedio 
                      << " microsegundos" << std::endl;
        }
    }
    
    // Benchmark de MPI_Reduce
    if (rank == 0) {
        std::cout << std::endl << "Ejecutando benchmark de MPI_Reduce..." << std::endl;
    }
    
    for (int dataSize : dataSizes) {
        double tiempoPromedio = benchmarkReduce(dataSize, numIterations, rank, numProcs);
        
        if (rank == 0) {
            std::string resultado = "MPI_Reduce," + std::to_string(dataSize) + "," + 
                                   std::to_string(numProcs) + "," + 
                                   std::to_string(tiempoPromedio);
            resultados.push_back(resultado);
            
            std::cout << "  MPI_Reduce con " << dataSize << " elementos: " 
                      << std::fixed << std::setprecision(2) << tiempoPromedio 
                      << " microsegundos" << std::endl;
        }
    }
    
    // Benchmark del programa completo
    if (rank == 0) {
        std::cout << std::endl << "Ejecutando benchmark del programa completo..." << std::endl;
    }
    
    for (int N : NValues) {
        double tiempoPromedio = benchmarkCompleto(N, numIterations, rank, numProcs);
        
        if (rank == 0) {
            std::string resultado = "ProgramaCompleto," + std::to_string(N) + "," + 
                                   std::to_string(numProcs) + "," + 
                                   std::to_string(tiempoPromedio);
            resultados.push_back(resultado);
            
            std::cout << "  Programa completo con N=" << N << ": " 
                      << std::fixed << std::setprecision(2) << tiempoPromedio 
                      << " microsegundos" << std::endl;
        }
    }
    
    // Guardar resultados
    std::string filename = "benchmark_results_" + std::to_string(numProcs) + "procs.csv";
    guardarResultados(filename, resultados);
    
    if (rank == 0) {
        std::cout << std::endl << "=== BENCHMARK COMPLETADO ===" << std::endl;
        std::cout << "Los resultados han sido guardados en: " << filename << std::endl;
    }
    
    MPI_Finalize();
    return 0;
} 