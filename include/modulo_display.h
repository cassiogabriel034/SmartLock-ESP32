#ifndef MODULO_DISPLAY_H
#define MODULO_DISPLAY_H

#include <Arduino.h>
#include "config.h"

/**
 * @brief Inicializa o barramento I2C nos pinos corretos e configura a tela OLED.
 */
void inicializarDisplay();

/**
 * @brief Exibe uma mensagem de texto centralizada no display com tamanho de fonte ajustável.
 * @param mensagem Texto a ser renderizado na tela.
 * @param tamanhoFonte Tamanho do texto (padrão = 1 caso não seja informado).
 */
void atualizarStatusTela(String mensagem, uint8_t tamanhoFonte = 1);

/**
 * @brief Preenche toda a tela com cor ativa para detectar linhas mortas (pixels queimados).
 */
void testarPixelsTela();

/**
 * @brief Gera e renderiza um QR Code centralizado na tela OLED.
 * @param url Texto/URL que será codificado no QR Code.
 */
void exibirQRCode(String url);

#endif // MODULO_DISPLAY_H