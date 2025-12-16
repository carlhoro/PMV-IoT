PIC-MCU — Firmware de Control de Proceso

Versión 1.0

Rol del PIC en el sistema
	Este firmware convierte al PIC-MCU en el controlador principal del proceso físico, responsable de:
		Ejecutar la lógica completa del reprocesado
		Leer sensores y entradas físicas
		Controlar actuadores
		Gestionar tiempos y validaciones
		Generar un resumen estructurado del proceso
		El PIC no conoce Internet, no conoce cloud y no depende de conectividad para operar correctamente.

Principios de diseño
	Control local autónomo
	Independencia de red
	Separación estricta de responsabilidades
	Compatibilidad con equipos legacy
	Comunicación mínima y determinista
	El proceso físico siempre se ejecuta, exista o no conectividad externa.

Arquitectura del firmware
	El firmware está organizado alrededor de una Máquina de Estados Finitos (FSM) que representa el ciclo completo del proceso.

Estados del proceso
HOME → END → LP → JE → DES → ALM → HOME

Estado	Descripción: Los estados están definidos en states.h.
	HOME	Estado inicial / reposo
	END	Identificación del endoscopio
	LP	Limpieza previa
	JE	Inmersión en jabón enzimático
	DES	Inmersión en desinfectante
	ALM	Cierre del proceso y envío de datos

Funcionalidades principales
	Lectura de botones físicos
	Lectura de sensor de pH
	Lectura de RFID (operador y endoscopio)
	Control de LEDs y buzzer
	Control de tiempos críticos del proceso
	Visualización en pantalla LCD

Generación del reporte final del ciclo
	Organización del código
	pic/
	├── main.ino        ← FSM y control principal
	├── pins.h          ← Definición de pines
	├── states.h        ← Estados del proceso
	├── config.h        ← Parámetros y tiempos
	├── protocol.h     ← Contrato serial
		Cada archivo cumple una única responsabilidad, facilitando mantenimiento y validación.

Contrato de comunicación serial (PIC → Gateway)
	El PIC envía un único mensaje al finalizar completamente el proceso.

Canal
	UART

Comunicación unidireccional
	PIC → ESP8266
	
Momento de envío
	Al salir del estado ALM
	El mensaje representa todo el ciclo ejecutado

Formato del mensaje
	<clave>-<valor>,<clave>-<valor>,...,E
	Separador de pares: ','
	Separador clave-valor: -
	Sentinela de fin: E
	Ejemplo: A-08:32,B-2025/09/14,C-OPER123,D-ENDO456,E

Las claves están definidas en protocol.h.
	
Generación del reporte del proceso
	Durante la ejecución del ciclo, el PIC va almacenando valores en un arreglo interno (arrayVar[]).
	Al finalizar:
		Se serializa el arreglo
		Se envía por UART
		El PIC no espera respuesta
		El proceso se da por cerrado

Responsabilidades del PIC
	Controlar el proceso físico
	Garantizar tiempos y secuencia
	Validar identificación
	Proveer datos confiables
	Operar sin conectividad

Lo que este firmware NO hace (por diseño)
	No envía datos a Internet
	No interpreta datos externos
	No recibe comandos del gateway
	No depende de WiFi o cloud
	No implementa seguridad de red

Casos de uso previstos
	Equipos biomédicos
	Maquinaria industrial
	Sistemas embebidos legacy
	Procesos donde la validación local es crítica
	Escenarios donde modificar el firmware principal implica alto riesgo

Estado del firmware
	Funcional
	Modular
	Determinista
	Independiente
	Preparado para integración con gateways IoT