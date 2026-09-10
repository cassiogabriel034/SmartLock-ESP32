// ============================================================================
// MÓDULO BOTOES - IMPLEMENTAÇÃO
// ============================================================================
// Executa a leitura de borda de descida (solto -> pressionado) ignorando o ruído
// mecânico (bounce) por meio de temporização assíncrona com millis().

#include "modulo_botoes.h"

// ============================================================================
// FUNÇÃO: botao_inicializar
// Configura os valores padrão da struct e ativa o Pull-Up interno da GPIO.
// ============================================================================
void botao_inicializar(Botao *b, int pinoGpio) {
    if (b == NULL) return;

    b->pino = pinoGpio;
    b->ultimoEstado = HIGH;          // Com INPUT_PULLUP, o estado padrão é HIGH (desconectado/solto)
    b->pressionado = false;          // Inicia como não pressionado
    b->ultimoTempoDebounce = 0;
    b->tempoDebounce = 25;           // 50ms para estabilização do ruído mecânico

    pinMode(b->pino, INPUT_PULLUP);  // Habilita o resistor interno de Pull-Up do ESP32
}

// ============================================================================
// FUNÇÃO: botao_verificar_clique
// Processa a leitura do botão e retorna o evento de clique filtrado.
// ============================================================================
int botao_verificar_clique(Botao *b) {
    // Retorno -1: Erro de parâmetro (Ponteiro inválido)
    if (b == NULL) return -1;

    int leitura = digitalRead(b->pino);
    int retorno = 0; // Valor padrão: 0 (Não clique)

    // Se houve variação no pino em relação à leitura anterior, reinicia a contagem do debounce
    if (leitura != b->ultimoEstado) {
        b->ultimoTempoDebounce = millis();
        b->ultimoEstado = leitura;
    }

    // Se o sinal se manteve estável pelo tempo especificado em tempoDebounce
    if ((millis() - b->ultimoTempoDebounce) > b->tempoDebounce) {
        // Detecta a transição (LOW = Pressionado devido ao INPUT_PULLUP)
        if (leitura == LOW && !b->pressionado) {
            b->pressionado = true; // Marca que o botão está afundado
            retorno = 1;           // Retorno 1: Sim clique
        } 
        // Quando o botão for solto (retorna para HIGH), libera para um novo clique
        else if (leitura == HIGH) {
            b->pressionado = false;
        }
    }

    // Retorna 0 (Sem clique) ou 1 (Clique detectado)
    return retorno;
}