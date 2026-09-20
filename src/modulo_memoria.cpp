// MÓDULO MEMÓRIA LOCAL - IMPLEMENTAÇÃO
// Gerencia em RAM as configurações do dispositivo (Wi-Fi, API, Dashboard) e
// o vetor de TAGs NFC autorizadas. Utiliza armazenamento volátil (será perdido
// ao reiniciar o ESP32 até a integração com NVS/EEPROM).

#include "modulo_memoria.h"
#include <string.h>

// VARIÁVEIS ESTÁTICAS DO MÓDULO (visíveis apenas neste arquivo)

// Cópia local da configuração do sistema
static ConfigSistema configLocal;

// Vetor de TAGs NFC autorizadas a operar a trava
static TagNfc vetorTags[MAX_TAGS_CADASTRADAS];

// Contador de TAGs atualmente cadastradas no vetor
static uint8_t quantidadeTags = 0;

// Flag de controle de estado do módulo
static bool moduloInicializado = false;

// FUNÇÕES AUXILIARES INTERNAS (não expostas no .h)

/**
 * @brief Busca o índice de uma TAG dentro do vetor de cadastrados.
 * @param tag Ponteiro para a TAG a ser buscada.
 * @return Índice da TAG (0 a quantidadeTags-1) ou -1 se não encontrada.
 */
static int8_t buscarIndiceTag(const TagNfc *tag) {
    for (uint8_t i = 0; i < quantidadeTags; i++) {
        if (compararTags(&vetorTags[i], tag)) {
            return (int8_t)i;
        }
    }
    return -1; // TAG não encontrada no vetor
}

/**
 * @brief Remove uma TAG do vetor pelo índice, deslocando os elementos à esquerda.
 * @param indice Posição da TAG a ser removida no vetor.
 */
static void removerTagPorIndice(uint8_t indice) {
    // Desloca todas as TAGs posteriores uma posição para trás
    for (uint8_t i = indice; i < quantidadeTags - 1; i++) {
        copiarTag(&vetorTags[i], &vetorTags[i + 1]);
    }

    // Decrementa o contador e limpa a última posição (agora duplicada)
    quantidadeTags--;
    memset(&vetorTags[quantidadeTags], 0, sizeof(TagNfc));
}

// FUNÇÃO: memoria_inicializar
// Zera todas as estruturas de dados e prepara o módulo para uso.

int memoria_inicializar(void) {
    // Limpa a configuração do sistema
    memset(&configLocal, 0, sizeof(ConfigSistema));
    configLocal.configurado = false;

    // Limpa o vetor de TAGs e zera o contador
    memset(vetorTags, 0, sizeof(vetorTags));
    quantidadeTags = 0;

    moduloInicializado = true;
    Serial.println(F("[MEMORIA] Módulo inicializado (RAM volátil)."));
    return 1; // Sucesso
}

// FUNÇÃO: memoria_configurar
// Salva a configuração inicial do sistema (primeira execução após Bluetooth).

int memoria_configurar(const ConfigSistema *config) {
    // Validação defensiva: módulo deve estar inicializado e ponteiro válido
    if (!moduloInicializado || config == NULL) return -1;

    // Impede reconfiguração se o sistema já foi configurado
    if (configLocal.configurado) {
        Serial.println(F("[MEMORIA] Sistema já configurado. Use alterar_config()."));
        return 0;
    }

    // Copia toda a estrutura recebida para a memória local
    memcpy(&configLocal, config, sizeof(ConfigSistema));
    configLocal.configurado = true;

    Serial.println(F("[MEMORIA] Configuração inicial salva com sucesso."));
    return 1; // Sucesso
}


// FUNÇÃO: memoria_add_rmv_tag
// Comportamento Toggle: adiciona a TAG se não existir, remove se já existir.
// Exige validação prévia da TAG Master (administradora) do sistema.

int memoria_add_rmv_tag(const TagNfc *tagAdmin, const TagNfc *novaTag) {
    // Validação defensiva: módulo, ponteiros e configuração
    if (!moduloInicializado || tagAdmin == NULL || novaTag == NULL) return -1;

    // O sistema precisa estar configurado para ter uma TAG Master definida
    if (!configLocal.configurado) {
        Serial.println(F("[MEMORIA] Erro: Sistema não configurado (sem TAG Master)."));
        return -1;
    }

    // Verifica se a TAG apresentada como admin é de fato a TAG Master cadastrada
    if (!compararTags(tagAdmin, &configLocal.tagMaster)) {
        Serial.println(F("[MEMORIA] Erro: TAG Admin inválida!"));
        return -1;
    }

    // Impede o cadastro da própria TAG Master como TAG comum
    if (compararTags(novaTag, &configLocal.tagMaster)) {
        Serial.println(F("[MEMORIA] Erro: TAG Master não pode ser cadastrada como comum."));
        return -1;
    }

    // Busca se a TAG já existe no vetor
    int8_t indice = buscarIndiceTag(novaTag);

    if (indice >= 0) {
        // ---- TAG ENCONTRADA: REMOVER (toggle OFF) ----
        removerTagPorIndice((uint8_t)indice);

        Serial.print(F("[MEMORIA] TAG removida. Total: "));
        Serial.println(quantidadeTags);
        return 0; // TAG removida
    } else {
        // ---- TAG NÃO ENCONTRADA: ADICIONAR (toggle ON) ----
        if (quantidadeTags >= MAX_TAGS_CADASTRADAS) {
            Serial.println(F("[MEMORIA] Erro: Vetor de TAGs cheio!"));
            return -1; // Sem espaço no vetor
        }

        copiarTag(&vetorTags[quantidadeTags], novaTag);
        quantidadeTags++;

        Serial.print(F("[MEMORIA] TAG adicionada. Total: "));
        Serial.println(quantidadeTags);
        return 1; // TAG adicionada
    }
}

// FUNÇÃO: memoria_reset_sistema
// Restaura o módulo ao estado de fábrica, limpando todas as configurações
// e removendo todas as TAGs do vetor de autorizados.

int memoria_reset_sistema(void) {
    if (!moduloInicializado) return -1;

    // Zera a configuração completa do sistema
    memset(&configLocal, 0, sizeof(ConfigSistema));
    configLocal.configurado = false;

    // Limpa o vetor de TAGs e reseta o contador
    memset(vetorTags, 0, sizeof(vetorTags));
    quantidadeTags = 0;

    Serial.println(F("[MEMORIA] Reset de fábrica executado. Todos os dados apagados."));
    return 1; // Sucesso
}

// FUNÇÃO: memoria_consulta_tag
// Pesquisa uma TAG no vetor de autorizados por comparação byte a byte.

int memoria_consulta_tag(const TagNfc *tag) {
    // Validação defensiva
    if (!moduloInicializado || tag == NULL) return -1;

    int8_t indice = buscarIndiceTag(tag);

    // Retorna 1 se encontrada, 0 se não
    return (indice >= 0) ? 1 : 0;
}


// FUNÇÃO: memoria_alterar_config
// Atualiza os parâmetros de rede e API mantendo a TAG Master e o vetor intactos.

int memoria_alterar_config(const ConfigSistema *novaConfig) {
    // Validação defensiva
    if (!moduloInicializado || novaConfig == NULL) return -1;

    // Só permite alteração se o sistema já passou pelo setup inicial
    if (!configLocal.configurado) {
        Serial.println(F("[MEMORIA] Erro: Configure o sistema antes de alterar."));
        return 0;
    }

    // Salva a TAG Master atual antes de sobrescrever (proteção)
    TagNfc tagMasterBackup;
    copiarTag(&tagMasterBackup, &configLocal.tagMaster);

    // Sobrescreve a configuração com os novos valores
    memcpy(&configLocal, novaConfig, sizeof(ConfigSistema));

    // Restaura a TAG Master original e mantém o flag de configurado
    copiarTag(&configLocal.tagMaster, &tagMasterBackup);
    configLocal.configurado = true;

    Serial.println(F("[MEMORIA] Configurações atualizadas com sucesso."));
    return 1; // Sucesso
}


// FUNÇÃO: memoria_obter_config
// Retorna acesso somente-leitura à configuração armazenada em RAM.

const ConfigSistema* memoria_obter_config(void) {
    if (!moduloInicializado) return NULL;
    return &configLocal;
}


// FUNÇÃO: memoria_obter_quantidade_tags
// Retorna o número de TAGs atualmente registradas no vetor.

uint8_t memoria_obter_quantidade_tags(void) {
    return quantidadeTags;
}
