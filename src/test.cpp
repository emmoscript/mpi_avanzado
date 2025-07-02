/**
 * @file test.cpp
 * @brief Programa de pruebas para validar la corrección de las operaciones colectivas
 * @author Emil M
 * @date 2025
 * 
 * Este programa realiza pruebas unitarias para:
 * - Validar que MPI_Bcast funciona correctamente
 * - Validar que MPI_Reduce funciona correctamente
 * - Validar que el cálculo del promedio es correcto
 * - Verificar la sincronización entre procesos
 */

#include <mpi.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <cassert>

/**
 * @brief Prueba la funcionalidad de MPI_Bcast
 * @param rank Rank del proceso actual
 * @param numProcs Número total de procesos
 * @return true si la prueba pasa, false en caso contrario
 */
bool testBroadcast(int rank, int numProcs) {
    std::cout << "Proceso " << rank << ": Ejecutando prueba de MPI_Bcast..." << std::endl;
    
    int valorOriginal = 42;
    int valorRecibido = 0;
    
    // El proceso raíz inicializa el valor
    if (rank == 0) {
        valorRecibido = valorOriginal;
    }
    
    // Broadcast del valor
    MPI_Bcast(&valorRecibido, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Verificar que todos los procesos recibieron el valor correcto
    bool resultado = (valorRecibido == valorOriginal);
    
    std::cout << "Proceso " << rank << ": Valor recibido = " << valorRecibido 
              << " (esperado = " << valorOriginal << ")" << std::endl;
    
    return resultado;
}

/**
 * @brief Prueba la funcionalidad de MPI_Reduce
 * @param rank Rank del proceso actual
 * @param numProcs Número total de procesos
 * @return true si la prueba pasa, false en caso contrario
 */
bool testReduce(int rank, int numProcs) {
    std::cout << "Proceso " << rank << ": Ejecutando prueba de MPI_Reduce..." << std::endl;
    
    double valorLocal = static_cast<double>(rank + 1);
    double sumaTotal = 0.0;
    
    // Calcular la suma esperada
    double sumaEsperada = 0.0;
    for (int i = 0; i < numProcs; ++i) {
        sumaEsperada += (i + 1);
    }
    
    // Reduce para sumar todos los valores
    MPI_Reduce(&valorLocal, &sumaTotal, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    bool resultado = true;
    if (rank == 0) {
        resultado = (std::abs(sumaTotal - sumaEsperada) < 1e-10);
        std::cout << "Proceso " << rank << ": Suma total = " << sumaTotal 
                  << " (esperada = " << sumaEsperada << ")" << std::endl;
    }
    
    return resultado;
}

/**
 * @brief Prueba el cálculo del promedio con valores conocidos
 * @param rank Rank del proceso actual
 * @param numProcs Número total de procesos
 * @return true si la prueba pasa, false en caso contrario
 */
bool testPromedio(int rank, int numProcs) {
    std::cout << "Proceso " << rank << ": Ejecutando prueba de cálculo de promedio..." << std::endl;
    
    // Usar valores conocidos para facilitar la verificación
    std::vector<double> valores = {1.0, 2.0, 3.0, 4.0, 5.0};
    int N = valores.size();
    
    // Calcular suma parcial
    double sumaParcial = 0.0;
    for (double valor : valores) {
        sumaParcial += valor;
    }
    
    // Ajustar la suma parcial según el rank para simular diferentes procesos
    sumaParcial += rank * N * 10.0;
    
    double sumaTotal = 0.0;
    double promedioFinal = 0.0;
    
    // MPI_Reduce
    MPI_Reduce(&sumaParcial, &sumaTotal, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // Calcular promedio en proceso raíz
    if (rank == 0) {
        promedioFinal = sumaTotal / (N * numProcs);
        
        // Calcular el promedio esperado
        double sumaEsperada = 0.0;
        for (int i = 0; i < numProcs; ++i) {
            double sumaLocal = 0.0;
            for (double valor : valores) {
                sumaLocal += valor;
            }
            sumaLocal += i * N * 10.0;
            sumaEsperada += sumaLocal;
        }
        double promedioEsperado = sumaEsperada / (N * numProcs);
        
        bool resultado = (std::abs(promedioFinal - promedioEsperado) < 1e-10);
        std::cout << "Proceso " << rank << ": Promedio calculado = " << promedioFinal 
                  << " (esperado = " << promedioEsperado << ")" << std::endl;
        
        return resultado;
    }
    
    return true;
}

/**
 * @brief Prueba la sincronización entre procesos
 * @param rank Rank del proceso actual
 * @param numProcs Número total de procesos
 * @return true si la prueba pasa, false en caso contrario
 */
bool testSincronizacion(int rank, int numProcs) {
    std::cout << "Proceso " << rank << ": Ejecutando prueba de sincronización..." << std::endl;
    
    // Simular trabajo de diferentes duraciones
    int trabajo = rank + 1;
    for (int i = 0; i < trabajo * 1000; ++i) {
        // Trabajo simulado
        volatile double x = std::sqrt(static_cast<double>(i));
    }
    
    // Punto de sincronización
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Si llegamos aquí, la sincronización funcionó
    std::cout << "Proceso " << rank << ": Sincronización completada exitosamente" << std::endl;
    
    return true;
}

/**
 * @brief Prueba completa del programa principal con valores aleatorios
 * @param rank Rank del proceso actual
 * @param numProcs Número total de procesos
 * @return true si la prueba pasa, false en caso contrario
 */
bool testProgramaCompleto(int rank, int numProcs) {
    std::cout << "Proceso " << rank << ": Ejecutando prueba completa del programa..." << std::endl;
    
    int N = 100;
    std::vector<double> valores(N);
    double sumaParcial = 0.0;
    double sumaTotal = 0.0;
    double promedioFinal = 0.0;
    
    // Generar valores aleatorios con semilla fija para reproducibilidad
    std::mt19937 generador(rank + 42);
    std::uniform_real_distribution<double> distribucion(0.0, 100.0);
    
    for (int i = 0; i < N; ++i) {
        valores[i] = distribucion(generador);
        sumaParcial += valores[i];
    }
    
    // MPI_Reduce
    MPI_Reduce(&sumaParcial, &sumaTotal, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // Calcular promedio en proceso raíz
    if (rank == 0) {
        promedioFinal = sumaTotal / (N * numProcs);
        
        // Verificar que el promedio está en un rango razonable (0-100)
        bool resultado = (promedioFinal >= 0.0 && promedioFinal <= 100.0);
        std::cout << "Proceso " << rank << ": Promedio final = " << promedioFinal 
                  << " (rango válido: 0-100)" << std::endl;
        
        return resultado;
    }
    
    return true;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    
    if (rank == 0) {
        std::cout << "=== PRUEBAS DE COMUNICACIONES COLECTIVAS MPI ===" << std::endl;
        std::cout << "Número de procesos: " << numProcs << std::endl;
        std::cout << std::endl;
    }
    
    // Sincronizar antes de comenzar las pruebas
    MPI_Barrier(MPI_COMM_WORLD);
    
    bool todasLasPruebasPasan = true;
    
    // Ejecutar todas las pruebas
    todasLasPruebasPasan &= testBroadcast(rank, numProcs);
    MPI_Barrier(MPI_COMM_WORLD);
    
    todasLasPruebasPasan &= testReduce(rank, numProcs);
    MPI_Barrier(MPI_COMM_WORLD);
    
    todasLasPruebasPasan &= testPromedio(rank, numProcs);
    MPI_Barrier(MPI_COMM_WORLD);
    
    todasLasPruebasPasan &= testSincronizacion(rank, numProcs);
    MPI_Barrier(MPI_COMM_WORLD);
    
    todasLasPruebasPasan &= testProgramaCompleto(rank, numProcs);
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Verificar resultados globales
    int resultadoGlobal = todasLasPruebasPasan ? 1 : 0;
    int resultadoTotal = 0;
    
    MPI_Reduce(&resultadoGlobal, &resultadoTotal, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // Sincronizar antes de imprimir resultados finales
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (rank == 0) {
        std::cout << std::endl << "=== RESULTADOS DE LAS PRUEBAS ===" << std::endl;
        if (resultadoTotal == numProcs) {
            std::cout << "✅ TODAS LAS PRUEBAS PASARON EXITOSAMENTE" << std::endl;
        } else {
            std::cout << "❌ ALGUNAS PRUEBAS FALLARON" << std::endl;
            std::cout << "Procesos que pasaron las pruebas: " << resultadoTotal << "/" << numProcs << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Sincronizar antes de finalizar
    MPI_Barrier(MPI_COMM_WORLD);
    
    MPI_Finalize();
    
    return (resultadoTotal == numProcs) ? 0 : 1;
} 