// ============================================================================
// MÓDULO BLUETOOTH - CABEÇALHO (HEADER)
// ============================================================================
// Define a interface pública para o gerenciamento da comunicação Bluetooth Serial
// (SPP) durante o modo de setup do dispositivo.

#ifndef MODULO_BLUETOOTH_H
#define MODULO_BLUETOOTH_H

#include "config_structs.h"

/**
 * @brief Inicializa o rádio Bluetooth Serial com o nome de pareamento definido.
 * @param nomeDispositivo Nome que aparecerá na busca do smartphone (Ex: "SmartLock-Setup").
 * @return 1 para sucesso na inicialização, -1 se falhar ao iniciar o driver[cite: 17].
 */
int bt_inicializar(const char* nomeDispositivo);

/**
 * @brief Verifica o estado de conexão de um dispositivo remoto.
 * @return 1 se um cliente estiver conectado, 0 se estiver aguardando, -1 se não iniciado[cite: 17, 18].
 */
int bt_verificar_conexao(void);

/**
 * @brief Processa os comandos recebidos via Bluetooth e grava os dados na struct global.
 * @param configs Ponteiro para a estrutura de memória do sistema onde os dados serão salvos.
 * @return 1 para atualização parcial de parâmetro, 2 para sinal de finalização ("SALVAR_FIM"), 
 *         0 para ausência de dados, -1 para comando/formato inválido[cite: 17, 18].
 */
int bt_ler_entradas(ConfigSistema *configs);

/**
 * @brief Encerra a pilha do Bluetooth Serial e libera o rádio para economizar RAM e energia.
 * @return 1 se desativado com sucesso[cite: 17, 18].
 */
int bt_desligar(void);

#endif // MODULO_BLUETOOTH_H