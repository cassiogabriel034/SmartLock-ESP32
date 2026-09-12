#include <Arduino.h>
#include "config.h"
#include "config_structs.h"
#include "modulo_nfc.h"
#include "modulo_bluetooth.h"
#include "modulo_display.h"
#include "modulo_botoes.h"

// Objetos e Variáveis Globais
TagNfc tagLida;
ConfigSistema configs;
Botao btnNavegacao; // Utiliza o botão do GPIO 13

// Estados do Sistema
enum EstadoSistema { 
    SETUP_BLUETOOTH, 
    NAVEGACAO_CONFIG, 
    MODO_OPERACIONAL, 
    TRAVA_ABERTA 
};

EstadoSistema estadoAtual = SETUP_BLUETOOTH;
uint8_t paginaAtual = 0;

// Exibe todas as configurações recebidas de uma vez no Terminal do VS Code
void imprimirConfigsTerminal() {
    Serial.println(F("\n=============================================="));
    Serial.println(F("    CONFIGURACOES RECEBIDAS VIA BLUETOOTH     "));
    Serial.println(F("=============================================="));
    Serial.print(F("SSID Wi-Fi   : ")); Serial.println(configs.wifi.ssid);
    Serial.print(F("Senha Wi-Fi  : ")); Serial.println(configs.wifi.password);
    Serial.print(F("IP Estatico  : ")); Serial.println(configs.wifi.ipEstatico);
    Serial.print(F("Link da API  : ")); Serial.println(configs.apiLink);
    Serial.print(F("Dash Usuario : ")); Serial.println(configs.dashboard.usuario);
    Serial.println(F("==============================================\n"));
}

// Atualiza a tela OLED com a página de configuração correspondente
void exibirPaginaConfig(uint8_t pagina) {
    switch (pagina) {
        case 0:
            atualizarStatusTela("Setup Concluido!\n\nBtn GPIO13: Avançar");
            break;
        case 1:
            atualizarStatusTela(String("[1/5] SSID Wi-Fi:\n") + configs.wifi.ssid);
            break;
        case 2:
            atualizarStatusTela(String("[2/5] Senha Wi-Fi:\n") + configs.wifi.password);
            break;
        case 3:
            atualizarStatusTela(String("[3/5] IP Estatico:\n") + configs.wifi.ipEstatico);
            break;
        case 4:
            atualizarStatusTela(String("[4/5] Link API:\n") + configs.apiLink);
            break;
        case 5:
            atualizarStatusTela(String("[5/5] Dash User:\n") + configs.dashboard.usuario);
            break;
        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Inicialização dos Pinos Digitais (LED no GPIO 17 e Sensor no GPIO 4)
    pinMode(PINO_RELE_TRAVA, OUTPUT);
    digitalWrite(PINO_RELE_TRAVA, LOW); // Garante LED desligado no boot

    pinMode(PINO_FIM_DE_CURSO, INPUT_PULLUP);

    // Inicializa o botão no GPIO 13
    botao_inicializar(&btnNavegacao, PINO_BOTAO_VERDE);

    // Inicializa periféricos de display e leitor NFC
    inicializarDisplay();
    atualizarStatusTela("Iniciando...");

    if (inicializarNFC() == 1) {
        Serial.println(F("[NFC] Hardware inicializado."));
    } else {
        Serial.println(F("[NFC] ERRO de comunicação SPI!"));
        atualizarStatusTela("ERRO: NFC SPI");
        delay(2000);
    }

    // Inicializa o Bluetooth SPP
    if (bt_inicializar("SmartLock-Setup") == 1) {
        Serial.println(F("[BT] Aguardando envio de configuracoes..."));
        atualizarStatusTela("BT: Aguardando...");
    }
}

void loop() {
    switch (estadoAtual) {
        // --------------------------------------------------------------------
        // ETAPA 1: Configuração via Bluetooth
        // --------------------------------------------------------------------
        case SETUP_BLUETOOTH: {
            if (bt_verificar_conexao() == 1) {
                int statusBt = bt_ler_entradas(&configs);

                if (statusBt == 1) {
                    atualizarStatusTela("BT: Dado Recebido");
                } 
                else if (statusBt == 2 || configs.configurado) {
                    // Imprime o relatório completo de uma só vez no Terminal
                    imprimirConfigsTerminal();

                    // Desliga a pilha de rádio Bluetooth
                    bt_desligar();

                    // Prepara navegação de telas via botão
                    paginaAtual = 0;
                    exibirPaginaConfig(paginaAtual);
                    estadoAtual = NAVEGACAO_CONFIG;
                }
            }
            break;
        }

        // --------------------------------------------------------------------
        // ETAPA 2: Paginação das Configurações na Tela (Via Botão GPIO 13)
        // --------------------------------------------------------------------
        case NAVEGACAO_CONFIG: {
            if (botao_verificar_clique(&btnNavegacao) == 1) {
                paginaAtual++;
                if (paginaAtual > 5) {
                    Serial.println(F("[SISTEMA] Leitura NFC Liberada!"));
                    atualizarStatusTela("Aproxime a Tag");
                    estadoAtual = MODO_OPERACIONAL;
                } else {
                    exibirPaginaConfig(paginaAtual);
                }
            }
            break;
        }

        // --------------------------------------------------------------------
        // ETAPA 3: Modo Operacional (Aguardando Leitura NFC)
        // --------------------------------------------------------------------
        case MODO_OPERACIONAL: {
            // Permite resetar a mensagem da tela pressionando o botão 13
            if (botao_verificar_clique(&btnNavegacao) == 1) {
                atualizarStatusTela("Aproxime a Tag");
            }

            if (lerTagNFC(&tagLida) == 1) {
                char bufferUid[30];
                formatarUidTexto(&tagLida, bufferUid);

                Serial.print(F("[NFC] Tag Detectada! UID: "));
                Serial.println(bufferUid);

                // Liga o LED no GPIO 17 (Simulando o acionamento da trava/relé)
                digitalWrite(PINO_RELE_TRAVA, HIGH);

                Serial.println(F("[ATUADOR] LED (GPIO 17) LIGADO - Trava Aberta!"));
                atualizarStatusTela(String("Tag Lida!\n") + bufferUid + "\nLED: LIGADO");

                estadoAtual = TRAVA_ABERTA;
            }
            break;
        }

        // --------------------------------------------------------------------
        // ETAPA 4: Trava Aberta / LED Ligado (Aguardando Chave Fim de Curso)
        // --------------------------------------------------------------------
        case TRAVA_ABERTA: {
            // O botão 13 exibe o status atual do atuador se pressionado
            if (botao_verificar_clique(&btnNavegacao) == 1) {
                atualizarStatusTela("Status: LED LIGADO\nPresione FimCurso");
            }

            // Pressionar a Chave Fim de Curso no GPIO 4 (Nível Lógico LOW)
            if (digitalRead(PINO_FIM_DE_CURSO) == LOW) {
                // Desliga o LED no GPIO 17
                digitalWrite(PINO_RELE_TRAVA, LOW);

                Serial.println(F("[SENSOR] Fim de Curso acionado (GPIO 4)!"));
                Serial.println(F("[ATUADOR] LED (GPIO 17) DESLIGADO - Trava Fechada."));

                atualizarStatusTela("Porta Fechada!\nLED Desligado");
                delay(1500);

                atualizarStatusTela("Aproxime a Tag");
                estadoAtual = MODO_OPERACIONAL;
            }
            break;
        }
    }
}