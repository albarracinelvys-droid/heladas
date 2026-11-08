# Detección temprana de heladas V2.0 🌡️💨

Firmware para **ATmega328P a 16 MHz**, Arduino Uno o Nano clásico, con DS18B20 y anemómetro de pulsos. Mide cada 2 segundos, muestra datos CSV por USB y activa indicadores y buzzer. Incluye demostración sin sensores y compilación automática en GitHub Actions.

## Estado del proyecto

El repositorio original contenía únicamente un proyecto de edición `.clipchamp`. Ese archivo se conserva; referencia archivos multimedia externos y no contiene el firmware ni el video completo. Esta es una implementación nueva basada en temperatura y viento, no una recuperación del código del video.

El programa se puede compilar y cargar. **La velocidad del viento requiere calibración antes de usar sus valores como mediciones.** La validación física del montaje queda pendiente de realizar en la placa del usuario.

## Componentes y conexiones

| Componente | Conexión en Uno/Nano |
|---|---|
| DS18B20 VDD / GND | 5 V / GND |
| DS18B20 DATA | D4 y resistencia de 4,7 kΩ entre DATA y 5 V |
| Anemómetro de contacto seco o colector abierto | Señal D2, retorno GND; pull-up interno habilitado |
| LED verde | D6 → resistencia 330 Ω → ánodo; cátodo GND |
| LED amarillo | D7 → resistencia 330 Ω → ánodo; cátodo GND |
| LED rojo | D8 → resistencia 330 Ω → ánodo; cátodo GND |
| Piezo pasivo de baja corriente | D9 / GND |

Verifica el pinout de tu DS18B20: los colores de los cables varían. Usa alimentación de tres hilos, no alimentación parásita. No conectes señales de 12/24 V a D2; adapta el nivel según la ficha del anemómetro. Para una sirena usa un transistor y alimentación apropiada, no el pin directamente.

Un ATmega328P suelto necesita circuito de alimentación, desacoplo, reloj de 16 MHz, reset, bootloader y adaptador USB-serie equivalentes a Uno. Los números de esta tabla son pines Arduino, no patas físicas del chip.

## Compilar y cargar

Instala Python y ejecuta en la carpeta del repositorio:

```sh
python -m pip install platformio==6.2.0
python -m platformio run
python -m platformio run -e uno -t upload
python -m platformio device monitor -b 9600
```

Para Nano clásico, cambia `-e uno` por `-e nano`. El entorno Nano usa el bootloader antiguo; si tu Nano tiene el bootloader nuevo, añade `upload_speed = 115200` al entorno `nano`. Si hay varios puertos, añade `--upload-port COM3` al comando de carga y `--port COM3` al monitor, usando tu puerto real. Cierra el monitor antes de cargar.

### Demostración sin sensores

```sh
python -m platformio run -e demo -t upload
python -m platformio device monitor -b 9600
```

Recorre NORMAL, VIGILANCIA, ALERTA, HELADA, recuperación y ERROR_SENSOR cada dos segundos. Usa la misma placa Uno; no necesita sensores, pero los LED y piezo permiten observar los avisos. Para volver a medir datos reales, carga el entorno `uno` o `nano`. El modo demo se identifica al arrancar.

## Avisos

| Estado | Entrada | Indicador |
|---|---|---|
| NORMAL | T > 4 °C | Verde |
| VIGILANCIA | 2 < T ≤ 4 °C | Amarillo |
| ALERTA | 0 < T ≤ 2 °C | Amarillo y sonido intermitente |
| HELADA | T ≤ 0 °C | Rojo y sonido intermitente |
| ERROR_SENSOR | Desconectado, lectura inválida o 85 °C | Rojo parpadeante y sonido grave |

Al subir la temperatura se exige superar el umbral en 0,5 °C para salir del estado anterior (histéresis). Una lectura de 85 °C se trata conservadoramente como fallo porque coincide con el valor de encendido del DS18B20; este equipo está orientado a heladas, no a medir temperaturas altas. Se vuelve a buscar el sensor después de cada lectura para permitir su reconexión.

Los umbrales son **reglas de aviso configurables**, no un modelo meteorológico validado ni una predicción con horas de anticipación. El viento débil se informa como contexto (`viento_debil=1` bajo 2 m/s); el viento fuerte nunca cancela un aviso de frío. Temperatura y viento por sí solos no determinan el daño en cada cultivo.

## Calibrar el anemómetro

Edita `include/config.h`, recompila y vuelve a cargar:

1. Consulta la ficha técnica: si da `v = a × f + b` en m/s, coloca `a` en `WIND_MS_PER_HZ` y `b` en `WIND_OFFSET_MS`. Si entrega km/h, divide ambos coeficientes por 3,6.
2. Sin ficha, compara con un anemómetro de referencia a varias velocidades estables. El CSV informa los pulsos y la frecuencia en Hz: `f = pulsos × 1000 / intervalo_ms`.
3. Con un ajuste por el origen usa `a = velocidad_referencia / f`. Ejemplo exclusivamente ilustrativo: 3 m/s y 5 Hz dan `a = 0,6`; no es una calibración universal. Si el modelo requiere intercepto, ajusta `a` y `b` con varios puntos.
4. Verifica el resultado a distintas velocidades. Para cero pulsos se informa cero m/s, incluso si hay intercepto.

El valor inicial `a = 1` es un ejemplo sin calibrar y se anuncia al arrancar. Se cuentan flancos descendentes con 5 ms de antirrebote: limita la frecuencia aceptada a unos 200 Hz. Ajusta `DEBOUNCE_US` si la ficha requiere más frecuencia. No multipliques de nuevo por pulsos por vuelta si la fórmula del fabricante ya usa Hz de salida.

Ausencia de pulsos puede significar calma **o cable desconectado**; un contacto simple no permite distinguirlos. Con viento débil, la resolución de una ventana de dos segundos puede ser insuficiente; aumentar `SAMPLE_MS` mejora la resolución pero retrasa los avisos.

## Verificación práctica

1. Carga demo y comprueba todos los estados y sus indicadores.
2. Carga firmware real; a temperatura ambiente debe aparecer una lectura razonable. Desconecta el DS18B20: en el siguiente ciclo debe indicar ERROR_SENSOR; reconéctalo y comprueba recuperación.
3. Enfría la sonda con una mezcla de hielo y agua (solo la parte apta para inmersión) y contrasta con un termómetro de referencia.
4. Gira el anemómetro y comprueba pulsos. Con el sensor desconectado puedes cerrar brevemente D2 contra GND para comprobar el contador; no apliques tensión externa.
5. Calibra el viento y ajusta umbrales según cultivo y ubicación. Instala la sonda con protección de radiación y registra el comportamiento en campo.

El CSV empieza con líneas informativas `#` y una cabecera. `NA` indica temperatura inválida; no se reutiliza una lectura antigua. Cada reinicio inicia el contador de tiempo desde cero.

## Pruebas automáticas y referencias

GitHub Actions compila Uno, Nano y demo, ejecuta pruebas C++ de límites, recuperación, histéresis y conversión de pulsos, y publica los archivos `.hex` como artefactos. Para ejecutar las pruebas en un equipo con g++:

```sh
g++ -std=c++11 -Wall -Wextra -Werror -Iinclude tests/risk_test.cpp -o risk_test
./risk_test
```

- [Interrupciones externas Arduino](https://docs.arduino.cc/language-reference/en/functions/external-interrupts/attachInterrupt)
- [Configuración Uno en PlatformIO](https://docs.platformio.org/en/stable/boards/atmelavr/uno.html)
- [Ficha técnica DS18B20](https://www.analog.com/media/en/technical-documentation/data-sheets/DS18B20.pdf)
