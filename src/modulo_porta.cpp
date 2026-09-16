#include "modulo_porta.h"

// Variável estática para controle interno do estado da trava
static bool travaAtiva = false;

void inicializarPorta() {
    pinMode(PINO_RELE_TRAVA, OUTPUT);
    digitalWrite(PINO_RELE_TRAVA, LOW);

    // INPUT_PULLUP impede que o pino do sensor fique flutuando na leitura
    pinMode(PINO_FIM_DE_CURSO, INPUT_PULLUP);
}

void abrirPorta() {
    digitalWrite(PINO_RELE_TRAVA, HIGH);
    travaAtiva = true;
}

void fecharPorta() {
    digitalWrite(PINO_RELE_TRAVA, LOW);
    travaAtiva = false;
}

bool estaPortaFechada() {
    // Retorna true quando o sensor está pressionado (Nível Lógico 0 / LOW)
    return (digitalRead(PINO_FIM_DE_CURSO) == FIM_DE_CURSO_PRESSIONADO);
}

bool obterEstadoTrava() {
    return travaAtiva;
}