// ============================================================================
// MÓDULO WI-FI - IMPLEMENTAÇÃO
// ============================================================================
// Executa a estratégia em duas etapas para configurar o IP Fixo no ESP32:
//   1. Conecta temporariamente via DHCP para ler o Gateway e a Máscara de Sub-rede do roteador.
//   2. Reconfigura a interface sem fio aplicando o IP Fixo desejado com os parâmetros capturados.

#include "modulo_wifi.h"
#include <WiFi.h>

// ============================================================================
// FUNÇÃO: wifi_conectar_ip_fixo
// Gerencia o fluxo de conexão em duas fases para garantir IP Fixo e rotas válidas.
// ============================================================================
int wifi_conectar_ip_fixo(const WifiConfig *config) {
    // Programação defensiva: valida se a struct é válida e se o SSID não está vazio[cite: 19]
    if (config == NULL || strlen(config->ssid) == 0) return 0;

    // Converte a string texto do IP (ex: "192.168.1.184") para o tipo de objeto IPAddress[cite: 19]
    IPAddress ipDesejado;
    if (!ipDesejado.fromString(config->ipEstatico)) {
        Serial.println(F("[Wi-Fi] Erro: Formato de IP invalido!"));
        return 0; // Aborta caso a formatação do IP seja inválida[cite: 19]
    }

    // ------------------------------------------------------------------------
    // PASSO 1: Conexão via DHCP para descoberta de rotas da rede local
    // ------------------------------------------------------------------------
    Serial.println(F("[Wi-Fi] Passo 1: Conectando via DHCP para descobrir rotas..."));
    
    WiFi.disconnect(true); // Reseta a interface Wi-Fi limpando estados anteriores[cite: 19]
    WiFi.mode(WIFI_STA);   // Configura o ESP32 exclusivamente no modo Estação (Cliente)[cite: 19]
    WiFi.begin(config->ssid, config->password); // Inicia conexão temporária DHCP[cite: 19]

    // Laço com timeout de até 10 segundos (20 tentativas x 500ms) aguardando conexão[cite: 19]
    uint8_t tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
        delay(500);
        Serial.print(".");
        tentativas++;
    }

    // Aborta se não for possível conectar à rede Wi-Fi informada[cite: 19]
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("\n[Wi-Fi] Erro: Falha ao conectar no modo DHCP!"));
        return 0;
    }

    // ------------------------------------------------------------------------
    // PASSO 2: Captura das configurações dinâmicas e aplicação do IP Fixo
    // ------------------------------------------------------------------------
    // Salva o Gateway, a Máscara de Sub-rede e o DNS atribuídos pelo roteador[cite: 19]
    IPAddress gateway = WiFi.gatewayIP();
    IPAddress subnet = WiFi.subnetMask();
    IPAddress dns = WiFi.dnsIP();

    Serial.println(F("\n[Wi-Fi] Passo 2: Aplicando IP Fixo com dados dinamicos salvos..."));

    // Força o IP Fixo reaproveitando os parâmetros de rota descobertos[cite: 19]
    if (!WiFi.config(ipDesejado, gateway, subnet, dns)) {
        Serial.println(F("[Wi-Fi] Erro ao aplicar IP Fixo!"));
        return 0;
    }

    // Imprime na porta Serial o resumo das rotas ativas na interface[cite: 19]
    Serial.println(F("[Wi-Fi] Conectado com sucesso!"));
    Serial.print(F("IP Fixo: ")); Serial.println(WiFi.localIP());
    Serial.print(F("Gateway: ")); Serial.println(WiFi.gatewayIP());
    Serial.print(F("Subnet: "));  Serial.println(WiFi.subnetMask());

    return 1; // Sucesso na atribuição de IP e conexão[cite: 19]
}

// ============================================================================
// FUNÇÃO: wifi_esta_conectado
// Retorna o estado atual da conexão sem fio.
// ============================================================================
bool wifi_esta_conectado(void) {
    // Retorna true apenas se a camada lógica indicar conexão ativa[cite: 19]
    return (WiFi.status() == WL_CONNECTED);
}