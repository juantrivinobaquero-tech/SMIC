# Semáforo Inteligente - ESP32

Este proyecto consiste en la implementación de un sistema de semaforización inteligente basado en el microcontrolador **ESP32**. La iniciativa busca optimizar el flujo vehicular mediante el uso de tecnología embebida, permitiendo una gestión más eficiente y dinámica de los tiempos de respuesta en vía.

## Descripción
El sistema utiliza las capacidades de procesamiento y conectividad del ESP32 para gestionar un semáforo que no depende únicamente de temporizadores fijos, sino que está diseñado para integrarse en entornos de movilidad inteligente. Se enfoca en la estabilidad del sistema y la correcta gestión de estados para garantizar una transición segura y lógica.

## Características Principales
* **Control de Estados:** Implementación de lógica de transición de luces (Rojo, Amarillo, Verde) optimizada.
* **Conectividad:** Uso de las funciones integradas del ESP32 para monitoreo o control remoto.
* **Gestión de Tiempos:** Manejo eficiente de delays o interrupciones para una respuesta precisa.

##️ Tecnologías Utilizadas
* **Hardware:** ESP32..
* **Herramientas:** Git para control de versiones.

## Autores
Proyecto desarrollado por:
* **Juan José Triviño**
* **Carlos Jose Castiblanco**


## Librerías Utilizadas:
* WiFi.h: Gestiona la conectividad de red de la ESP32.
* PubSubClient.h: Implementa el protocolo MQTT para comunicación con brokers externos.

## Funciones Implementadas:

### 1. Detección de Presencia (hayAutos_XX())
Monitorea los sensores infrarrojos delanteros para determinar si hay vehículos detenidos esperando el cambio de luz.
* Lógica: Utiliza un temporizador de 3 segundos para confirmar que el vehículo está efectivamente esperando.
* Retorno: Devuelve un valor booleano (true/false) que activa la lógica de prioridad.

### 2. Conteo de Flujo Vehicular (contar_autos_XX())
Registra el volumen de tráfico de forma independiente para cada punto cardinal.
* Filtro de Ruido: Implementa una validación de 150ms mediante millis() para asegurar que cada incremento corresponda a un vehículo real.
* Detección de Flancos: Compara el estado actual con el anterior para detectar con precisión el cruce de cada unidad por el sensor.

##  Configuración de Hardware
El sistema utiliza arreglos de enteros para gestionar los pines de los LEDs (SEM_Norte, SEM_Sur, etc.) y define entradas específicas para sensores de proximidad infrarrojos en posiciones estratégicas (Adelante/Atrás).
---

