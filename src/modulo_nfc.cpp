// ============================================================================
// MÓDULO NFC / RFID (MFRC522) - IMPLEMENTAÇÃO
// ============================================================================
// Este arquivo gerencia o barramento de comunicação SPI e as rotinas de 
// leitura, validação e manipulação de tags RFID/NFC de forma assíncrona.

#include "modulo_nfc.h"
#include <SPI.h>
#include <MFRC522.h>

// Frequência reduzida do SPI (1 MHz) para imunidade a ruídos em cabos compridos
#define FREQUENCIA_SPI_NFC 1000000 

// Instanciação estática do driver MFRC522 utilizando as macros de pinagem do config.h.
static MFRC522 mfrc522(PINO_RFID_SDA, PINO_RFID_RST);

// ============================================================================
// FUNÇÃO: inicializarNFC
// Configura o barramento SPI e verifica se o hardware MFRC522 responde corretamente.
// ============================================================================
int8_t inicializarNFC(void) {
    // Configura e inicia as linhas do barramento SPI (SCK, MISO, MOSI, SS)
    SPI.begin(PINO_RFID_SCK, PINO_RFID_MISO, PINO_RFID_MOSI, PINO_RFID_SDA);
    
    // Reduz a velocidade do barramento no ESP32 para estabilizar sinais em fios compridos
    SPI.setFrequency(FREQUENCIA_SPI_NFC);
    
    // Envia o sinal de reset e inicialização lógica do leitor
    mfrc522.PCD_Init();

    // Valida se o chip físico responde lendo o registrador de versão interna
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

    // 1. Teste de integridade do hardware (SPI)
    byte versao = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
    if (versao == 0x00 || versao == 0xFF) {
        Serial.println(F("Aviso: Barramento SPI travado. Reiniciando..."));
        SPI.end();
        delay(15);
        SPI.begin(PINO_RFID_SCK, PINO_RFID_MISO, PINO_RFID_MOSI, PINO_RFID_SDA);
        SPI.setFrequency(FREQUENCIA_SPI_NFC);
        mfrc522.PCD_Init();
        delay(40);
        versao = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
        if (versao == 0x00 || versao == 0xFF) return -1;
    }

    // 2. Proteção contra desativação de antena por ruído indutivo
    byte txControl = mfrc522.PCD_ReadRegister(MFRC522::TxControlReg);
    if ((txControl & 0x03) != 0x03) {
        Serial.println(F("[NFC] Antena desligada por ruido. Reativando leitor..."));
        mfrc522.PCD_Init();      // Reinicializa registradores e reativa a antena
        mfrc522.PCD_AntennaOn();
    }

    // Checa se há uma nova tag no campo
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return 0;
    }

    if (!mfrc522.PICC_ReadCardSerial()) {
        return 0;
    }

    uint8_t tamanhoLido = mfrc522.uid.size;
    if (tamanhoLido > TAMANHO_MAXIMO_UID) {
        tamanhoLido = TAMANHO_MAXIMO_UID;
    }

    tagSaida->tamanho = tamanhoLido;
    memcpy(tagSaida->bytes, mfrc522.uid.uidByte, tamanhoLido);

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    return 1;
}

// ============================================================================
// FUNÇÃO: compararTags
// ============================================================================
bool compararTags(const TagNfc *tagA, const TagNfc *tagB) {
    if (tagA == NULL || tagB == NULL) return false;
    if (tagA->tamanho != tagB->tamanho) return false;
    return (memcmp(tagA->bytes, tagB->bytes, tagA->tamanho) == 0);
}

// ============================================================================
// FUNÇÃO: copiarTag
// ============================================================================
void copiarTag(TagNfc *destino, const TagNfc *origem) {
    if (destino == NULL || origem == NULL) return;
    destino->tamanho = origem->tamanho;
    memcpy(destino->bytes, origem->bytes, origem->tamanho);
}

// ============================================================================
// FUNÇÃO: formatarUidTexto
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