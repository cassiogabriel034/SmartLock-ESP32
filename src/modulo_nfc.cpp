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
    // Configura e inicia as linhas do barramento SPI (SCK, MISO, MOSI, SS)
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
    if (tagSaida == NULL) return -1;

    // Teste de integridade do hardware
    byte versao = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
    
    // Se o leitor travou devido ao surto indutivo da fechadura (retornando 0x00 ou 0xFF)
    if (versao == 0x00 || versao == 0xFF) {
        Serial.println(F("Aviso: Ruído detectado. Reiniciando barramento SPI..."));
        
        // Finaliza o barramento SPI travado para limpar registradores internos
        SPI.end();
        delay(15);
        
        // Inicializa as linhas físicas do SPI do zero com os pinos do config.h
        SPI.begin(PINO_RFID_SCK, PINO_RFID_MISO, PINO_RFID_MOSI, PINO_RFID_SDA);
        mfrc522.PCD_Init();
        delay(40);
        
        // Faz uma contraprova de leitura após a limpeza do barramento
        versao = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
        if (versao == 0x00 || versao == 0xFF) {
            return -1; // Se persistir, o leitor foi realmente desconectado
        }
    }

    // Checa se há uma nova tag no campo
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return 0;
    }

    // Tenta ler o código serial da tag
    if (!mfrc522.PICC_ReadCardSerial()) {
        return 0;
    }

    uint8_t tamanhoLido = mfrc522.uid.size;
    if (tamanhoLido > TAMANHO_MAXIMO_UID) {
        tamanhoLido = TAMANHO_MAXIMO_UID;
    }

    tagSaida->tamanho = tamanhoLido;
    memcpy(tagSaida->bytes, mfrc522.uid.uidByte, tamanhoLido);

    // Encerra a comunicação da tag e reseta a criptografia do chip
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    return 1;
}

// ============================================================================
// FUNÇÃO: compararTags
// Compara o conteúdo e o tamanho de duas estruturas de tags NFC.
// ============================================================================
bool compararTags(const TagNfc *tagA, const TagNfc *tagB) {
    if (tagA == NULL || tagB == NULL) return false;
    if (tagA->tamanho != tagB->tamanho) return false;
    return (memcmp(tagA->bytes, tagB->bytes, tagA->tamanho) == 0);
}

// ============================================================================
// FUNÇÃO: copiarTag
// Duplica os dados de uma estrutura TagNfc de origem para uma de destino.
// ============================================================================
void copiarTag(TagNfc *destino, const TagNfc *origem) {
    if (destino == NULL || origem == NULL) return;
    destino->tamanho = origem->tamanho;
    memcpy(destino->bytes, origem->bytes, origem->tamanho);
}

// ============================================================================
// FUNÇÃO: formatarUidTexto
// Converte os bytes brutos do UID em uma String formato hexadecimal legível (ex: "4A 2B 3C").
// ============================================================================
void formatarUidTexto(const TagNfc *tag, char *bufferTexto) {
    if (tag == NULL || bufferTexto == NULL) return;

    String stringHexadecimal = "";

    for (uint8_t i = 0; i < tag->tamanho; i++) {
        if (tag->bytes[i] < 0x10) stringHexadecimal += "0";
        stringHexadecimal += String(tag->bytes[i], HEX);
        if (i < tag->tamanho - 1) stringHexadecimal += " ";
    }
    
    stringHexadecimal.toUpperCase();
    strcpy(bufferTexto, stringHexadecimal.c_str());
}