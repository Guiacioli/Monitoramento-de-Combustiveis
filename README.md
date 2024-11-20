# Sistema de Monitoramento de Combustíveis com Sensor de Etanol e Controle de Bomba

Este projeto implementa um sistema de monitoramento da qualidade do combustível (etanol/gasolina) utilizando um sensor de etanol, Arduino e comunicação MQTT. O sistema controla automaticamente uma bomba submersível dependendo da qualidade do combustível detectado, e permite monitoramento e controle remoto via Node-RED.

## Código e Documentação

Os códigos principais estão como Arduno.ino e ESP-01.ino e foram desenvolvido na **Arduino IDE**. Os arquivos incluem a configuração da conexão Wi-Fi, envio de dados via MQTT e controle da bomba submersível com base nas leituras do sensor de etanol.

## Hardware

- **Placa Arduino Uno com Wi-Fi**: Utilizada para controle do sistema e envio de dados via Wi-Fi.
- **Módulo ESP-01**: Utilizado para comunicação Wi-Fi com o broker MQTT.
- **Sensor de Etanol (VW Nivus 2023)**: Responsável pela coleta de dados sobre a qualidade do combustível.
- **Relé de 5V**: Controla a bomba submersível com base nas condições do combustível.
- **Display LCD 20x4 I2C**: Exibe informações em tempo real sobre o percentual de etanol e o estado da bomba.

## Interfaces e Protocolos de Comunicação

O sistema utiliza o protocolo **MQTT** para comunicação com a plataforma Node-RED, e o **protocolo TCP/IP** para conexão via rede Wi-Fi. São utilizados os seguintes tópicos MQTT:

- **`sensor/etanol`**: Envia o percentual de etanol detectado pelo sensor.
- **`estado/bomba`**: Indica o estado da bomba (ligada ou desligada).
- **`tipo/combustivel`**: Indica o tipo de combustível (gasolina, etanol ou adulterado).

Além disso, a plataforma Node-RED é configurada para monitorar e controlar a bomba remotamente através de botões virtuais para **Ligar** e **Desligar** a bomba, utilizando os tópicos MQTT correspondentes.

## Protocolo MQTT e Comunicação TCP/IP

A comunicação do sistema é feita pela rede Wi-Fi (TCP/IP) e utiliza o protocolo MQTT para envio e recebimento de dados. A configuração inclui:

- **Credenciais do Wi-Fi**: Configure o `wifi_ssid` e `wifi_senha` no código para conexão com a rede.
- **Broker MQTT**: O sistema está configurado para usar o broker público `test.mosquitto.org`. Caso queira usar um broker privado, configure o endereço IP e a porta no código.
- **Tópicos MQTT**: São configurados os tópicos de **sensor de etanol**, **estado da bomba**, e **tipo de combustível**.
