# Registro de uso de IA (copiloto)

Ferramenta usada: Qoder (agente de código), em novembro de 2026.

## O que a IA ajudou a fazer

- Estruturar o `sketch.ino` em funções pequenas (leitura, classificação, alarme, nuvem).
- Revisar o cálculo do HC-SR04 (`duração × 0,0343 / 2`) e o timeout do `pulseIn`.
- Explicar por que o ECHO de 5 V precisa de divisor de tensão (1 kΩ / 2 kΩ) antes do GPIO 18.
- Sugerir mediana de 3 amostras para reduzir leitura falsa no ultrassônico.
- Montar o `diagram.json` do Wokwi e o envio HTTP ao ThingSpeak.
- Organizar o README e as evidências.

## Exemplos de prompts usados

1. "Escreva um sketch ESP32 que leia HC-SR04 e DHT22, acenda LEDs por faixa e envie ao ThingSpeak a cada 15 s, sem travar quando o Wi-Fi cair."
2. "Por que o ECHO do HC-SR04 queima o ESP32 e qual divisor usar?"
3. "Como publicar campos no ThingSpeak com HTTPClient e formulário x-www-form-urlencoded?"
4. "Revise o JSON do serial: preciso de seq, dist_cm, temp_c, hum_pct, status, alert e uptime_ms."
5. "Escreva um README de entrega com problema, funcionamento, pinos, evidências e ODS."

## O que foi decidido pelo grupo (não pela IA)

- Tema, cidadão e ponto monitorado (Av. Brasil).
- Limiares de 10 cm e 20 cm para crítico e atenção.
- Escolha de ThingSpeak em vez de outras plataformas.
- Textos da entrega e do pitch.

A IA não gerou nenhuma credencial; a Write API Key nunca foi versionada.
