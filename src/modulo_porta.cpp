#include "modulo_porta.h"

// Definição da lógica Active-LOW (Comum em módulos relé de protoboard)
#define RELE_LIGAR    LOW   // Sinal LOW energiza o relé e LIGA o LED
#define RELE_DESLIGAR HIGH  // Sinal HIGH desenergiza o relé e DESLIGA o LED

static bool travaAtiva = false;

void inicializarPorta() {
    pinMode(PINO_RELE_TRAVA, OUTPUT);
    // Garante que o relé inicie DESLIGADO
    digitalWrite(PINO_RELE_TRAVA, RELE_DESLIGAR);
    travaAtiva = false;

    // Configura o sensor fim de curso com pull-up interno
    pinMode(PINO_FIM_DE_CURSO, INPUT_PULLUP);
}

void abrirPorta() {
    digitalWrite(PINO_RELE_TRAVA, RELE_LIGAR);
    travaAtiva = true;
}

void fecharPorta() {
    digitalWrite(PINO_RELE_TRAVA, RELE_DESLIGAR);
    travaAtiva = false;
}

bool estaPortaFechada() {
    // Retorna true quando a chave de fim de curso é pressionada (LOW)
    return (digitalRead(PINO_FIM_DE_CURSO) == LOW);
}

bool obterEstadoTrava() {
    return travaAtiva;
}