// MÓDULO MEMÓRIA LOCAL
// Define a interface pública para o gerenciamento em RAM das configurações
// do sistema e do vetor de TAGs NFC autorizadas.

#ifndef MODULO_MEMORIA_H
#define MODULO_MEMORIA_H

#include "config_structs.h"
#include "modulo_nfc.h"

// Capacidade máxima do vetor de TAGs cadastradas em RAM
#define MAX_TAGS_CADASTRADAS 20

/**
 * @brief Inicializa o módulo de memória, zerando as configurações e o vetor de TAGs.
 * @return 1 para sucesso na inicialização.
 */
int memoria_inicializar(void);

/**
 * @brief Grava a configuração inicial do sistema (primeira vez após setup Bluetooth).
 *        Só executa se o sistema ainda não estiver configurado.
 * @param config Ponteiro para a estrutura ConfigSistema preenchida.
 * @return 1 para sucesso, 0 se o sistema já estiver configurado, -1 para erro de parâmetro.
 */
int memoria_configurar(const ConfigSistema *config);

/**
 * @brief Adiciona ou remove uma TAG do vetor de autorizados (comportamento toggle).
 *        Exige a apresentação prévia da TAG Master (admin) para validação.
 *        - Se a TAG não existir no vetor: adiciona.
 *        - Se a TAG já existir no vetor: remove.
 * @param tagAdmin Ponteiro para a TAG apresentada como administradora.
 * @param novaTag  Ponteiro para a TAG a ser adicionada ou removida.
 * @return 1 (TAG adicionada), 0 (TAG removida), -1 (erro: admin inválida, vetor cheio ou ponteiro nulo).
 */
int memoria_add_rmv_tag(const TagNfc *tagAdmin, const TagNfc *novaTag);

/**
 * @brief Restaura o módulo ao estado de fábrica (zera configs e limpa todas as TAGs).
 * @return 1 para sucesso, -1 se o módulo não foi inicializado.
 */
int memoria_reset_sistema(void);

/**
 * @brief Consulta se uma TAG está cadastrada no vetor de autorizados.
 * @param tag Ponteiro para a TAG a ser pesquisada.
 * @return 1 (TAG encontrada / autorizada), 0 (TAG não encontrada), -1 (erro de parâmetro).
 */
int memoria_consulta_tag(const TagNfc *tag);

/**
 * @brief Atualiza as configurações do sistema já configurado (Wi-Fi, API, Dashboard).
 *        Preserva a TAG Master e o vetor de TAGs cadastradas.
 * @param novaConfig Ponteiro para a estrutura com os novos valores.
 * @return 1 para sucesso, 0 se o sistema ainda não foi configurado, -1 para erro de parâmetro.
 */
int memoria_alterar_config(const ConfigSistema *novaConfig);

/**
 * @brief Retorna um ponteiro somente-leitura para a configuração atual do sistema.
 * @return Ponteiro para ConfigSistema, ou NULL se o módulo não foi inicializado.
 */
const ConfigSistema* memoria_obter_config(void);

/**
 * @brief Retorna a quantidade atual de TAGs cadastradas no vetor.
 * @return Número de TAGs registradas (0 a MAX_TAGS_CADASTRADAS).
 */
uint8_t memoria_obter_quantidade_tags(void);

#endif // MODULO_MEMORIA_H
