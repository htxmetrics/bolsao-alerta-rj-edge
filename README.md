# Bolsão Alerta RJ — Edge Computing & Computer Systems

Protótipo de IoT (ESP32 + HC-SR04 + DHT22) que mede o nível da água em um ponto de alagamento, decide **na borda** (normal / atenção / crítico) e publica as leituras no **ThingSpeak**.

> **ANTES DE ENTREGAR:** substitua a linha do link do Wokwi abaixo pelo link do projeto salvo (Wokwi → **SAVE** → **SHARE** → copiar link). O restante já está pronto.

## Integrantes

| Nome completo | RM |
|---|---|
| Heitor Anacleto Araujo | RM573599 |
| Matheus Henrique Pedersen Guerra | RM571197 |
| Marcos Vinicius | RM571080 |

## Links

- **Wokwi (projeto funcional):** `COLE-AQUI-O-LINK-DO-WOKWI`
- **ThingSpeak (canal público):** https://thingspeak.com/channels/3586321

## Ficha da CitizenTech

- **Nome:** Bolsão Alerta RJ
- **Cidade / bairro:** Rio de Janeiro — Avenida Brasil e vias do subúrbio carioca
- **Cidadão beneficiado:** motoristas de aplicativo, motoboys e passageiros de ônibus que cruzam a Av. Brasil todos os dias
- **Problema, em uma frase:** os bolsões d'água fecham vias em minutos e o aviso chega tarde, quando o motorista já está dentro da água
- **Solução:** sensor na rua classifica o nível localmente e publica o dado para que o cidadão veja o risco antes de sair
- **ODS:** ODS 11 (cidades e comunidades sustentáveis) e ODS 13 (ação contra a mudança global do clima)

## O problema

Em chuvas de verão, pontos baixos da Av. Brasil acumulam água rapidamente. Quem dirige descobre o alagamento quando já está preso nele, e os órgãos públicos não medem o nível da água ponto a ponto em tempo hábil. Falta um sensor barato, que funcione sozinho na rua e avise antes.

## Como funciona

1. **HC-SR04** mede a distância até a água (instalado a 60 cm do chão).
2. **DHT22** mede temperatura e umidade do ar, complementando o contexto do ponto.
3. O **ESP32** classifica cada leitura **na borda**, sem depender da internet:

   | Distância | Nível (60 − distância) | Estado | LED | Buzzer |
   |---|---|---|---|---|
   | acima de 20 cm | abaixo de 40 cm | NORMAL | verde | desligado |
   | entre 10 e 20 cm | 40 a 50 cm | ATENÇÃO | amarelo | desligado |
   | abaixo de 10 cm | acima de 50 cm | CRÍTICO | vermelho | pulsa a 2 kHz |

4. A cada **15 s** o ESP32 envia as leituras por Wi-Fi (Wokwi-GUEST) para o **ThingSpeak**, como formulário `api_key` + `field1..field6`.
5. O serial publica um JSON por segundo (`seq`, `dist_cm`, `temp_c`, `hum_pct`, `status`, `alert`, `uptime_ms`), usado como evidência e pelas demais disciplinas.

Leituras de distância usam **mediana de 3 amostras válidas** (até 5 tentativas, timeout de 30 ms) para reduzir ruído do sensor ultrassônico.

### Campos no ThingSpeak

| Campo | Conteúdo |
|---|---|
| field1 | distância até a água (cm) |
| field2 | nível da água (cm) |
| field3 | severidade (0 = normal, 1 = atenção, 2 = crítico) |
| field4 | alerta (0 = não, 1 = sim) |
| field5 | temperatura (°C) |
| field6 | umidade (%) |

## Componentes e pinos

| Componente | Pino ESP32 | Observação |
|---|---|---|
| HC-SR04 TRIG | GPIO 5 | pulso de 10 µs |
| HC-SR04 ECHO | GPIO 18 | **divisor de tensão 1 kΩ / 2 kΩ** (ECHO em 5 V) |
| DHT22 SDA | GPIO 15 | biblioteca DHT sensor library |
| LED verde | GPIO 4 | resistor 220 Ω |
| LED amarelo | GPIO 25 | resistor 220 Ω |
| LED vermelho | GPIO 26 | resistor 220 Ω |
| Buzzer | GPIO 27 | pulsos de 300 ms a 2 kHz no crítico |

Circuito completo em [`src/diagram.json`](src/diagram.json); bibliotecas em [`src/libraries.txt`](src/libraries.txt).

## Evidências

| Arquivo | O que mostra |
|---|---|
| [01-wokwi-normal.png](evidencias/01-wokwi-normal.png) | simulação em 40,3 cm — `NORMAL`, LED verde, serial com distância, temperatura e umidade |
| [02-wokwi-atencao.png](evidencias/02-wokwi-atencao.png) | simulação em 15,0 cm — `ATENCAO`, LED amarelo |
| [03-wokwi-critico.png](evidencias/03-wokwi-critico.png) | simulação em 8,0 cm — `CRITICO`, LED vermelho e buzzer |
| [04-thingspeak-canal.png](evidencias/04-thingspeak-canal.png) | canal público com os gráficos de distância, nível, severidade e alerta |

A alteração dos valores simulados (slider do HC-SR04 no Wokwi: 40 → 15 → 8 cm) está registrada nas três primeiras imagens, com a resposta do dispositivo em cada estado.

## Como reproduzir

1. Abra o projeto no Wokwi (link acima) e clique em **Start the simulation**.
2. Aguarde o serial mostrar `# wifi conectado em 10.10.0.2` e as linhas JSON com `temp_c`/`hum_pct` preenchidos.
3. Clique no sensor HC-SR04 e mova o **Distance** para 15 cm e depois 8 cm — LED e buzzer respondem, o serial muda de `NORMAL` para `ATENCAO` e `CRITICO`.
4. Para publicar no seu próprio canal, troque `SEU_WRITE_KEY` em `src/sketch.ino` pela Write API Key do canal (a chave não é versionada) e reinicie.

## Estrutura do repositório

```
src/sketch.ino      código-fonte completo do ESP32
src/diagram.json    circuito do Wokwi
src/libraries.txt   bibliotecas usadas
evidencias/         imagens da simulação e do ThingSpeak
prompts.md          registro do uso de IA como copiloto
```

## Uso de IA

O grupo usou IA como copiloto para revisar o código, conferir o divisor de tensão e organizar a documentação. O registro está em [`prompts.md`](prompts.md); cada integrante sabe explicar as linhas do `sketch.ino`.
