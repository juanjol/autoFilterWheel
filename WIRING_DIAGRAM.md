# Diagrama de Conexiones - Rueda de Filtros ESP32-C3

## Componentes Necesarios
1. **ESP32-C3 con OLED integrada** (128x64 píxeles)
2. **Motor paso a paso 28BYJ-48** (5V)
3. **Driver ULN2003** para el motor
4. **Sensor magnético AS5600** (encoder rotatorio I2C)
5. **Imán de neodimio** (para el AS5600, montado en el eje de la rueda)
6. **Fuente de alimentación 5V** (mínimo 1A para el motor)

## Esquema de Conexiones

### 1. ESP32-C3 OLED → Driver ULN2003

```
ESP32-C3         ULN2003 Driver
--------         --------------
GPIO2     →      IN1
GPIO3     →      IN2
GPIO4     →      IN3
GPIO5     →      IN4
5V        →      VCC (5V)
GND       →      GND
```

### 2. Driver ULN2003 → Motor 28BYJ-48

```
ULN2003          Motor 28BYJ-48
-------          --------------
Conector 5 pines → Conector del motor (directo)
                   - Cable Azul
                   - Cable Rosa
                   - Cable Amarillo
                   - Cable Naranja
                   - Cable Rojo (5V común)
```

### 3. ESP32-C3 OLED → Sensor AS5600

```
ESP32-C3         AS5600
--------         ------
GPIO8 (SDA)  →   SDA
GPIO9 (SCL)  →   SCL
3.3V         →   VCC (3.3V importante!)
GND          →   GND
             →   DIR (No conectar - dejar flotante)
             →   PGO (No conectar)
```

### 4. Pantalla OLED
La pantalla OLED ya está integrada en la placa ESP32-C3 OLED y utiliza los pines I2C internos.

### 5. Alimentación

```
Fuente 5V Externa
-----------------
(+) 5V  → ESP32-C3 pin 5V
        → ULN2003 VCC

(-) GND → ESP32-C3 GND
        → ULN2003 GND
        → AS5600 GND
```

### 6. Conexiones Opcionales (Botones manuales)

```
ESP32-C3         Botones
--------         -------
GPIO6     →      Botón NEXT → GND
GPIO7     →      Botón PREV → GND
GPIO10    →      LED de estado (con resistencia 220Ω) → GND
```

## Diagrama ASCII

```
                    ESP32-C3 OLED Board
    ┌────────────────────────────────────────────┐
    │  [OLED Display 128x64]                     │
    │                                             │
    │  3V3 ────────────────┐                     │
    │  GND ─────────┐      │                     │
    │              │      │      ┌──── 5V        │
    │              │      │      │     GND ────┐ │
    │              │      │      │             │ │
    │  GPIO2 ──────┼──────┼──────┼─────────┐   │ │
    │  GPIO3 ──────┼──────┼──────┼──────┐  │   │ │
    │  GPIO4 ──────┼──────┼──────┼───┐  │  │   │ │
    │  GPIO5 ──────┼──────┼──────┼┐  │  │  │   │ │
    │              │      │      ││  │  │  │   │ │
    │  GPIO8(SDA) ─┼──┐   │      ││  │  │  │   │ │
    │  GPIO9(SCL) ─┼┐ │   │      ││  │  │  │   │ │
    │              ││ │   │      ││  │  │  │   │ │
    └──────────────┼┼─┼───┼──────┼┼──┼──┼──┼───┼─┘
                   ││ │   │      ││  │  │  │   │
                   ││ │   │      ││  │  │  │   │
    ┌──────────────┼┼─┼───┼──────┼┼──┼──┼──┼───┼─┐
    │   AS5600     ││ │   │      ││  │  │  │   │ │
    │   Sensor     ││ │   │      ││  │  │  │   │ │
    │              ││ │   │      ││  │  │  │   │ │
    │  SCL ────────┘│ │   │      ││  │  │  │   │ │
    │  SDA ─────────┘ │   │      ││  │  │  │   │ │
    │  VCC ───────────┘   │      ││  │  │  │   │ │
    │  GND ───────────────┘      ││  │  │  │   │ │
    │  [Imán en el eje]          ││  │  │  │   │ │
    └─────────────────────────────┘│  │  │  │   │ │
                                   │  │  │  │   │ │
    ┌──────────────────────────────┼──┼──┼──┼───┼─┼─┐
    │       ULN2003 Driver         │  │  │  │   │ │ │
    │                              │  │  │  │   │ │ │
    │  IN1 ────────────────────────┘  │  │  │   │ │ │
    │  IN2 ───────────────────────────┘  │  │   │ │ │
    │  IN3 ──────────────────────────────┘  │   │ │ │
    │  IN4 ──────────────────────────────────┘   │ │ │
    │  VCC ───────────────────────────────────────┘ │ │
    │  GND ──────────────────────────────────────────┘ │
    │                                                  │
    │  [Motor Connector] ──→ 28BYJ-48 Motor           │
    └──────────────────────────────────────────────────┘

         ┌──────────────────┐
         │   Fuente 5V/1A   │
         │                  │
         │  (+) ──→ ESP32 5V│
         │      ──→ ULN2003 │
         │                  │
         │  (-) ──→ GND     │
         └──────────────────┘
```

## Notas Importantes de Montaje

### 1. Sensor AS5600
- **IMPORTANTE**: Alimentar con 3.3V, NO con 5V
- El imán debe estar centrado sobre el chip AS5600 a una distancia de 0.5-3mm
- El imán debe girar solidario con el eje de la rueda de filtros
- Usar un imán de neodimio diametralmente magnetizado de 6mm de diámetro

### 2. Motor 28BYJ-48
- El motor requiere 5V y puede consumir hasta 240mA
- La secuencia de pasos está gestionada por la librería AccelStepper
- El motor tiene 2048 pasos por revolución (reducción interna 64:1)

### 3. Alimentación
- NO alimentar el motor desde el USB del ESP32
- Usar fuente externa de 5V con al menos 1A de capacidad
- Conectar todas las GND juntas (ESP32, ULN2003, AS5600)

### 4. Resistencias Pull-up I2C
- El ESP32-C3 tiene pull-ups internas para I2C
- Si hay problemas de comunicación, añadir resistencias de 4.7kΩ externas en SDA y SCL

### 5. Montaje Mecánico
- La rueda de filtros debe estar balanceada para evitar vibraciones
- Considerar usar rodamientos para el eje principal
- El acoplamiento entre motor y rueda puede ser directo o mediante engranajes/correa

## Configuración en el Software

Todos los pines son configurables en el archivo `src/config.h`:

```cpp
// Pines del motor (actualizados para evitar conflictos)
#define MOTOR_PIN1 2   // GPIO2 → IN1
#define MOTOR_PIN2 3   // GPIO3 → IN2
#define MOTOR_PIN3 4   // GPIO4 → IN3
#define MOTOR_PIN4 8   // GPIO8 → IN4

// Pines I2C (corregidos para ESP32-C3 OLED 0.42")
#define I2C_SDA 5      // GPIO5 (SDA para OLED 0.42")
#define I2C_SCL 6      // GPIO6 (SCL para OLED 0.42")

// Opcionales
#define LED_PIN 10     // GPIO10
#define BUTTON_NEXT 9  // GPIO9
#define BUTTON_PREV 7  // GPIO7
```

## Comandos Disponibles

### 📡 Protocolo de Comunicación
- **Puerto Serie**: 115200 baudios
- **Formato**: `#COMANDO` seguido de ENTER
- **Respuestas**: Confirmación o mensaje de error

### 🎯 Comandos de Posición de Filtros

| Comando | Descripción | Ejemplo | Respuesta |
|---------|-------------|---------|-----------|
| `#GP` | Obtener posición actual | `#GP` | `P3` |
| `#MP[1-5]` | Mover a posición de filtro | `#MP2` | `M2` |
| `#SP[1-5]` | Establecer posición actual | `#SP1` | `S1` |
| `#GF` | Obtener número de filtros | `#GF` | `F5` |

### ⚙️ Comandos de Movimiento Manual

| Comando | Descripción | Ejemplo | Respuesta |
|---------|-------------|---------|-----------|
| `#SF[X]` | Avanzar X pasos | `#SF100` | `SF100` |
| `#SB[X]` | Retroceder X pasos | `#SB50` | `SB50` |
| `#ST[X]` | Ir a posición absoluta (en pasos) | `#ST1024` | `ST1024` |
| `#GST` | Obtener posición actual (en pasos) | `#GST` | `STEP:1024` |

### 🔧 Comandos de Sistema

| Comando | Descripción | Ejemplo | Respuesta |
|---------|-------------|---------|-----------|
| `#CAL` | Calibrar posición inicial | `#CAL` | `CALIBRATED` |
| `#STATUS` | Estado completo del sistema | `#STATUS` | `STATUS:POS=1,MOVING=NO,CAL=YES,ANGLE=0.5,ERROR=0` |
| `#VER` | Versión del firmware | `#VER` | `VERSION:1.0.0` |
| `#STOP` | Parada de emergencia | `#STOP` | `STOPPED` |

### 📊 Respuestas de Estado (STATUS)

El comando `#STATUS` devuelve información detallada:
- **POS**: Posición actual del filtro (1-5)
- **MOVING**: Si el motor está en movimiento (YES/NO)
- **CAL**: Si el sistema está calibrado (YES/NO)
- **ANGLE**: Ángulo actual del sensor AS5600 (si está conectado)
- **ERROR**: Código de error (0 = sin errores)

### 🚨 Códigos de Error

| Código | Significado |
|--------|-------------|
| 0 | Sin errores |
| 1 | Timeout del motor |
| 2 | Fallo del encoder AS5600 |
| 3 | Posición inválida |
| 4 | Fallo de calibración |
| 5 | Fallo de EEPROM |
| 6 | Error de comunicación |

### 💡 Límites y Validaciones

- **Posiciones de filtro**: 1 a 5 únicamente
- **Pasos manuales**: Máximo 2048 pasos por comando
- **Timeout de movimiento**: 10 segundos máximo
- **Calibración**: Requerida para funcionamiento preciso

## Prueba Inicial

1. Conectar todo según el diagrama
2. Abrir el monitor serie a 115200 baudios
3. Enviar comando `#CAL` para calibrar la posición inicial
4. Probar con `#MP2` para mover a la posición 2
5. Verificar con `#GP` para obtener la posición actual

## Secuencia de Calibración Recomendada

```
#CAL          ← Calibrar posición inicial como filtro 1
#STATUS       ← Verificar estado del sistema
#MP2          ← Mover a posición 2
#GP           ← Confirmar posición alcanzada
#SF10         ← Ajuste fino si es necesario
#SP2          ← Establecer como posición 2 definitiva
```

## Ejemplos de Uso Avanzado

### Ajuste Fino de Posiciones
```
#MP3          ← Ir a filtro 3
#GST          ← Ver posición en pasos (ej: STEP:1228)
#SF5          ← Ajustar 5 pasos adelante
#SP3          ← Guardar nueva posición del filtro 3
```

### Movimiento Manual Preciso
```
#ST0          ← Ir al paso 0 (origen)
#SF409        ← Avanzar 409 pasos (1/5 de vuelta = 72°)
#GST          ← Verificar posición actual
#SP1          ← Establecer como filtro 1
```

### Verificación con Encoder AS5600
```
#STATUS       ← Ver ángulo actual del encoder
#MP4          ← Mover a filtro 4 (debería ser ~288°)
#STATUS       ← Verificar que el ángulo sea correcto
```

## Especificaciones Técnicas

### Motor 28BYJ-48
- **Pasos por revolución**: 2048 pasos
- **Resolución angular**: 0.176° por paso
- **Pasos por grado**: 5.689 pasos/grado
- **Separación entre filtros**: 409.6 pasos (72°)

### Sensor AS5600
- **Rango de detección**: 360° completos
- **Resolución**: 12 bits (4096 posiciones)
- **Precisión angular**: 0.088° por paso
- **Comunicación**: I2C (dirección 0x36)

## Solución de Problemas

| Problema | Posible Causa | Solución |
|----------|---------------|----------|
| Motor no gira | Alimentación insuficiente | Usar fuente externa 5V/1A |
| AS5600 no detectado | Conexiones I2C | Verificar SDA/SCL y alimentación 3.3V |
| Posición incorrecta | Falta calibración | Enviar comando #CAL |
| OLED no muestra | Dirección I2C incorrecta | Cambiar OLED_ADDRESS en config.h |