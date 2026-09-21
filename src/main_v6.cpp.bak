#include <Arduino.h>
#include "config.h"
#include "config_structs.h"
#include "modulo_nfc.h"
#include "modulo_bluetooth.h"
#include "modulo_display.h"
#include "modulo_botoes.h"
#include "modulo_porta.h"

// Objetos e Variáveis Globais
TagNfc tagLida;
ConfigSistema configs;

// Instância dos três botões
Botao btnBranco;   // GPIO 15 (PINO_BOTAO_VERDE - Botão de Avanço / Abertura)[cite: 12]
Botao btnVermelho; // GPIO 4  (PINO_BOTAO_VERMELHO - Botão de Voltar / Fechamento)[cite: 12]
Botao btnAzul;     // GPIO 18 (PINO_BOTAO_AZUL - Botão de Status / Pular)[cite: 12]

// Estados do Sistema
enum EstadoSistema { 
    SETUP_BLUETOOTH, 
    NAVEGACAO_CONFIG, 
    MODO_OPERACIONAL, 
    TRAVA_ABERTA 
};

#define fonte_tela 2

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

// Atualiza a tela OLED com a página de configuração correspondente (Tudo em Fonte 2)
void exibirPaginaConfig(uint8_t pagina) {
    switch (pagina) {
        case 0:
            atualizarStatusTela("Setup Ok!\nB:Avan\nV:Volt", fonte_tela);
            break;
        case 1:
            atualizarStatusTela(String("[1/5] SSID:\n") + configs.wifi.ssid, fonte_tela);
            break;
        case 2:
            atualizarStatusTela(String("[2/5] Senha:\n") + configs.wifi.password, fonte_tela);
            break;
        case 3:
            atualizarStatusTela(String("[3/5] IP:\n") + configs.wifi.ipEstatico, fonte_tela);
            break;
        case 4:
            atualizarStatusTela(String("[4/5] API:\n") + configs.apiLink, fonte_tela);
            break;
        case 5:
            atualizarStatusTela(String("[5/5] Dash:\n") + configs.dashboard.usuario, fonte_tela);
            break;
        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // 1. Inicialização do hardware da porta
    inicializarPorta();

    // 2. Inicializa os 3 botões do sistema[cite: 12, 19]
    botao_inicializar(&btnBranco, PINO_BOTAO_VERDE);
    botao_inicializar(&btnVermelho, PINO_BOTAO_VERMELHO);
    botao_inicializar(&btnAzul, PINO_BOTAO_AZUL);

    // 3. Inicializa Display e Leitor NFC
    inicializarDisplay();
    atualizarStatusTela("Iniciando...", fonte_tela);

    if (inicializarNFC() == 1) {
        Serial.println(F("[NFC] Hardware inicializado com sucesso."));
    } else {
        Serial.println(F("[NFC] ERRO de comunicação SPI!"));
        atualizarStatusTela("ERRO: NFC", fonte_tela);
        delay(2000);
    }

    // 4. Inicializa o Bluetooth SPP
    if (bt_inicializar("SmartLock-Setup") == 1) {
        Serial.println(F("[BT] Aguardando envio de configuracoes..."));
        atualizarStatusTela("BT:\nAguardando", fonte_tela);
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
                    atualizarStatusTela("BT: Dado\nRecebido", fonte_tela);
                } 
                else if (statusBt == 2 || configs.configurado) {
                    imprimirConfigsTerminal();
                    bt_desligar();

                    paginaAtual = 0;
                    exibirPaginaConfig(paginaAtual);
                    estadoAtual = NAVEGACAO_CONFIG;
                }
            }
            break;
        }

        // --------------------------------------------------------------------
        // ETAPA 2: Navegação das Configurações na Tela (Todos os botões)
        // --------------------------------------------------------------------
        case NAVEGACAO_CONFIG: {
            // Botão Branco: Avançar Página
            if (botao_verificar_clique(&btnBranco) == 1) {
                paginaAtual++;
                if (paginaAtual > 5) {
                    Serial.println(F("[SISTEMA] Navegação concluída -> Modo Operacional."));
                    atualizarStatusTela("Aproxime\na Tag", fonte_tela);
                    estadoAtual = MODO_OPERACIONAL;
                } else {
                    exibirPaginaConfig(paginaAtual);
                }
            }

            // Botão Vermelho: Voltar Página
            if (botao_verificar_clique(&btnVermelho) == 1) {
                if (paginaAtual > 0) {
                    paginaAtual--;
                    exibirPaginaConfig(paginaAtual);
                }
            }

            // Botão Azul: Pular Navegação e ir direto para Operacional
            if (botao_verificar_clique(&btnAzul) == 1) {
                Serial.println(F("[SISTEMA] Pular menu -> Modo Operacional."));
                atualizarStatusTela("Aproxime\na Tag", fonte_tela);
                estadoAtual = MODO_OPERACIONAL;
            }
            break;
        }

        // --------------------------------------------------------------------
        // ETAPA 3: Modo Operacional (Aguardando Leitura NFC ou Comandos)
        // --------------------------------------------------------------------
        case MODO_OPERACIONAL: {
            // Leitura NFC
            if (lerTagNFC(&tagLida) == 1) {
                char bufferUid[30];
                formatarUidTexto(&tagLida, bufferUid);

                Serial.print(F("[NFC] Tag Detectada! UID: "));
                Serial.println(bufferUid);

                // Aciona o relé (LIGA LED / TRAVA)
                abrirPorta();

                atualizarStatusTela(String("Tag Lida!\n") + bufferUid, fonte_tela);
                delay(1200);
                atualizarStatusTela("LED: LIGADO\nFimCurso", fonte_tela);

                estadoAtual = TRAVA_ABERTA;
                break;
            }

            // Botão Branco: Abertura Manual da Trava
            if (botao_verificar_clique(&btnBranco) == 1) {
                abrirPorta();
                Serial.println(F("[MANUAL] Trava ABERTA via Botão Branco"));
                atualizarStatusTela("LED: LIGADO\n(Manual)", fonte_tela);
                estadoAtual = TRAVA_ABERTA;
            }

            // Botão Vermelho: Resetar mensagem da tela
            if (botao_verificar_clique(&btnVermelho) == 1) {
                atualizarStatusTela("Aproxime\na Tag", fonte_tela);
            }

            // Botão Azul: Exibir status atual do sistema
            if (botao_verificar_clique(&btnAzul) == 1) {
                String statusStr = "LED: OFF\nPorta: ";
                statusStr += estaPortaFechada() ? "FECH" : "ABER";
                atualizarStatusTela(statusStr, fonte_tela);
            }
            break;
        }

        // --------------------------------------------------------------------
        // ETAPA 4: Trava Aberta / LED Ligado (Aguardando Fim de Curso ou Botão)
        // --------------------------------------------------------------------
        case TRAVA_ABERTA: {
            // 1. Encerramento Automático via Chave Fim de Curso (Porta Fechada)
            if (estaPortaFechada()) {
                fecharPorta(); // Desliga Relé / LED

                Serial.println(F("[SENSOR] Fim de curso acionado -> Trava Fechada."));
                atualizarStatusTela("Fechado!\nLED: OFF", fonte_tela);
                delay(1500);

                atualizarStatusTela("Aproxime\na Tag", fonte_tela);
                estadoAtual = MODO_OPERACIONAL;
            }

            // 2. Botão Vermelho: Fechamento Manual Forçado
            if (botao_verificar_clique(&btnVermelho) == 1) {
                fecharPorta();
                Serial.println(F("[MANUAL] Trava DESLIGADA via Botão Vermelho"));
                atualizarStatusTela("Fechado!\nLED: OFF", fonte_tela);
                delay(1200);

                atualizarStatusTela("Aproxime\na Tag", fonte_tela);
                estadoAtual = MODO_OPERACIONAL;
            }

            // 3. Botão Branco ou Azul: Exibir Status na Tela sem Fechar
            if (botao_verificar_clique(&btnBranco) == 1 || botao_verificar_clique(&btnAzul) == 1) {
                String statusStr = "LED: ON\nPorta: ";
                statusStr += estaPortaFechada() ? "FECH" : "ABER";
                atualizarStatusTela(statusStr, fonte_tela);
            }
            break;
        }
    }
}