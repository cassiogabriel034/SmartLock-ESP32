// ============================================================================
// MÓDULO NFC / RFID (MFRC522) - IMPLEMENTAÇÃO
// ============================================================================
// Este arquivo gerencia o barramento de comunicação SPI e as rotinas de 
// leitura, validação e manipulação de tags RFID/NFC de forma assíncrona.

#include "modulo_nfc.h"
#include <SPI.h>
#include <MFRC522.h>

// Instanciação estática do driver MFRC522 utilizando as macros de pinagem do config.h.
// PINO_RFID_SDA (CS/SS) e PINO_RFID_RST (Reset por hardware).
static MFRC522 mfrc522(PINO_RFID_SDA, PINO_RFID_RST);

// ============================================================================
// FUNÇÃO: inicializarNFC
// Configura o barramento SPI e verifica se o hardware MFRC522 responde corretamente.
// ============================================================================
int8_t inicializarNFC(void) {
    // Configura e inicia as linhas do barramento SPI (SCK: 18, MISO: 19, MOSI: 23, SS: 5)
    SPI.begin(PINO_RFID_SCK, PINO_RFID_MISO, PINO_RFID_MOSI, PINO_RFID_SDA);
    
    // Envia o sinal de reset e inicialização lógica do leitor
    mfrc522.PCD_Init();

    // Valida se o chip físico responde lendo o registrador de versão interna.
    // Se retornar 0x00 ou 0xFF, significa falha física (mau contato, fiação solta ou pino incorreto).
    byte versao = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
    if (versao == 0x00 || versao == 0xFF) {
        Serial.println(F("ERRO: Falha de comunicacao com o leitor RFID MFRC522!"));
        return -1; // Retorna erro de comunicação por hardware
    }

    return 1; // Leitor detectado e pronto para operação
}

// ============================================================================
// FUNÇÃO: lerTagNFC
// Realiza a busca assíncrona por cartões/tags no campo de radiofrequência.
// Retornos: 1 (Nova TAG lida), 0 (Nenhuma TAG no campo), -1 (Erro de hardware)
// ============================================================================
int8_t lerTagNFC(TagNfc *tagSaida) {
    // Programação defensiva: impede acesso a ponteiros nulos
    if (tagSaida == NULL) return -1;

    // Teste preventivo no registrador de versão para detectar se o leitor foi desconectado
    byte versao = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
    if (versao == 0x00 || versao == 0xFF) {
        return -1; // Retorna erro de comunicação
    }

    // Checagem não-bloqueante: verifica se há um novo cartão no campo de indução
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return 0; // Nenhuma tag detectada no momento
    }

    // Tenta efetuar a leitura dos bytes seriais (UID) da tag selecionada
    if (!mfrc522.PICC_ReadCardSerial()) {
        return 0; // Tag presente, mas falhou ao extrair o código serial
    }

    // Limita a quantidade de bytes lidos ao tamanho máximo seguro do buffer interno
    uint8_t tamanhoLido = mfrc522.uid.size;
    if (tamanhoLido > TAMANHO_MAXIMO_UID) {
        tamanhoLido = TAMANHO_MAXIMO_UID;
    }

    // Armazena a dimensão do UID e copia o array de bytes bruto para a estrutura de saída
    tagSaida->tamanho = tamanhoLido;
    memcpy(tagSaida->bytes, mfrc522.uid.uidByte, tamanhoLido);

    // Envia comandos de repouso para interromper a transmissão e evitar leituras duplicadas continuas
    mfrc522.PICC_HaltA();       // Coloca o cartão em modo sleep
    mfrc522.PCD_StopCrypto1();  // Desativa a criptografia na comunicação SPI

    return 1; // Nova tag capturada com sucesso
}

// ============================================================================
// FUNÇÃO: compararTags
// Compara o conteúdo e o tamanho de duas estruturas de tags NFC.
// ============================================================================
bool compararTags(const TagNfc *tagA, const TagNfc *tagB) {
    // Valida se os ponteiros são válidos
    if (tagA == NULL || tagB == NULL) return false;
    
    // Se o comprimento em bytes for diferente, as tags não são iguais
    if (tagA->tamanho != tagB->tamanho) return false;

    // Compara o bloco de memória byte a byte; retorna true apenas se forem exatamente idênticos
    return (memcmp(tagA->bytes, tagB->bytes, tagA->tamanho) == 0);
}

// ============================================================================
// FUNÇÃO: copiarTag
// Duplica os dados de uma estrutura TagNfc de origem para uma de destino.
// ============================================================================
void copiarTag(TagNfc *destino, const TagNfc *origem) {
    // Proteção contra ponteiros nulos
    if (destino == NULL || origem == NULL) return;

    // Copia o tamanho do UID e transfere o bloco de bytes na memória
    destino->tamanho = origem->tamanho;
    memcpy(destino->bytes, origem->bytes, origem->tamanho);
}

// ============================================================================
// FUNÇÃO: formatarUidTexto
// Converte os bytes brutos do UID em uma String formato hexadecimal legível (ex: "4A 2B 3C").
// ============================================================================
void formatarUidTexto(const TagNfc *tag, char *bufferTexto) {
    // Proteção contra ponteiros inválidos
    if (tag == NULL || bufferTexto == NULL) return;

    String stringHexadecimal = "";

    // Varre cada byte do UID convertendo para representação ASCII em formato hexadecimal
    for (uint8_t i = 0; i < tag->tamanho; i++) {
        // Insere o caractere '0' à esquerda para manter a formatação de 2 dígitos em valores menores que 0x10
        if (tag->bytes[i] < 0x10) stringHexadecimal += "0";
        
        stringHexadecimal += String(tag->bytes[i], HEX);
        
        // Adiciona um espaço delimitador entre os bytes (exceto no último)
        if (i < tag->tamanho - 1) stringHexadecimal += " ";
    }
    
    // Converte todas as letras do texto para maiúsculas (ex: "4a" -> "4A")
    stringHexadecimal.toUpperCase();

    // Copia com segurança a cadeia de caracteres gerada para o buffer de destino passado pelo chamador
    strcpy(bufferTexto, stringHexadecimal.c_str());
}