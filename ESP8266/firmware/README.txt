ESP8266 — Serial IoT Gateway Firmware

Versión 1.0

Rol del ESP8266 en el sistema

Este firmware convierte al ESP8266 en un gateway IoT genérico, diseñado para recibir información estructurada por puerto serial (UART) desde cualquier MCU (PIC, AVR, STM32, etc.) y reenviarla a un servicio cloud mediante HTTPS.

El ESP8266 no conoce el proceso, no interpreta datos, y no controla hardware externo.
Su único rol es actuar como adaptador de transporte entre un sistema embebido legacy y la nube.

Principios de diseño
	Separación estricta de responsabilidades
	Reutilización del firmware en múltiples proyectos
	Prototipado rápido
	Compatibilidad con sistemas legacy
	Independencia del dominio de negocio

Este firmware puede reutilizarse en cualquier equipo que:
	Genere datos por UART
	Cumpla el contrato serial definido en el README raíz

Funcionalidades principales
	Conexión WiFi
	Recepción de datos seriales
	Detección de fin de mensaje mediante sentinela
	Parseo genérico clave–valor
	Construcción de JSON neutro
	Envío HTTPS a endpoint cloud (AWS Lambda u otro)
	
Contrato serial esperado
	Canal
		UART
	
Comunicación unidireccional
	MCU → ESP8266
	El ESP8266 no responde comandos

Momento de recepción
	El mensaje se recibe una sola vez
	Corresponde al estado completo del proceso

Formato
	<clave>-<valor>,<clave>-<valor>,...,E
	Separador de pares: ,
	Separador clave–valor: -

Carácter sentinela de fin: E
	Ejemplo:
		A-08:32,B-2025/09/14,C-OPER123,D-ENDO456,E

Flujo interno del firmware
	1. Leer bytes por UART
	2. Acumular en buffer
	3. Detectar sentinela 'E'
	4. Parsear pares clave–valor
	5. Construir JSON genérico
	6. Enviar vía HTTPS
	7. Limpiar buffer

Estructura del JSON generado
	El firmware genera un JSON agrupado y neutro, sin semántica de dominio:
		{
		  "data": {
			"A": "08:32",
			"B": "2025/09/14",
			"C": "OPER123",
			"D": "ENDO456"
		  }
		}

Consideraciones
	Todas las claves y valores se manejan como string
	No se infieren tipos
	No se valida significado
	La interpretación queda delegada al backend
	Arquitectura del firmware

El código está organizado en tres funciones principales, cada una con una única responsabilidad:

leerMensajeSerial()
	Lee datos desde UART
	Acumula hasta detectar el carácter E
	Garantiza mensajes completos

construirPayload()
	Parsea la estructura clave–valor
	Construye JSON válido y agrupado
	No interpreta el contenido

enviarPOST()
	Envía el JSON por HTTPS
	Maneja respuesta HTTP básica
	Configuración externa

Los parámetros sensibles y de entorno se mantienen fuera del firmware principal, en archivos de configuración:
	wifi_config.h → credenciales WiFi
	lambda_config.h → endpoint cloud
	certs.h → certificados (si aplica)
	constants.h → mensajes y constantes

Esto facilita:

Portabilidad

Seguridad

Reutilización

Alcance y limitaciones
Incluido

Gateway serial → cloud

Prototipado rápido
	Integración con equipos legacy no incluido (por diseño) este SoC tiene un uso general, dominio en pic-mcu
	Lógica de proceso
	Control de sensores o actuadores
	Interpretación semántica
	Respuestas al MCU

OTA
	Hardening de seguridad

Casos de uso previstos
	Retrofit IoT de maquinaria existente
	Equipos biomédicos legacy
	Prototipos industriales
	Gateways seriales genéricos
	Adaptadores edge-to-cloud

Estado del firmware
	Funcional
	Modular
	Reutilizable
	Orientado a prototipado

Base para extensiones futuras (OTA, TLS estricto, versionado de contrato)