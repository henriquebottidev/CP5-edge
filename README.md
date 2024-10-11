# Projeto de Monitoramento com ESP32 e Dashboard

## Descrição do Projeto
Este projeto consiste em um sistema de monitoramento de dados de sensores de temperatura, umidade e luminosidade utilizando um **ESP32**, um **sensor DHT11** e um **sensor LDR**. Os dados coletados são enviados via **protocolo MQTT** para uma **API Python**, que os armazena e exibe em um dashboard interativo desenvolvido com a biblioteca **Dash**. Este dashboard é hospedado em uma máquina virtual **AWS**, acessível através de um endereço IP dedicado.

## Funcionalidades
- Coleta de dados de temperatura, umidade e luminosidade em tempo real
- Transmissão de dados utilizando o protocolo MQTT
- Interface gráfica com gráficos de atualização automática para visualização de dados
- Suporte para histórico de dados

## Estrutura do Projeto
```plaintext
projeto/
│
├── api-sth.py      # Código Python para API e dashboard
├── index.ino       # Código Arduino para ESP32
└── README.md       # Documento com informações do projeto
```

# Pré-requisitos
## 1. Hardware
    - ESP32
    - Sensor DHT11
    - Sensor LDR

## 2. Software
    - Python 3.x
    - Arduino IDE

# Instalação
  ## Passo 1: Configuração do ESP32
      - Configurar os pinos: Conecte o DHT11 e o LDR aos pinos especificados no código index.ino.
      - Rede Wi-Fi: No código index.ino, configure o SSID e PASSWORD da rede Wi-Fi.
      - Broker MQTT: Certifique-se de que o IP do broker MQTT está definido corretamente (verifique com o administrador se necessário).

  ## Passo 2: Instalar Dependências no Dashboard
      - Primeiro, clone este repositório e navegue até a pasta onde o código Python (api-sth.py) está localizado. Em seguida, instale as bibliotecas necessárias com o seguinte comando:

```bash
pip install dash plotly requests pytz
```

  ## Passo 3: Executar a API Python e o Dashboard
      - Configure o IP da máquina virtual AWS no api-sth.py substituindo o IP_ADDRESS pelo endereço correto.
      - Execute a aplicação com o comando abaixo:

```bash
python api-sth.py
```

  ## Passo 4: Carregar o Código para o ESP32
      - Conecte o ESP32 ao seu computador.
      - Abra index.ino na Arduino IDE.
      - Carregue o código para o ESP32.

  ## Passo 5: Acessar o Dashboard
      - Após iniciar a aplicação em api-sth.py, acesse o dashboard no navegador pelo endereço:

```plaintext
http://<IP_DA_SUA_MAQUINA>:8050
```

  # Estrutura do Código
  ## ESP32 (index.ino)
    O código do ESP32 inclui:

    Conexão com o Wi-Fi e o broker MQTT
    Coleta de dados dos sensores DHT11 e LDR
    Publicação dos dados nos tópicos MQTT configurados
    
  ## API e Dashboard (api-sth.py)
    A aplicação em Python inclui:

    Funções para buscar dados de temperatura, umidade e luminosidade
    Conversão dos timestamps para o fuso horário de Lisboa
    Configuração do layout do dashboard com gráficos de luminosidade, temperatura e umidade
    Atualização automática dos gráficos a cada 10 segundos

  ## Troubleshooting
    Caso encontre problemas:

    Erro de conexão com o broker MQTT: Verifique as configurações de IP e porta.
    Erro no dashboard: Certifique-se de que todas as bibliotecas estão instaladas e que o IP está correto.
