#ifndef MODULO_NFC_H
#define MODULO_NFC_H

#include <Arduino.h>
#include "config.h"

#define TAMANHO_MAXIMO_UID 10

// Estrutura para armazenar os dados brutos da Tag lida
typedef struct {
    uint8_t bytes[TAMANHO_MAXIMO_UID];
    uint8_t tamanho;
} TagNfc;

/**
 * @brief Inicializa o barramento SPI e o leitor MFRC522 com base nas GPIOs do config.h.
 * @return 1 em caso de sucesso, -1 em caso de erro de comunicação SPI.
 */
int8_t inicializarNFC(void);

/**
 * @brief Lê de forma assíncrona uma TAG NFC/RFID presente no leitor.
 * @param tagSaida Ponteiro para a estrutura onde o UID lido será armazenado.
 * @return 1 (nova TAG lida com sucesso), 0 (nenhuma TAG presente), -1 (erro de comunicação com o leitor).
 */
int8_t lerTagNFC(TagNfc *tagSaida);

/**
 * @brief Compara duas TAGs NFC para verificar se são idênticas.
 * @return true se forem iguais, false caso contrário.
 */
bool compararTags(const TagNfc *tagA, const TagNfc *tagB);

/**
 * @brief Copia os bytes de uma TAG origem para uma TAG destino.
 */
void copiarTag(TagNfc *destino, const TagNfc *origem);

/**
 * @brief Formata os bytes do UID em uma representação texto hexadecimal (Ex: "4A 2B 3C").
 * @param tag Ponteiro da estrutura com os dados da TAG.
 * @param bufferTexto Ponteiro do buffer char onde a String formatada será gravada.
 */
void formatarUidTexto(const TagNfc *tag, char *bufferTexto);

#endif // MODULO_NFC_H