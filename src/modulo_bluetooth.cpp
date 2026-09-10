// ============================================================================
// MÓDULO BLUETOOTH - IMPLEMENTAÇÃO
// ============================================================================
// Executa o parser de comandos de texto recebidos via Bluetooth Serial durante
// a etapa de configuração inicial do sistema.

#include "modulo_bluetooth.h"
#include <BluetoothSerial.h>

// Instância estática do driver Bluetooth Serial (visível apenas neste arquivo)
static BluetoothSerial SerialBT;

// Flag interna para controle de estado do módulo
static bool btInicializado = false;

// ============================================================================
// FUNÇÃO: bt_inicializar
// Liga o rádio Bluetooth com o nome especificado para transmissão SPP.
// ============================================================================
int bt_inicializar(const char* nomeDispositivo) {
    // Evita reinicializações desnecessárias se o rádio já estiver ativo
    if (btInicializado) return 1;

    // Tenta iniciar a pilha Bluetooth do ESP32 com o nome informado
    if (!SerialBT.begin(nomeDispositivo)) {
        Serial.println(F("[BT] Erro ao iniciar Bluetooth."));
        return -1; // Retorna falha de inicialização por hardware/driver
    }

    btInicializado = true;
    Serial.println(F("[BT] Aguardando comandos de configuracao..."));
    return 1; // Sucesso
}

// ============================================================================
// FUNÇÃO: bt_verificar_conexao
// Retorna se um smartphone/terminal está ativamente conectado ao ESP32.
// ============================================================================
int bt_verificar_conexao(void) {
    if (!btInicializado) return -1;
    
    // Retorna 1 caso haja um cliente pareado e conectado via SPP
    return SerialBT.hasClient() ? 1 : 0;
}

// ============================================================================
// FUNÇÃO: bt_ler_entradas
// Ouve a porta Serial Bluetooth, processa comandos de texto e atualiza a struct.
// Comandos aceitos:
//   - SET_WIFI:<SSID>,<SENHA>,<IP_FIXO>
//   - SET_API:<LINK_DA_API>
//   - SET_DASH:<USUARIO>,<SENHA>
//   - SALVAR_FIM
// ============================================================================
int bt_ler_entradas(ConfigSistema *configs) {
    // Validação defensiva: verifica se o módulo foi iniciado, se há dados no buffer e se o ponteiro é válido
    if (!btInicializado || !SerialBT.available() || configs == NULL) {
        return 0; // Nenhum dado pendente para leitura
    }

    // Lê a mensagem recebida até encontrar a quebra de linha '\n'
    String comando = SerialBT.readStringUntil('\n');
    comando.trim(); // Remove caracteres invisíveis (como '\r' ou espaços nas extremidades)

    // ------------------------------------------------------------------------
    // COMANDO 1: Configuração do Wi-Fi Local (SSID, Senha, IP Fixo)
    // Formato esperado: SET_WIFI:MinhaRede,Senha123,192.168.1.184
    // ------------------------------------------------------------------------
    if (comando.startsWith("SET_WIFI:")) {
        String payload = comando.substring(9); // Extrai o texto após "SET_WIFI:"
        int pos1 = payload.indexOf(',');      // Localiza a primeira vírgula (divisão entre SSID e Senha)
        int pos2 = payload.lastIndexOf(',');  // Localiza a última vírgula (divisão entre Senha e IP)

        // Garante que existem exatamente duas vírgulas delimitadoras e em posições válidas
        if (pos1 != -1 && pos2 != -1 && pos1 != pos2) {
            String ssid = payload.substring(0, pos1);
            String pass = payload.substring(pos1 + 1, pos2);
            String ip   = payload.substring(pos2 + 1);

            // Copia as Strings lidas de forma segura para os buffers de char da struct
            strncpy(configs->wifi.ssid, ssid.c_str(), sizeof(configs->wifi.ssid));
            strncpy(configs->wifi.password, pass.c_str(), sizeof(configs->wifi.password));
            strncpy(configs->wifi.ipEstatico, ip.c_str(), sizeof(configs->wifi.ipEstatico));

            SerialBT.println("OK: Wi-Fi atualizado!");
            return 1; // Atualização bem-sucedida
        }
    }

    // ------------------------------------------------------------------------
    // COMANDO 2: Configuração da URL da API Central
    // Formato esperado: SET_API:http://192.168.1.100:3000/api/v1
    // ------------------------------------------------------------------------
    if (comando.startsWith("SET_API:")) {
        String api = comando.substring(8); // Extrai o link após "SET_API:"
        
        // Copia a URL informada para a variável da struct
        strncpy(configs->apiLink, api.c_str(), sizeof(configs->apiLink));

        SerialBT.println("OK: API Link atualizado!");
        return 1; // Atualização bem-sucedida
    }

    // ------------------------------------------------------------------------
    // COMANDO 3: Credenciais do Dashboard (Usuário e Senha)
    // Formato esperado: SET_DASH:admin,senha123
    // ------------------------------------------------------------------------
    if (comando.startsWith("SET_DASH:")) {
        String payload = comando.substring(9); // Extrai o texto após "SET_DASH:"
        int pos = payload.indexOf(',');        // Localiza a vírgula que separa o usuário da senha

        if (pos != -1) {
            String usr = payload.substring(0, pos);
            String pwd = payload.substring(pos + 1);

            // Salva o usuário e a senha do Dashboard na struct
            strncpy(configs->dashboard.usuario, usr.c_str(), sizeof(configs->dashboard.usuario));
            strncpy(configs->dashboard.senha, pwd.c_str(), sizeof(configs->dashboard.senha));

            SerialBT.println("OK: Credenciais Dashboard atualizadas!");
            return 1; // Atualização bem-sucedida
        }
    }

    // ------------------------------------------------------------------------
    // COMANDO 4: Encerramento do Setup Inicial
    // Formato esperado: SALVAR_FIM
    // ------------------------------------------------------------------------
    if (comando == "SALVAR_FIM") {
        configs->configurado = true; // Seta a flag indicando que a configuração foi concluída
        SerialBT.println("OK: Configuracao finalizada!");
        return 2; // Retorna código especial '2' para sinalizar a conclusão do ciclo de setup
    }

    // Tratamento para comandos com sintaxe incorreta ou não reconhecidos
    SerialBT.println("Erro: Comando invalido.");
    return -1;
}

// ============================================================================
// FUNÇÃO: bt_desligar
// Desativa o rádio Bluetooth e libera o consumo de memória RAM no ESP32.
// ============================================================================
int bt_desligar(void) {
    if (!btInicializado) return 1;

    SerialBT.end(); // Encerra o stack Bluetooth no sistema operacional
    btInicializado = false;
    Serial.println(F("[BT] Modulo desligado."));
    return 1; // Sucesso
}