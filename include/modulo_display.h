#ifndef MODULO_DISPLAY_H
#define MODULO_DISPLAY_H

#include <Arduino.h>
#include "config.h"

/**
 * @brief Inicializa o barramento I2C nos pinos corretos e configura a tela OLED.
 */
void inicializarDisplay();

/**
 * @brief Exibe uma mensagem de texto simples centralizada no display.
 * @param mensagem Texto a ser renderizado na tela.
 */
void atualizarStatusTela(String mensagem);

/**
 * @brief Gera e renderiza um QR Code centralizado na tela OLED.
 * @param url Texto/URL que será codificado no QR Code.
 */
void exibirQRCode(String url);

#endif // MODULO_DISPLAY_H