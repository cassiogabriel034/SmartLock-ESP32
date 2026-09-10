#ifndef CONFIG_STRUCTS_H
#define CONFIG_STRUCTS_H

#include <Arduino.h>
#include "modulo_nfc.h"

// Estrutura para os dados da rede Wi-Fi Local
typedef struct {
    char ssid[32];
    char password[64];
    char ipEstatico[16]; // Ex: "192.168.1.184"
} WifiConfig;

// Estrutura para as credenciais do Dashboard
typedef struct {
    char usuario[32];
    char senha[32];
} DashboardConfig;

// Estrutura consolidada do sistema
typedef struct {
    WifiConfig wifi;
    DashboardConfig dashboard;
    char apiLink[128];     // Link normal da API
    TagNfc tagMaster;      // Tag NFC Mestra cadastrada no setup
    bool configurado;      // Indica se o setup inicial foi concluído
} ConfigSistema;

#endif // CONFIG_STRUCTS_H