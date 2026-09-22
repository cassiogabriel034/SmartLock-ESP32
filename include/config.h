/*
 * Arquivo: config.h
 * Projeto: SmartLock-ESP32-Ligacoes-Eletricas
 * 
 * Guarda de Inclusão (#ifndef / #define / #endif)
 * Impede que este arquivo de cabeçalho seja incluído múltiplas vezes na mesma
 * unidade de compilação, evitando erros de redefinição de símbolos pela toolchain.
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/*
 * Mapeamento da Interface I2C (Display OLED SSD1306)
 */
#define PINO_OLED_SDA   16    // GPIO21 -> Linha de Dados (SDA)
#define PINO_OLED_SCL   17    // GPIO22 -> Linha de Clock (SCL)
#define ENDERECO_OLED   0x3C  // Endereço I2C padrão do display
#define LARGURA_TELA    128   // Largura da tela em pixels
#define ALTURA_TELA     64    // Altura da tela em pixels

/*
 * Mapeamento da Interface SPI (Leitor RFID MFRC522)
 */
#define PINO_RFID_SDA   26     // GPIO05 -> Slave Select (SS / CS)
#define PINO_RFID_SCK   27    // GPIO18 -> Serial Clock (SCK)
#define PINO_RFID_MOSI  14    // GPIO23 -> Master Out Slave In (MOSI)
#define PINO_RFID_MISO  12    // GPIO19 -> Master In Slave Out (MISO)
#define PINO_RFID_RST   13    // GPIO27 -> Reset lógico por hardware (RST)

/*
 * Mapeamento de Atuadores
 */
#define PINO_RELE_TRAVA 5    // GPIO17 -> Comando do Módulo Relé (Trava Solenoide / LED)

/*
 * Mapeamento de Sensores e Botões (Entradas Digitais)
 */
#define PINO_FIM_DE_CURSO   25   // GPIO04 -> Validador de Porta (Módulo YL-99 / Slide Switch)
#define PINO_BOTAO_AZUL     18  // GPIO12 -> Botão Azul
#define PINO_BOTAO_VERDE    15  // GPIO13 -> Botão Verde
#define PINO_BOTAO_VERMELHO 4  // GPIO14 -> Botão Vermelho

/*
 * Códigos de Retorno Padronizados (Programação Defensiva)
 */
#define STATUS_SUCESSO         1  // Operação executada perfeitamente
#define STATUS_FALHA         0  // Operação com falha
#define STATUS_ERRO_PARAMETRO -1  // Argumento inválido ou fora dos limites

#endif // CONFIG_H