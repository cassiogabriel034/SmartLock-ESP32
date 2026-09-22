/*
 * ============================================================================
 * Projeto: SmartLock ESP32 - Trava Eletrônica Inteligente
 * Arquivo: main.cpp
 * Descrição: Arquivo principal de execução do protótipo. Implementa a lógica 
 *            de controle baseada em Máquina de Estados Finitos (FSM)
 *            assíncrona, gerenciando a leitura NFC/RFID, botoes físicos,
 *            display OLED, controle da trava elétrica, persistência de dados
 *            e comunicação Bluetooth/Wi-Fi.
 * ============================================================================
 */

#include <Arduino.h>
#include "config.h"
#include "config_structs.h"
#include "modulo_botoes.h"
#include "modulo_display.h"
#include "modulo_porta.h"
#include "modulo_nfc.h"
#include "modulo_memoria.h"
#include "modulo_bluetooth.h"
#include "modulo_wifi.h"

#define FONTE_PADRAO 2

// ============================================================================
// DEFINIÇÃO DA MÁQUINA DE ESTADOS FINITOS (FSM)
// Enumeração responsável por mapear todos os estados operacionais da trava
// ============================================================================
typedef enum {
    ESTADO_PADRAO,                 // Modo normal: aguarda leitura de TAGs ou clique em botões
    ESTADO_TRAVA_ABERTA,           // Trava acionada: monitora abertura e fechamento físico da porta
    ESTADO_BLUETOOTH,              // Setup Bluetooth: aguarda recepção de credenciais via aplicativo
    ESTADO_SOLICITAR_TAG_ADMIN_BT, // Aguarda leitura da primeira TAG para definir como Master/Admin
    ESTADO_GERENCIAR_TAG,          // Solicita aprovação da TAG Admin para cadastrar/remover cartões
    ESTADO_AGUARDAR_NOVA_TAG,      // Aguarda encostar a nova TAG para alternar cadastro (toggle)
    ESTADO_RESET_CONFIRMAR,        // Valida se a TAG aproximada é a do Administrador antes do Reset
    ESTADO_RESET                   // Confirmação final via botões para execução do Reset de Fábrica
} EstadoSmartLock;

// ============================================================================
// INSTANCIAÇÃO DE OBJETOS E VARIÁVEIS GLOBAIS
// ============================================================================

// Instâncias para os três botões físicos de controle
Botao btnBranco;   // Botão Verde/Branco: Navegação / Gerenciamento de TAGs
Botao btnVermelho; // Botão Vermelho: Cancelar / Voltar ao modo padrão / Reset
Botao btnAzul;     // Botão Azul: Ativar Modo Bluetooth / Confirmar Ação

// Variável de controle do estado atual do sistema
EstadoSmartLock estadoAtual = ESTADO_PADRAO;

// Estruturas globais mantidas em memória RAM durante as transições de estado
TagNfc tagLida;      // Armazena a última TAG NFC/RFID lida pelo leitor MFRC522
ConfigSistema cfgS;  // Armazena as configurações ativas (Wi-Fi, API, Tag Master)

// ============================================================================
// INICIALIZAÇÃO DO SISTEMA (SETUP)
// ============================================================================
void setup() {
    // Inicializa a comunicação serial para monitoramento (Debug)
    Serial.begin(115200);
    
    // Inicialização dos módulos de hardware e periféricos
    inicializarDisplay();  // Tela OLED SSD1306 (I2C)
    inicializarPorta();    // Controle do Relé da Trava e Sensor Fim de Curso
    inicializarNFC();      // Leitor RFID/NFC MFRC522 (SPI)
    memoria_inicializar(); // Sistema de armazenamento interno na RAM

    // Configuração e associação dos pinos GPIO aos botões de controle
    botao_inicializar(&btnAzul, PINO_BOTAO_AZUL);
    botao_inicializar(&btnBranco, PINO_BOTAO_VERDE);
    botao_inicializar(&btnVermelho, PINO_BOTAO_VERMELHO);

    // Mensagem de boas-vindas no display OLED e no log serial - OTIMIZADO FONTE 2
    atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO); 
    Serial.println("Sistema iniciado. Estado: ESTADO_PADRAO");
}

// ============================================================================
// LAÇO PRINCIPAL DE EXECUÇÃO (LOOP)
// ============================================================================
void loop() {
    // Polling contínuo dos botões para captura de cliques com filtro de debounce
    int cliqueAzul = botao_verificar_clique(&btnAzul);
    int cliqueBranco = botao_verificar_clique(&btnBranco);
    int cliqueVermelho = botao_verificar_clique(&btnVermelho);

    // Execução da máquina de estados principal
    switch (estadoAtual) {
        
        // --------------------------------------------------------------------
        // ESTADO 1: MODO PADRÃO (Aguardando uso routineiro ou navegação)
        // --------------------------------------------------------------------
        case ESTADO_PADRAO: {
            // Tenta efetuar a leitura de uma TAG no campo de radiofrequência
            if (lerTagNFC(&tagLida) == 1) {
                // Consulta se a TAG aproximada consta no banco de dados
                if (memoria_consulta_tag(&tagLida) == 1) {
                    Serial.println("[NFC] TAG Ok. Permissao presente na memoria.");

                    atualizarStatusTela("Acesso OK", FONTE_PADRAO);
                    abrirPorta(); // Energiza o solenoide da trava elétrica
                    estadoAtual = ESTADO_TRAVA_ABERTA;
                } else {
                    Serial.println("[NFC] TAG negada. Permissao ausente na memoria.");
                    
                    atualizarStatusTela("TAG Negada", FONTE_PADRAO);
                    delay(1500); // Pausa para leitura da mensagem pelo usuário
                    atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO);
                }
            }

            // Tratamento dos acionamentos por botão físico no Modo Padrão
            if (cliqueAzul == 1) {
                // Transita para o setup Bluetooth
                estadoAtual = ESTADO_BLUETOOTH;
                bt_inicializar("SmartLock_Setup");
                atualizarStatusTela("Modo BT\nAguardando", FONTE_PADRAO);
            }
            else if (cliqueBranco == 1) {
                // Transita para o gerenciamento de permissões (Adicionar/Remover TAG)
                estadoAtual = ESTADO_GERENCIAR_TAG;
                atualizarStatusTela("Aprox.\nAdmin", FONTE_PADRAO);
            }
            else if (cliqueVermelho == 1) {
                // Transita para o fluxo de redefinição de fábrica
                estadoAtual = ESTADO_RESET_CONFIRMAR;
                atualizarStatusTela("Admin p/\nReset", FONTE_PADRAO);
            }
            break;
        }

        // --------------------------------------------------------------------
        // ESTADO 2: TRAVA ABERTA (Gerenciamento do estado físico da porta)
        // --------------------------------------------------------------------
        case ESTADO_TRAVA_ABERTA: {
            // Quando a porta for fisicamente aberta (sensor desacionado), desativa o relé
            if (!estaPortaFechada()) {
                fecharPorta();
            }
            // Quando a porta for encostada/fechada novamente e o relé já estiver em repouso
            if (estaPortaFechada() && !obterEstadoTrava()) {
                Serial.println("[FSM] Porta fechada. Retornando ao ESTADO_PADRAO.");

                atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO);
                estadoAtual = ESTADO_PADRAO;
            }
            break;
        }

        // --------------------------------------------------------------------
        // ESTADO 3: MODO BLUETOOTH (Recepção de parâmetros de rede)
        // --------------------------------------------------------------------
        case ESTADO_BLUETOOTH: {
            // Monitora a chegada de comandos vindos do aplicativo móvel
            int statusBt = bt_ler_entradas(&cfgS); 
            
            // Retorno '2' indica que a transmissão das configurações foi finalizada com sucesso
            if (statusBt == 2) {
                /* 
                 * NOTA DE INTEGRAÇÃO:
                 * A chamada 'wifi_conectar_ip_fixo' permanece desativada temporariamente.
                 * Redes institucionais/acadêmicas possuem autenticação WPA2-Enterprise 
                 * ou isolamento de clientes (AP Isolation), o que impede a validação direta.
                 */
                // wifi_conectar_ip_fixo(&cfgS.wifi);
                
                bt_desligar(); // Desativa o rádio Bluetooth para economizar RAM/energia
                estadoAtual = ESTADO_SOLICITAR_TAG_ADMIN_BT;
                atualizarStatusTela("Cad. TAG\nAdmin", FONTE_PADRAO);
            }

            // O Botão Vermelho atua como atalho para cancelar e retornar ao modo padrão
            if (cliqueVermelho == 1) {
                bt_desligar();
                atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO);
                estadoAtual = ESTADO_PADRAO;
            }
            break;
        }

        // --------------------------------------------------------------------
        // ESTADO 4: CADASTRO DE TAG ADMINISTRATORIA (Pós-Bluetooth)
        // --------------------------------------------------------------------
        case ESTADO_SOLICITAR_TAG_ADMIN_BT: {
            // Aguarda a leitura da TAG que será registrada como Master/Admin
            if (lerTagNFC(&tagLida) == 1) {
                copiarTag(&cfgS.tagMaster, &tagLida);
                memoria_configurar(&cfgS); // Grava a estrutura de configuração na memória

                atualizarStatusTela("Admin\nSalvo!", FONTE_PADRAO);
                delay(1500);
                atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO);
                estadoAtual = ESTADO_PADRAO;
            }

            // Aborta a operação caso o usuário pressione o Botão Vermelho
            if (cliqueVermelho == 1) {
                atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO);
                estadoAtual = ESTADO_PADRAO;
            }
            break;
        }

        // --------------------------------------------------------------------
        // ESTADO 5: AUTENTICAÇÃO ADMINISTRATIVA PARA GERENCIAR TAGS
        // --------------------------------------------------------------------
        case ESTADO_GERENCIAR_TAG: {
            if (lerTagNFC(&tagLida) == 1) {
                const ConfigSistema *cfg = memoria_obter_config();
                // Valida se a TAG lida corresponde à TAG Master cadastrada
                if (cfg != NULL && compararTags(&tagLida, &cfg->tagMaster)) {
                    atualizarStatusTela("Admin OK!\nAprox. TAG", FONTE_PADRAO);
                    estadoAtual = ESTADO_AGUARDAR_NOVA_TAG;
                } else {
                    atualizarStatusTela("TAG\nInvalida", FONTE_PADRAO);
                    delay(1500);
                    atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO);
                    estadoAtual = ESTADO_PADRAO;
                }
            } 
            
            // Aborta a operação caso o botão vermelho seja pressionado
            if (cliqueVermelho == 1) {
                atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO);
                estadoAtual = ESTADO_PADRAO;
            }
            break;
        }

        // --------------------------------------------------------------------
        // ESTADO 6: CADASTRO / REMOÇÃO DE TAGS SECUNDÁRIAS (TOGGLE)
        // --------------------------------------------------------------------
        case ESTADO_AGUARDAR_NOVA_TAG: {
            if (lerTagNFC(&tagLida) == 1) {
                const ConfigSistema *cfg = memoria_obter_config();
                if (cfg != NULL) {
                    // Se a TAG já existir, é removida; se não existir, é adicionada
                    int resultado = memoria_add_rmv_tag(&cfg->tagMaster, &tagLida);
                    if (resultado == 1) {
                        atualizarStatusTela("TAG\nAdicion.", FONTE_PADRAO);
                    } else if (resultado == 0) {
                        atualizarStatusTela("TAG\nRemovida", FONTE_PADRAO);
                    } else {
                        atualizarStatusTela("Erro\nCadastro", FONTE_PADRAO);
                    }
                }
                delay(1500);
                atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO);
                estadoAtual = ESTADO_PADRAO;
            }

            // Aborta a operação se o botão vermelho for pressionado
            if (cliqueVermelho == 1) {
                atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO);
                estadoAtual = ESTADO_PADRAO;
            }
            break;
        }

        // --------------------------------------------------------------------
        // ESTADO 7: SOLICITAÇÃO DE TAG ADMIN PARA RESET DE FÁBRICA
        // --------------------------------------------------------------------
        case ESTADO_RESET_CONFIRMAR: {
            if (lerTagNFC(&tagLida) == 1) {
                const ConfigSistema *cfg = memoria_obter_config();
                // Exige credencial Master para prosseguir com o reset
                if (cfg != NULL && compararTags(&tagLida, &cfg->tagMaster)) {
                    atualizarStatusTela("Reset?\nA:SIM V:NAO", FONTE_PADRAO);
                    estadoAtual = ESTADO_RESET;
                } else {
                    atualizarStatusTela("Admin\nNegado", FONTE_PADRAO);
                    delay(1500);
                    atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO);
                    estadoAtual = ESTADO_PADRAO;
                }
            }

            // Cancela o fluxo de reset
            if (cliqueVermelho == 1) {
                atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO);
                estadoAtual = ESTADO_PADRAO;
            }
            break;
        }

        // --------------------------------------------------------------------
        // ESTADO 8: CONFIRMAÇÃO E EXECUÇÃO DO RESET DE FÁBRICA
        // --------------------------------------------------------------------
        case ESTADO_RESET: {
            // Botão Azul confirma a exclusão de todos os dados salvos
            if (cliqueAzul == 1) {
                Serial.println("Executando reset...");
                atualizarStatusTela("Resetando.", FONTE_PADRAO);
                memoria_reset_sistema(); // Limpa as TAGs e configurações da memória RAM
                delay(1500);
                atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO);
                estadoAtual = ESTADO_PADRAO;
            } 
            // Botão Vermelho cancela o reset e mantém os dados seguros
            else if (cliqueVermelho == 1) {
                atualizarStatusTela("Padrao\nAprox. TAG", FONTE_PADRAO);
                estadoAtual = ESTADO_PADRAO;
            }
            break;
        }
    }
}