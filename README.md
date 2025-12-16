# PMV-IoT
Basic Serial IoT Gateway Architecture scaffold, faster prototyping of ideas and validation. Non comercial ans non secure coding. 

Versión 1.0

Descripción general

Este repositorio presenta una arquitectura desacoplada de dos firmwares, diseñada para habilitar conectividad IoT en equipos especializados que en su diseño original no cuentan o contaron con capacidades nativas de red, pero sí con puerto serial (UART).

El sistema separa de forma estricta: El control del proceso físico (sensores, actuadores, lógica de negocio) de las funciones de conectividad a Internet y cloud

Este enfoque permite prototipado rápido, reutilización del módulo IoT y mínimo impacto sobre sistemas existentes, especialmente en contextos industriales y biomédicos.

Arquitectura del sistema

El sistema está compuesto por dos SoC independientes que se comunican mediante un contrato serial unidireccional:

[ MCU / PIC ]
  └─ Control de proceso
  └─ Sensores y actuadores
  └─ Máquina de estados
  └─ Generación de datos del proceso
          │
          │  UART (clave-valor + sentinela)
          ▼
[ ESP8266 ]
  └─ Gateway IoT
  └─ Recepción serial
  └─ Adaptación de formato
  └─ Envío HTTPS
          │
          ▼
[ Cloud / AWS Lambda ]

Principios de diseño
    Separación de responsabilidades
    Bajo acoplamiento
    No dependencia de red para el proceso físico
    Reutilización del firmware IoT en múltiples equipos
    Compatibilidad con sistemas legacy

Contrato de comunicación serial
    El contrato serial define cómo se intercambia la información entre ambos firmwares. Este contrato es el núcleo del sistema.

Canal: Comunicación UART
    Unidireccional: MCU/PIC → ESP8266
    El ESP8266 no responde comandos

Momento de transmisión
    El envío ocurre una única vez, al finalizar completamente cada proceso y contiene el estado completo del ciclo.

Formato del mensaje
    El MCU envía una secuencia de pares clave–valor, separados por comas: <clave>-<valor>,<clave>-<valor>,...,E

Ejemplo conceptual:
    A-08:32,B-2025/09/14,C-OPER123,...,E

Fin de mensaje
    El carácter E actúa como sentinela de fin de transmisión. El ESP8266 acumula datos hasta detectar este carácter.

Responsabilidades por componente
    MCU / PIC
        Control total del proceso físico
        Lectura de sensores y control de actuadores
        Gestión de estados y tiempos
        Generación de los datos del proceso
        Emisión del mensaje serial según el contrato
    ESP8266
        Recepción del mensaje serial completo
        Interpretación del contrato
        Transformación a un formato adecuado para cloud (ej. JSON)
        Envío de la información vía HTTPS
        No contiene lógica de proceso
        Organización del repositorio


├── README.md        ← Arquitectura y contrato (este documento)
├── pic/             ← Firmware del controlador de proceso
│   └── README.md
└── esp8266/         ← Firmware del gateway IoT
    └── README.md


Cada carpeta contiene su propio README con:
    Detalles técnicos
    Consideraciones de implementación
    Decisiones específicas de cada firmware
    
Objetivo del diseño
    Este proyecto está orientado a:
    Retrofit IoT de maquinaria existente
    Prototipado rápido de dispositivos conectados
    Separación clara entre dominio físico y conectividad
    Escenarios donde modificar el firmware principal implica alto riesgo
    El contrato serial permite que cualquier equipo con UART pueda integrarse al ecosistema IoT sin rediseño del sistema principal.