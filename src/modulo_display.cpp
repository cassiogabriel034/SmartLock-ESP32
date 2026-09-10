#include "modulo_display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <qrcode.h>

// Instancia o objeto do display utilizando as configurações centralizadas no config.h
Adafruit_SSD1306 display(LARGURA_TELA, ALTURA_TELA, &Wire, -1);

void inicializarDisplay() {
    // Configura os pinos I2C corretos segundo o esquemático (SDA: 21, SCL: 22)
    Wire.begin(PINO_OLED_SDA, PINO_OLED_SCL);
    
    // Inicializa o controlador SSD1306 no endereço 0x3C
    if (!display.begin(SSD1306_SWITCHCAPVCC, ENDERECO_OLED)) {
        Serial.println(F("ERRO: Display OLED SSD1306 nao encontrado no barramento!"));
        for (;;); // Bloqueia a execução por segurança
    }
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
}

void atualizarStatusTela(String mensagem) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(10, 28);
    display.print(mensagem);
    display.display();
}

void exibirQRCode(String url) {
    QRCode qrcode;
    
    // Aloca o buffer na memória RAM para QR Code versão 3
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    
    // Inicializa a matriz com correção de erro nível baixo (ECC_LOW)
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, url.c_str());

    display.clearDisplay();
    
    // Calcula o deslocamento para centralizar a matriz na tela (128x64)
    int offsetX = (LARGURA_TELA - (qrcode.size * 2)) / 2;
    int offsetY = (ALTURA_TELA - (qrcode.size * 2)) / 2;

    // Varre e desenha cada módulo do QR Code
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                // Desenha blocos de 2x2 pixels para garantir leitura fácil
                display.fillRect(offsetX + (x * 2), offsetY + (y * 2), 2, 2, SSD1306_WHITE);
            }
        }
    }
    display.display();
}