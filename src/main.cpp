#include <Arduino.h>
#include "modulo_display.h"
#include "modulo_nfc.h"
#include "modulo_botoes.h"
#include "modulo_bluetooth.h"
#include "modulo_wifi.h"

// Instâncias para os botões de teste
Botao btnAzul, btnVerde, btnVermelho, chavePorta;
ConfigSistema configs;
TagNfc tagLida;

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== INICIANDO TESTE INTEGRADO DE MÓDULOS ===");

    // 1. Teste do Relé (GPIO 17)
    pinMode(17, OUTPUT);
    digitalWrite(17, HIGH); // Ativa trava/LED
    delay(1000);
    digitalWrite(17, LOW);  // Desativa trava/LED

    // 2. Teste do Display OLED
    inicializarDisplay();
    atualizarStatusTela("TESTE INICIAL");
    delay(1000);
    exibirQRCode("http://192.168.1.184");
    delay(1500);

    // 3. Teste dos Botões
    botao_inicializar(&chavePorta, 4);
    botao_inicializar(&btnAzul, 12);
    botao_inicializar(&btnVerde, 13);
    botao_inicializar(&btnVermelho, 14);

    // 4. Teste do Leitor RFID/NFC
    if (inicializarNFC() == 1) {
        Serial.println("[NFC] Leitor inicializado com sucesso!");
    } else {
        Serial.println("[NFC] ERRO na comunicacao com leitor!");
    }
}

void loop() {
    // Check de Botões
    if (botao_verificar_clique(&btnAzul) == 1)      Serial.println("[BOTAO] Azul Pressionado!");
    if (botao_verificar_clique(&btnVerde) == 1)     {
        Serial.println("[BOTAO] Verde Pressionado!");
        // 1. Teste do Relé (GPIO 17)
        pinMode(17, OUTPUT);
        digitalWrite(17, HIGH); // Ativa trava/LED
        delay(1000);
        digitalWrite(17, LOW);  // Desativa trava/LED
    }
    if (botao_verificar_clique(&btnVermelho) == 1)  Serial.println("[BOTAO] Vermelho Pressionado!");
    if (botao_verificar_clique(&chavePorta) == 1)   Serial.println("[SENSOR] Chave de Porta Alterada!");

    // Check de Leitura RFID
    if (lerTagNFC(&tagLida) == 1) {
        char bufferUid[30];
        formatarUidTexto(&tagLida, bufferUid);
        Serial.print("[NFC] Tag Detectada: ");
        Serial.println(bufferUid);
        atualizarStatusTela(String("TAG: ") + bufferUid);
    }
}