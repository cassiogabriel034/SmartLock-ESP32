// ============================================================================
// MÓDULO WI-FI - CABEÇALHO (HEADER)
// ============================================================================
// Interface pública responsável pela gestão da conexão sem fio da SmartLock.
// Suporta a técnica de IP estático com descoberta automática de rotas do roteador.

#ifndef MODULO_WIFI_H
#define MODULO_WIFI_H

#include "config_structs.h"

/**
 * @brief Conecta à rede Wi-Fi local obtendo Gateway, Subnet e DNS via DHCP e forçando o IP Fixo.
 * @param config Ponteiro para a estrutura WifiConfig com SSID, Senha e IP Estático desejado[cite: 20].
 * @return 1 em caso de sucesso na conexão com IP Fixo, 0 em caso de erro de parâmetros ou falha de rede[cite: 20].
 */
int wifi_conectar_ip_fixo(const WifiConfig *config);

/**
 * @brief Verifica de forma não-bloqueante se a conexão Wi-Fi está ativa.
 * @return true se o ESP32 estiver conectado à rede (WL_CONNECTED), false caso contrário[cite: 20].
 */
bool wifi_esta_conectado(void);

#endif // MODULO_WIFI_H