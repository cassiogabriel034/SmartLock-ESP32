#ifndef MODULO_PORTA_H
#define MODULO_PORTA_H

#include <Arduino.h>
#include "config.h"

// Mapeamento lógico do sensor Fim de Curso
#define FIM_DE_CURSO_PRESSIONADO LOW   // 0: Porta Fechada
#define FIM_DE_CURSO_SOLTO       HIGH  // 1: Porta Aberta

/**
 * @brief Configura os pinos do relé da trava e do sensor fim de curso.
 */
void inicializarPorta();

/**
 * @brief Aciona o relé (HIGH) para destravar a solenoide.
 */
void abrirPorta();

/**
 * @brief Desliga o relé (LOW) para fechar o circuito da trava.
 */
void fecharPorta();

/**
 * @brief Verifica se a porta está fisicamente fechada (sensor pressionado).
 * @return true se a porta estiver fechada (LOW), false se estiver aberta (HIGH).
 */
bool estaPortaFechada();

/**
 * @brief Retorna o estado atual do relé de acionamento da trava.
 */
bool obterEstadoTrava();

#endif // MODULO_PORTA_H