// ============================================================================
// MÓDULO BOTOES - CABEÇALHO (HEADER)
// ============================================================================
// Define a estrutura de dados e a interface para leitura não-bloqueante de
// botões/entradas digitais com tratamento de debounce mecânico.

#ifndef MODULO_BOTOES_H
#define MODULO_BOTOES_H

#include <Arduino.h>
#include "config.h"

// Estrutura de controle individual para cada botão
typedef struct {
    int pino;                          // Número do pino GPIO conectado ao botão
    int ultimoEstado;                  // Guarda a última leitura bruta do pino
    bool pressionado;                  // Estado lógico filtrado (true = pressionado)
    unsigned long ultimoTempoDebounce; // Cronômetro em ms para o filtro mecânico
    unsigned long tempoDebounce;       // Tempo de estabilização em ms (ex: 50ms)
} Botao;

/**
 * @brief Inicializa o pino GPIO do botão com resistor Pull-Up interno.
 * @param b Ponteiro para a estrutura Botao.
 * @param pinoGpio Número da GPIO do ESP32 (ex: PINO_BOTAO_AZUL).
 */
void botao_inicializar(Botao *b, int pinoGpio);

/**
 * @brief Verifica se o botão sofreu um clique (transição de solto para pressionado).
 * @param b Ponteiro para a estrutura Botao a ser verificada.
 * @return  1 : Sim clique (Botão foi pressionado)
 *          0 : Não clique (Botão solto ou sem alteração)
 *         -1 : Algum erro (Ponteiro de botão nulo)
 */
int botao_verificar_clique(Botao *b);

#endif // MODULO_BOTOES_H