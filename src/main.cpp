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

typedef enum {
    ESTADO_PADRAO,
    ESTADO_TRAVA_ABERTA,
    ESTADO_BLUETOOTH,
    ESTADO_SOLICITAR_TAG_ADMIN_BT,
    ESTADO_GERENCIAR_TAG,
    ESTADO_AGUARDAR_NOVA_TAG,
    ESTADO_RESET_CONFIRMAR,
    ESTADO_RESET
} EstadoSmartLock;

Botao btnBranco;
Botao btnVermelho;
Botao btnAzul;

EstadoSmartLock estadoAtual = ESTADO_PADRAO;

// Declaração global para persistência entre estados
TagNfc tagLida;
ConfigSistema cfgS;

void setup() {
  Serial.begin(115200);
  
  inicializarDisplay();
  inicializarPorta();
  inicializarNFC();
  memoria_inicializar();  

  botao_inicializar(&btnAzul, PINO_BOTAO_AZUL);
  botao_inicializar(&btnBranco, PINO_BOTAO_VERDE);
  botao_inicializar(&btnVermelho, PINO_BOTAO_VERMELHO);

  atualizarStatusTela("Modo Padrao\nAproxime TAG", 1); 
  Serial.println("Sistema iniciado. Estado: ESTADO_PADRAO");
}

void loop() {
  int cliqueAzul = botao_verificar_clique(&btnAzul);
  int cliqueBranco = botao_verificar_clique(&btnBranco);
  int cliqueVermelho = botao_verificar_clique(&btnVermelho);

  switch (estadoAtual) {
    
    // --------------------------------------------------------------------
    // MODO PADRÃO
    // --------------------------------------------------------------------
    case ESTADO_PADRAO: {
      if (lerTagNFC(&tagLida) == 1) {
        if (memoria_consulta_tag(&tagLida) == 1) {
          atualizarStatusTela("Acesso Permitido", 1);
          abrirPorta();
          estadoAtual = ESTADO_TRAVA_ABERTA;
        } else {
          atualizarStatusTela("TAG Negada", 1);
          delay(1500);
          atualizarStatusTela("Modo Padrao\nAproxime TAG", 1);
        }
      }

      if (cliqueAzul == 1) {
        estadoAtual = ESTADO_BLUETOOTH;
        bt_inicializar("SmartLock_Setup");
        atualizarStatusTela("Modo Bluetooth\nAguardando...", 1);
      }
      else if (cliqueBranco == 1) {
        estadoAtual = ESTADO_GERENCIAR_TAG;
        atualizarStatusTela("Aproxime Admin", 1);
      }
      else if (cliqueVermelho == 1) {
        estadoAtual = ESTADO_RESET_CONFIRMAR;
        atualizarStatusTela("Ler Admin p/\nReset", 1);
      }
      break;
    }

    // --------------------------------------------------------------------
    // CONTROLE ASSÍNCRONO DA PORTA
    // --------------------------------------------------------------------
    case ESTADO_TRAVA_ABERTA: {
      // Quando a porta for fisicamente aberta, desliga a trava elétrica
      if (!estaPortaFechada()) {
        fecharPorta();
      }
      // Quando a porta for fechada novamente e a trava já estiver desligada
      if (estaPortaFechada() && !obterEstadoTrava()) {
        atualizarStatusTela("Modo Padrao\nAproxime TAG", 1);
        estadoAtual = ESTADO_PADRAO;
      }
      break;
    }

    // --------------------------------------------------------------------
    // MODO BLUETOOTH E CADASTRO DE TAG ADMIN
    // --------------------------------------------------------------------
    case ESTADO_BLUETOOTH: {
      int statusBt = bt_ler_entradas(&cfgS); 
      
      if (statusBt == 2) {
        // wifi_conectar_ip_fixo(&cfgS.wifi);
        bt_desligar();
        estadoAtual = ESTADO_SOLICITAR_TAG_ADMIN_BT;
        atualizarStatusTela("Cadastre Tag Admin", 1);
      }

      if (cliqueVermelho == 1) {
        bt_desligar();
        atualizarStatusTela("Modo Padrao\nAproxime TAG", 1);
        estadoAtual = ESTADO_PADRAO;
      }
      break;
    }

    case ESTADO_SOLICITAR_TAG_ADMIN_BT: {
      if (lerTagNFC(&tagLida) == 1) {
        copiarTag(&cfgS.tagMaster, &tagLida);
        memoria_configurar(&cfgS);

        atualizarStatusTela("Admin Salvo!", 1);
        delay(1500);
        atualizarStatusTela("Modo Padrao\nAproxime TAG", 1);
        estadoAtual = ESTADO_PADRAO;
      }

      if (cliqueVermelho == 1) {
        atualizarStatusTela("Modo Padrao\nAproxime TAG", 1);
        estadoAtual = ESTADO_PADRAO;
      }
      break;
    }

    // --------------------------------------------------------------------
    // GERENCIAMENTO DE TAGS (ADICIONAR / REMOVER)
    // --------------------------------------------------------------------
    case ESTADO_GERENCIAR_TAG: {
      if (lerTagNFC(&tagLida) == 1) {
        const ConfigSistema *cfg = memoria_obter_config();
        if (cfg != NULL && compararTags(&tagLida, &cfg->tagMaster)) {
          atualizarStatusTela("Admin Validado!\nAproxime Nova TAG", 1);
          estadoAtual = ESTADO_AGUARDAR_NOVA_TAG;
        } else {
          atualizarStatusTela("TAG Invalida", 1);
          delay(1500);
          atualizarStatusTela("Modo Padrao\nAproxime TAG", 1);
          estadoAtual = ESTADO_PADRAO;
        }
      } 
      
      if (cliqueVermelho == 1) {
        atualizarStatusTela("Modo Padrao\nAproxime TAG", 1);
        estadoAtual = ESTADO_PADRAO;
      }
      break;
    }

    case ESTADO_AGUARDAR_NOVA_TAG: {
      if (lerTagNFC(&tagLida) == 1) {
        const ConfigSistema *cfg = memoria_obter_config();
        if (cfg != NULL) {
          int resultado = memoria_add_rmv_tag(&cfg->tagMaster, &tagLida);
          if (resultado == 1) {
            atualizarStatusTela("TAG Adicionada", 1);
          } else if (resultado == 0) {
            atualizarStatusTela("TAG Removida", 1);
          } else {
            atualizarStatusTela("Erro Cadastro", 1);
          }
        }
        delay(1500);
        atualizarStatusTela("Modo Padrao\nAproxime TAG", 1);
        estadoAtual = ESTADO_PADRAO;
      }

      if (cliqueVermelho == 1) {
        atualizarStatusTela("Modo Padrao\nAproxime TAG", 1);
        estadoAtual = ESTADO_PADRAO;
      }
      break;
    }

    // --------------------------------------------------------------------
    // RESET DE FÁBRICA
    // --------------------------------------------------------------------
    case ESTADO_RESET_CONFIRMAR: {
      if (lerTagNFC(&tagLida) == 1) {
        const ConfigSistema *cfg = memoria_obter_config();
        if (cfg != NULL && compararTags(&tagLida, &cfg->tagMaster)) {
          atualizarStatusTela("Confirma Reset?\nAzul:SIM  Verm:NAO", 1);
          estadoAtual = ESTADO_RESET;
        } else {
          atualizarStatusTela("Admin Negado", 1);
          delay(1500);
          atualizarStatusTela("Modo Padrao\nAproxime TAG", 1);
          estadoAtual = ESTADO_PADRAO;
        }
      }

      if (cliqueVermelho == 1) {
        atualizarStatusTela("Modo Padrao\nAproxime TAG", 1);
        estadoAtual = ESTADO_PADRAO;
      }
      break;
    }
    
    case ESTADO_RESET: {
      if (cliqueAzul == 1) {
        Serial.println("Executando reset...");
        atualizarStatusTela("A redefinir...", 1);
        memoria_reset_sistema();
        delay(1500);
        atualizarStatusTela("Modo Padrao\nAproxime TAG", 1);
        estadoAtual = ESTADO_PADRAO;
      } else if (cliqueVermelho == 1) {
        atualizarStatusTela("Modo Padrao\nAproxime TAG", 1);
        estadoAtual = ESTADO_PADRAO;
      }
      break;
    }
  }
}