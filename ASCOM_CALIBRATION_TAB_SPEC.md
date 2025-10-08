# ASCOM Driver - Especificación de Pestaña de Calibración Personalizada

## 📋 Índice

1. [Contexto del Proyecto](#contexto-del-proyecto)
2. [Sistema de Calibración del Firmware](#sistema-de-calibración-del-firmware)
3. [Comandos Serial - Referencia Completa](#comandos-serial---referencia-completa)
4. [Especificación de la Interface de Usuario](#especificación-de-la-interface-de-usuario)
5. [Flujos de Trabajo Detallados](#flujos-de-trabajo-detallados)
6. [Implementación Técnica](#implementación-técnica)
7. [Validaciones y Manejo de Errores](#validaciones-y-manejo-de-errores)
8. [Ejemplos de Código](#ejemplos-de-código)

---

## Contexto del Proyecto

Este documento especifica la **Pestaña de Calibración Personalizada** para el driver ASCOM (Windows Forms .NET) del controlador de rueda de filtros astronómico basado en ESP32-C3.

### Estado Actual del Firmware
- ✅ **Completamente implementado y probado**
- ✅ Sistema de calibración de ángulos personalizados funcional
- ✅ Almacenamiento en EEPROM
- ✅ Control PID con soporte de ángulos personalizados
- ✅ Comandos serial completamente documentados

### Propósito de la Calibración Personalizada

**¿Por qué es necesaria?**
- Compensar **espaciado no uniforme** de filtros en la rueda
- Corregir **tolerancias de fabricación** del hardware
- Soportar **diseños irregulares** de ruedas personalizadas
- Maximizar **precisión de posicionamiento** (<0.8° de error)

**Modo de operación:**
- **Con calibración personalizada**: Cada posición tiene un ángulo específico (ej: 0°, 68.5°, 142.3°, 218.7°, 291.2°)
- **Sin calibración (default)**: Distribución uniforme (ej: 0°, 72°, 144°, 216°, 288° para 5 filtros)

---

## Sistema de Calibración del Firmware

### Arquitectura EEPROM

```
Dirección    Contenido                        Tamaño
------------ -------------------------------- -------
0x00         Calibration flag (0xAA)          4 bytes
0x04         AS5600 encoder offset (float)    4 bytes
0x08         Current position (uint8_t)       1 byte
0x0C         Filter names flag (0xBB)         4 bytes
0x10         Filter count (uint8_t)           1 byte
0x11         Custom angles flag (0xCA)        1 byte
0x12-0x35    Custom angles array (9 floats)   36 bytes
0x40+        Filter names (16 bytes each)     144 bytes max
```

### Funcionamiento del Control de Posición

```cpp
// Pseudocódigo del firmware
float positionToAngle(uint8_t position) {
    if (customAnglesConfigured()) {
        return loadCustomAngle(position);  // Ej: 68.50° para posición 2
    } else {
        float degreesPerPosition = 360.0 / numFilters;
        return (position - 1) * degreesPerPosition;  // Ej: 72° para posición 2
    }
}
```

El control PID usa automáticamente los ángulos personalizados si están disponibles.

---

## Comandos Serial - Referencia Completa

### Configuración del Puerto Serial

```
Puerto:        Auto-detectar (típicamente COM3-COM10)
Baud Rate:     115200
Data Bits:     8
Parity:        None
Stop Bits:     1
Flow Control:  None
Line Ending:   LF (\n)
Timeout:       5000 ms (comandos normales)
               10000 ms (comandos de movimiento)
```

### 1. Comandos de Información del Sistema

#### `#GF` - Get Filter Count
Obtiene el número total de filtros configurados.

**Request:**
```
#GF\n
```

**Response:**
```
F5\n
```

**Parsing:**
```csharp
string response = SendCommand("#GF");
int filterCount = int.Parse(response.Substring(1)); // "F5" -> 5
```

---

#### `#GP` - Get Position
Obtiene la posición actual de la rueda (1-based).

**Request:**
```
#GP\n
```

**Response:**
```
P3\n
```

**Parsing:**
```csharp
string response = SendCommand("#GP");
int currentPosition = int.Parse(response.Substring(1)); // "P3" -> 3
```

---

#### `#GN` - Get Filter Names (All)
Obtiene los nombres de todos los filtros.

**Request:**
```
#GN\n
```

**Response:**
```
NAMES:Luminance,Red,Green,Blue,H-Alpha\n
```

**Parsing:**
```csharp
string response = SendCommand("#GN");
string namesStr = response.Substring(6); // Skip "NAMES:"
string[] filterNames = namesStr.Split(',');
// filterNames[0] = "Luminance", filterNames[1] = "Red", etc.
```

---

#### `#GN[1-9]` - Get Filter Name (Single)
Obtiene el nombre de un filtro específico.

**Request:**
```
#GN2\n
```

**Response:**
```
N2:Red\n
```

**Parsing:**
```csharp
string response = SendCommand("#GN2");
int colonPos = response.IndexOf(':');
string filterName = response.Substring(colonPos + 1); // "Red"
```

---

### 2. Comandos de Estado del Encoder

#### `#ENCSTATUS` - Get Encoder Status
Obtiene el estado completo del encoder, incluyendo el **ángulo actual**.

**Request:**
```
#ENCSTATUS\n
```

**Response (una sola línea en formato CSV):**
```
ENCSTATUS:Angle=144.50,Expected=72.00,Error=0.35,Raw=2048,Offset=12.30,Dir=CW,Health=OK\n
```

**Cuando el encoder no está disponible:**
```
ENCSTATUS:Not connected\n
```

**Parsing:**
```csharp
string response = SendCommand("#ENCSTATUS");

// Verificar si el encoder está disponible
if (response.Contains("Not connected")) {
    LogMessage("Encoder no disponible");
    return;
}

// Parse formato: "ENCSTATUS:Angle=144.50,Expected=72.00,Error=0.35,Raw=2048,Offset=12.30,Dir=CW,Health=OK"
string data = response.Substring(10); // Skip "ENCSTATUS:"
string[] pairs = data.Split(',');

var values = new Dictionary<string, string>();
foreach (string pair in pairs) {
    string[] parts = pair.Split('=');
    if (parts.Length == 2) {
        values[parts[0]] = parts[1];
    }
}

// Extraer valores
float currentAngle = float.Parse(values["Angle"]);
float expectedAngle = float.Parse(values["Expected"]);
float error = float.Parse(values["Error"]);
int rawValue = int.Parse(values["Raw"]);
float offset = float.Parse(values["Offset"]);
string direction = values["Dir"]; // "CW", "CCW", o "STOP"
string health = values["Health"]; // "OK" o "FAULT"

LogMessage($"Ángulo actual: {currentAngle:F2}°, Error: {error:F2}°");
```

**Frecuencia recomendada:** Cada 500ms con un Timer para actualización en tiempo real.

---

### 3. Comandos de Movimiento Manual

#### `#SF[steps]` - Step Forward
Mueve el motor hacia adelante un número específico de pasos.

**Request:**
```
#SF10\n    // Avanza 10 pasos
#SF100\n   // Avanza 100 pasos
```

**Response:**
```
SF10\n
```

**Notas:**
- Comando **bloqueante**: No retorna hasta completar el movimiento
- Rango válido: 1-4096 pasos
- 1 revolución completa = 2048 pasos
- Útil para ajustes finos de posición

**Ejemplo de uso:**
```csharp
private void btnStepForward10_Click(object sender, EventArgs e)
{
    try {
        string response = SendCommand("#SF10", timeout: 5000);
        LogMessage($"Motor avanzado 10 pasos: {response}");
        UpdateEncoderAngle(); // Actualizar display de ángulo
    }
    catch (Exception ex) {
        MessageBox.Show($"Error: {ex.Message}");
    }
}
```

---

#### `#SB[steps]` - Step Backward
Mueve el motor hacia atrás un número específico de pasos.

**Request:**
```
#SB10\n    // Retrocede 10 pasos
#SB100\n   // Retrocede 100 pasos
```

**Response:**
```
SB10\n
```

**Notas:**
- Mismo comportamiento que `#SF` pero en dirección opuesta
- Comando **bloqueante**
- Rango válido: 1-4096 pasos

**Ejemplo de uso:**
```csharp
private void btnStepBackward1_Click(object sender, EventArgs e)
{
    try {
        string response = SendCommand("#SB1", timeout: 5000);
        LogMessage($"Motor retrocedido 1 paso: {response}");
        UpdateEncoderAngle();
    }
    catch (Exception ex) {
        MessageBox.Show($"Error: {ex.Message}");
    }
}
```

---

### 4. Comandos de Calibración de Ángulos Personalizados

#### `#SETANG[pos]:[angle]` - Set Custom Angle
Establece un ángulo personalizado para una posición específica.

**Request:**
```
#SETANG1:0.0\n      // Posición 1 = 0°
#SETANG2:68.5\n     // Posición 2 = 68.5°
#SETANG3:142.3\n    // Posición 3 = 142.3°
```

**Response:**
```
SETANG:Position 1 set to 0.00°\n
```

**Efecto en el firmware:**
- Guarda el ángulo directamente en EEPROM (dirección 0x12 + (pos-1)*4)
- Establece flag de ángulos personalizados (0xCA en dirección 0x11)
- Ejecuta `EEPROM.commit()` para persistencia
- **NO requiere comando adicional para guardar** - es inmediato

**Validaciones:**
- Posición debe estar en rango 1-filterCount
- Ángulo debe estar en rango 0.0-359.99°
- Encoder debe estar disponible
- Si no: `ERROR:Invalid position (10). Must be 1-5`

**Parsing:**
```csharp
private void SetCustomAngle(int position, float angle)
{
    string command = $"#SETANG{position}:{angle:F2}";
    string response = SendCommand(command);

    // Parse: "SETANG:Position 1 set to 0.00°"
    if (response.StartsWith("SETANG:")) {
        LogMessage($"Ángulo personalizado establecido: Posición {position} = {angle:F2}°");
        UpdatePositionAngleDisplay(position, angle);
    }
}
```

**Uso típico:**
- Configuración manual cuando los ángulos ya se conocen
- Corrección de una sola posición sin recalibrar todo
- Importación de calibración desde archivo

---

#### `#GETANG[pos]` - Get Custom Angle (Single)
Obtiene el ángulo configurado para una posición específica.

**Request:**
```
#GETANG1\n
#GETANG2\n
```

**Response (con ángulo personalizado):**
```
GETANG1:0.00° (custom)\n
```

**Response (sin ángulo personalizado, usando default):**
```
GETANG1:0.00° (default)\n
```

**Parsing:**
```csharp
string response = SendCommand("#GETANG2");
// Parse: "GETANG2:68.50° (custom)"

int colonPos = response.IndexOf(':');
int degreePos = response.IndexOf('°');
string angleStr = response.Substring(colonPos + 1, degreePos - colonPos - 1);
float angle = float.Parse(angleStr);

bool isCustom = response.Contains("(custom)");

UpdatePositionDisplay(2, angle, isCustom);
```

---

#### `#GETANG` - Get All Custom Angles
Obtiene todos los ángulos configurados de una vez.

**Request:**
```
#GETANG\n
```

**Response (con calibración personalizada):**
```
GETANG:1=0.00°,2=68.50°,3=142.30°,4=218.75°,5=291.20°\n
```

**Response (sin calibración personalizada):**
```
GETANG:No custom angles configured (using uniform distribution)\n
```

**Parsing:**
```csharp
Dictionary<int, float> GetAllCustomAngles()
{
    string response = SendCommand("#GETANG");
    var angles = new Dictionary<int, float>();

    if (response.Contains("No custom angles")) {
        LogMessage("No hay ángulos personalizados. Usando distribución uniforme.");
        return angles; // Empty dictionary
    }

    // Parse: "GETANG:1=0.00°,2=68.50°,3=142.30°,4=218.75°,5=291.20°"
    string anglesPart = response.Substring(7); // Skip "GETANG:"
    string[] pairs = anglesPart.Split(',');

    foreach (string pair in pairs) {
        string[] parts = pair.Split('=');
        int position = int.Parse(parts[0]);
        string angleStr = parts[1].TrimEnd('°');
        float angle = float.Parse(angleStr);
        angles[position] = angle;
    }

    return angles;
}
```

---

#### `#CLEARANG` - Clear All Custom Angles
Borra todos los ángulos personalizados y revierte a distribución uniforme.

**Request:**
```
#CLEARANG\n
```

**Response:**
```
CLEARANG:All custom angles cleared. Using uniform distribution.\n
```

**Efecto en el firmware:**
- Borra el flag de ángulos personalizados (0x00 en dirección 0x11)
- Ejecuta `EEPROM.commit()`
- El sistema revierte automáticamente a distribución uniforme
- **NO requiere reinicio** - efecto inmediato

**Parsing:**
```csharp
private void btnClearCalibration_Click(object sender, EventArgs e)
{
    // Confirmación obligatoria
    DialogResult result = MessageBox.Show(
        "¿Está seguro de que desea borrar toda la calibración personalizada?\n\n" +
        "El sistema volverá a usar distribución uniforme de ángulos.",
        "Confirmar Borrado de Calibración",
        MessageBoxButtons.YesNo,
        MessageBoxIcon.Warning
    );

    if (result == DialogResult.Yes) {
        string response = SendCommand("#CLEARANG");
        LogMessage("Calibración personalizada borrada");
        RefreshAllAngles(); // Actualizar display con ángulos uniformes
        ClearCalibrationMarkers(); // Quitar checks de "calibrado"
    }
}
```

---

### 5. Comandos de Estado del Sistema

#### `#STATUS` - Get System Status
Obtiene el estado completo del sistema.

**Request:**
```
#STATUS\n
```

**Response (una sola línea en formato CSV):**
```
STATUS:POS=3,MOVING=NO,CAL=YES,ANGLE=144.5,ERROR=0\n
```

**Cuando el encoder NO está disponible:**
```
STATUS:POS=3,MOVING=NO,CAL=YES,ERROR=0\n
```
(El campo `ANGLE` solo aparece cuando el encoder está disponible)

**Parsing:**
```csharp
string response = SendCommand("#STATUS");

// Parse formato: "STATUS:POS=3,MOVING=NO,CAL=YES,ANGLE=144.5,ERROR=0"
string data = response.Substring(7); // Skip "STATUS:"
string[] pairs = data.Split(',');

var values = new Dictionary<string, string>();
foreach (string pair in pairs) {
    string[] parts = pair.Split('=');
    if (parts.Length == 2) {
        values[parts[0]] = parts[1];
    }
}

// Extraer valores
int position = int.Parse(values["POS"]);
bool isMoving = values["MOVING"] == "YES";
bool isCalibrated = values["CAL"] == "YES";
int errorCode = int.Parse(values["ERROR"]);

// Ángulo solo si está disponible
float angle = 0.0f;
if (values.ContainsKey("ANGLE")) {
    angle = float.Parse(values["ANGLE"]);
}

LogMessage($"Posición: {position}, Moviendo: {isMoving}, Calibrado: {isCalibrated}");
if (values.ContainsKey("ANGLE")) {
    LogMessage($"Ángulo: {angle:F1}°");
}
```

**Uso:**
- Verificar estado antes de iniciar calibración
- Monitorear salud del sistema
- Detectar si el encoder está funcionando

---

## Especificación de la Interface de Usuario

### Layout General de la Pestaña

```
┌─────────────────────────────────────────────────────────────────┐
│  CALIBRACIÓN PERSONALIZADA DE ÁNGULOS                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ INFORMACIÓN DEL SISTEMA                                   │  │
│  │  • Número de filtros: 5                                  │  │
│  │  • Posición actual: 3 (Blue)                             │  │
│  │  • Ángulo del encoder: 144.50°                           │  │
│  │  • Estado: ⚫ Conectado                                   │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ CONTROL DE MOVIMIENTO MANUAL                              │  │
│  │                                                           │  │
│  │  Retroceder:  [-100] [-50] [-10] [-1]                    │  │
│  │  Avanzar:     [+1] [+10] [+50] [+100]                    │  │
│  │                                                           │  │
│  │  Pasos personalizados: [____50____] [Retroceder] [Avanzar] │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ CALIBRACIÓN DE POSICIONES                                 │  │
│  │                                                           │  │
│  │  Pos  Filtro       Ángulo Actual    Ángulo Deseado Acción│  │
│  │  ─────────────────────────────────────────────────────── │  │
│  │  1    Luminance    0.00° (custom)   [_0.00__]  [Calibrar]│  │
│  │  2    Red          68.50° (custom)  [_68.50_]  [Calibrar]│  │
│  │  3    Green        142.30° (custom) [142.30_]  [Calibrar]│  │
│  │  4    Blue         218.75° (custom) [218.75_]  [Calibrar]│  │
│  │  5    H-Alpha      291.20° (custom) [291.20_]  [Calibrar]│  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ ACCIONES GLOBALES                                         │  │
│  │  [Aplicar Ángulo Actual a Posición] [Limpiar Calibración]│  │
│  │  [Refrescar Ángulos] [Importar] [Exportar]               │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ LOG DE EVENTOS                                            │  │
│  │  12:34:56 - Posición 1 calibrada a 0.00°                 │  │
│  │  12:35:12 - Posición 2 calibrada a 68.50°                │  │
│  │  12:35:45 - Posición 3 calibrada a 142.30°               │  │
│  │  12:36:23 - Todas las posiciones calibradas              │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

### Componentes Detallados

#### 1. Panel de Información del Sistema

**Controles:**
```csharp
Label lblFilterCount;        // "Número de filtros: 5"
Label lblCurrentPosition;    // "Posición actual: 3 (Blue)"
Label lblEncoderAngle;       // "Ángulo del encoder: 144.50°"
PictureBox pbConnectionStatus; // ⚫ Verde=conectado, Rojo=desconectado
```

**Actualización:**
- Timer cada 500ms
- Ejecuta `#ENCSTATUS` para obtener ángulo
- Ejecuta `#GP` para obtener posición
- Colorea el indicador según estado de conexión

---

#### 2. Panel de Control de Movimiento Manual

**Controles:**
```csharp
// Botones de retroceso
Button btnStepBack100;   // "-100 pasos"
Button btnStepBack50;    // "-50 pasos"
Button btnStepBack10;    // "-10 pasos"
Button btnStepBack1;     // "-1 paso"

// Botones de avance
Button btnStepForward1;   // "+1 paso"
Button btnStepForward10;  // "+10 pasos"
Button btnStepForward50;  // "+50 pasos"
Button btnStepForward100; // "+100 pasos"

// Control personalizado
NumericUpDown nudCustomSteps;  // Default: 50, Min: 1, Max: 2048
Button btnCustomBack;          // "Retroceder"
Button btnCustomForward;       // "Avanzar"
```

**Eventos:**
```csharp
private void btnStepForward10_Click(object sender, EventArgs e)
{
    ExecuteStepCommand("#SF10", "Avanzar 10 pasos");
}

private void btnCustomForward_Click(object sender, EventArgs e)
{
    int steps = (int)nudCustomSteps.Value;
    ExecuteStepCommand($"#SF{steps}", $"Avanzar {steps} pasos");
}

private void ExecuteStepCommand(string command, string description)
{
    DisableMovementButtons();
    try {
        string response = SendCommand(command, timeout: 10000);
        LogMessage($"{description}: {response}");
        UpdateEncoderAngle();
    }
    catch (Exception ex) {
        LogMessage($"ERROR: {ex.Message}");
        MessageBox.Show($"Error al mover motor: {ex.Message}");
    }
    finally {
        EnableMovementButtons();
    }
}
```

---

#### 3. Panel de Calibración de Posiciones

**Controles:**
```csharp
DataGridView dgvPositions;

// Columnas:
// - Posición (int, read-only)
// - Filtro (string, read-only)
// - Ángulo Actual (string, read-only, formato "XXX.XX° (custom/default)")
// - Ángulo Deseado (TextBox column, editable)
// - Acción (DataGridViewButtonColumn, texto "Calibrar")

private void InitializePositionsGrid()
{
    dgvPositions.Columns.Add("Position", "Pos");
    dgvPositions.Columns["Position"].ReadOnly = true;

    dgvPositions.Columns.Add("FilterName", "Filtro");
    dgvPositions.Columns["FilterName"].ReadOnly = true;

    dgvPositions.Columns.Add("CurrentAngle", "Ángulo Actual");
    dgvPositions.Columns["CurrentAngle"].ReadOnly = true;

    var textBoxCol = new DataGridViewTextBoxColumn();
    textBoxCol.Name = "DesiredAngle";
    textBoxCol.HeaderText = "Ángulo Deseado";
    dgvPositions.Columns.Add(textBoxCol);

    var btnCol = new DataGridViewButtonColumn();
    btnCol.Name = "Action";
    btnCol.HeaderText = "Acción";
    btnCol.Text = "Calibrar";
    btnCol.UseColumnTextForButtonValue = true;
    dgvPositions.Columns.Add(btnCol);

    dgvPositions.AllowUserToAddRows = false;
    dgvPositions.CellContentClick += DgvPositions_CellContentClick;
}

private void DgvPositions_CellContentClick(object sender, DataGridViewCellEventArgs e)
{
    if (e.ColumnIndex == dgvPositions.Columns["Action"].Index && e.RowIndex >= 0) {
        int position = e.RowIndex + 1;
        float desiredAngle = float.Parse(dgvPositions.Rows[e.RowIndex].Cells["DesiredAngle"].Value.ToString());
        SetCustomAngle(position, desiredAngle);
    }
}
```

---

#### 4. Panel de Acciones Globales

**Controles:**
```csharp
Button btnApplyCurrentAngle;    // "Aplicar Ángulo Actual a Posición"
Button btnClearCalibration;     // "Limpiar Calibración"
Button btnRefreshAngles;        // "Refrescar Ángulos"
Button btnImportCalibration;    // "Importar"
Button btnExportCalibration;    // "Exportar"
ComboBox cmbTargetPosition;     // Para seleccionar posición al aplicar ángulo actual
```

**Eventos:**
```csharp
private void btnApplyCurrentAngle_Click(object sender, EventArgs e)
{
    try {
        int position = (int)cmbTargetPosition.SelectedValue;
        float currentAngle = GetCurrentEncoderAngle();

        DialogResult result = MessageBox.Show(
            $"¿Establecer posición {position} al ángulo actual?\n\nÁngulo: {currentAngle:F2}°",
            "Confirmar",
            MessageBoxButtons.YesNo,
            MessageBoxIcon.Question
        );

        if (result == DialogResult.Yes) {
            SetCustomAngle(position, currentAngle);
        }
    }
    catch (Exception ex) {
        MessageBox.Show($"Error: {ex.Message}");
    }
}

private void btnClearCalibration_Click(object sender, EventArgs e)
{
    DialogResult result = MessageBox.Show(
        "¿Borrar toda la calibración personalizada?\n\nSe volverá a distribución uniforme.",
        "Confirmar",
        MessageBoxButtons.YesNo,
        MessageBoxIcon.Warning
    );

    if (result == DialogResult.Yes) {
        try {
            string response = SendCommand("#CLEARANG");
            LogMessage("Calibración borrada");
            RefreshAllAngles();
        }
        catch (Exception ex) {
            MessageBox.Show($"Error: {ex.Message}");
        }
    }
}

private void btnRefreshAngles_Click(object sender, EventArgs e)
{
    RefreshAllAngles();
}
```

---

#### 5. Panel de Log de Eventos

**Controles:**
```csharp
TextBox txtLog;  // Multiline, ReadOnly, ScrollBars.Vertical

private void LogMessage(string message)
{
    string timestamp = DateTime.Now.ToString("HH:mm:ss");
    txtLog.AppendText($"{timestamp} - {message}\r\n");
    txtLog.ScrollToCaret();
}
```

---

## Flujos de Trabajo Detallados

### Flujo 1: Calibración Manual con Ángulos Conocidos

Para usuarios que conocen los ángulos exactos de cada posición:

```
┌─────────────────────────────────────┐
│ Usuario tiene ángulos medidos:     │
│ Pos 1: 0.0°                         │
│ Pos 2: 68.5°                        │
│ Pos 3: 142.3°                       │
│ Pos 4: 218.75°                      │
│ Pos 5: 291.2°                       │
└────────┬────────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Usuario ingresa ángulos            │
│ en campos de texto del grid        │
└────────┬───────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Usuario hace clic en "Calibrar"    │
│ para cada posición                 │
└────────┬───────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Sistema envía:                     │
│ #SETANG1:0.0                       │
│ #SETANG2:68.5                      │
│ #SETANG3:142.3                     │
│ #SETANG4:218.75                    │
│ #SETANG5:291.2                     │
└────────┬───────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Cada comando guarda               │
│ inmediatamente en EEPROM          │
└────────┬───────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Sistema verifica con #GETANG      │
│ Muestra ángulos guardados         │
└────────────────────────────────────┘
```

**Código:**
```csharp
public void SetManualAngles()
{
    try {
        for (int i = 0; i < dgvPositions.Rows.Count; i++) {
            int position = i + 1;
            float angle = float.Parse(dgvPositions.Rows[i].Cells["DesiredAngle"].Value.ToString());

            string response = SendCommand($"#SETANG{position}:{angle:F2}");
            LogMessage($"Posición {position} establecida a {angle:F2}°");

            UpdatePositionDisplay(position, angle, isCalibrated: true);
        }

        MessageBox.Show("Ángulos establecidos correctamente");
        RefreshAllAngles();
    }
    catch (Exception ex) {
        MessageBox.Show($"Error: {ex.Message}");
    }
}
```

---

### Flujo 2: Calibración por Posicionamiento Manual

Para usuarios que prefieren ajustar físicamente cada posición:

```
┌─────────────────────────────────────┐
│ Usuario selecciona posición a      │
│ calibrar (ej: Posición 2)          │
└────────┬────────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Usuario usa botones de movimiento  │
│ (#SF, #SB) para ajustar rueda     │
└────────┬───────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Display muestra ángulo en          │
│ tiempo real: 68.25° → 68.45° →     │
│ 68.50° (actualización cada 500ms)  │
└────────┬───────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Usuario satisfecho con posición    │
│ Hace clic "Aplicar Ángulo Actual"  │
└────────┬───────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Sistema lee ángulo actual          │
│ (#ENCSTATUS) = 68.50°              │
└────────┬───────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Sistema envía:                     │
│ #SETANG2:68.50                     │
└────────┬───────────────────────────┘
         │
         ▼
┌────────────────────────────────────┐
│ Ángulo guardado en EEPROM          │
│ Display actualizado                │
└────────────────────────────────────┘
```

**Código:**
```csharp
private void btnApplyCurrentAngle_Click(object sender, EventArgs e)
{
    try {
        int position = (int)cmbTargetPosition.SelectedValue;
        float currentAngle = GetCurrentEncoderAngle();

        DialogResult result = MessageBox.Show(
            $"¿Establecer posición {position} al ángulo actual?\n\n" +
            $"Ángulo: {currentAngle:F2}°\n" +
            $"Filtro: {GetFilterName(position)}",
            "Confirmar Calibración",
            MessageBoxButtons.YesNo,
            MessageBoxIcon.Question
        );

        if (result == DialogResult.Yes) {
            string response = SendCommand($"#SETANG{position}:{currentAngle:F2}");
            LogMessage($"✓ Posición {position} calibrada a {currentAngle:F2}°");
            RefreshAllAngles();
        }
    }
    catch (Exception ex) {
        MessageBox.Show($"Error: {ex.Message}");
    }
}
```

---

### Flujo 3: Actualización en Tiempo Real del Ángulo

```
┌──────────────────────┐
│ Timer cada 500ms     │
└────────┬─────────────┘
         │
         ▼
┌────────────────────────────────┐
│ Verifica conexión activa       │
└────────┬───────────────────────┘
         │
         ▼
┌────────────────────────────────┐
│ Envía: #ENCSTATUS              │
└────────┬───────────────────────┘
         │
         ▼
┌────────────────────────────────┐
│ Parse respuesta:               │
│ Angle: 144.50°                 │
│ Health: OK                     │
└────────┬───────────────────────┘
         │
         ▼
┌────────────────────────────────┐
│ Actualiza Label:               │
│ lblEncoderAngle.Text =         │
│ "Ángulo: 144.50°"              │
└────────┬───────────────────────┘
         │
         ▼
┌────────────────────────────────┐
│ Opcional: Colorea según error  │
│ Verde: < 1°                    │
│ Amarillo: 1-3°                 │
│ Rojo: > 3°                     │
└────────────────────────────────┘
```

**Código:**
```csharp
private System.Windows.Forms.Timer updateTimer;

private void InitializeTimer()
{
    updateTimer = new System.Windows.Forms.Timer();
    updateTimer.Interval = 500; // 500ms
    updateTimer.Tick += UpdateTimer_Tick;
    updateTimer.Start();
}

private void UpdateTimer_Tick(object sender, EventArgs e)
{
    if (!IsConnected()) return;

    try {
        // Actualizar ángulo del encoder
        float angle = GetCurrentEncoderAngle();
        lblEncoderAngle.Text = $"Ángulo: {angle:F2}°";

        // Actualizar posición actual
        int position = GetCurrentPosition();
        string filterName = GetFilterName(position);
        lblCurrentPosition.Text = $"Posición: {position} ({filterName})";

        // Opcional: Calcular y mostrar error
        float targetAngle = GetTargetAngle(position);
        float error = CalculateAngularError(angle, targetAngle);

        if (error < 1.0f) {
            lblEncoderAngle.ForeColor = Color.Green;
        } else if (error < 3.0f) {
            lblEncoderAngle.ForeColor = Color.Orange;
        } else {
            lblEncoderAngle.ForeColor = Color.Red;
        }
    }
    catch {
        // Conexión perdida, deshabilitar UI
        OnConnectionLost();
    }
}

private float GetCurrentEncoderAngle()
{
    string response = SendCommand("#ENCSTATUS");

    if (response.Contains("Not connected")) {
        throw new Exception("Encoder no disponible");
    }

    string data = response.Substring(10);
    string[] pairs = data.Split(',');

    foreach (string pair in pairs) {
        string[] parts = pair.Split('=');
        if (parts.Length == 2 && parts[0] == "Angle") {
            return float.Parse(parts[1]);
        }
    }

    throw new Exception("No se pudo leer el ángulo del encoder");
}
```

---

## Validaciones y Manejo de Errores

### 1. Validación de Conexión

```csharp
private bool IsConnected()
{
    if (_serialPort == null || !_serialPort.IsOpen) {
        return false;
    }

    try {
        string response = SendCommand("#ID", timeout: 1000);
        return response.StartsWith("DEVICE_ID:");
    }
    catch {
        return false;
    }
}

private void ValidateConnectionBeforeAction()
{
    if (!IsConnected()) {
        throw new InvalidOperationException(
            "No hay conexión con el dispositivo.\n" +
            "Verifique el cable y la conexión serial."
        );
    }
}
```

---

### 2. Validación de Encoder Disponible

```csharp
private bool IsEncoderAvailable()
{
    try {
        string response = SendCommand("#ENCSTATUS");
        return !response.Contains("Not connected");
    }
    catch {
        return false;
    }
}

private void ValidateEncoderForCalibration()
{
    if (!IsEncoderAvailable()) {
        throw new InvalidOperationException(
            "El encoder AS5600 no está disponible.\n\n" +
            "La calibración personalizada requiere el encoder para funcionar.\n" +
            "Verifique la conexión del sensor AS5600."
        );
    }
}
```

---

### 3. Validación de Parámetros

```csharp
private void ValidatePosition(int position)
{
    if (position < 1 || position > filterCount) {
        throw new ArgumentOutOfRangeException(
            nameof(position),
            $"Posición inválida: {position}. Debe estar entre 1 y {filterCount}."
        );
    }
}

private void ValidateAngle(float angle)
{
    if (angle < 0.0f || angle >= 360.0f) {
        throw new ArgumentOutOfRangeException(
            nameof(angle),
            $"Ángulo inválido: {angle}. Debe estar entre 0.0° y 359.99°."
        );
    }
}

private void ValidateSteps(int steps)
{
    if (steps < 1 || steps > 4096) {
        throw new ArgumentOutOfRangeException(
            nameof(steps),
            $"Número de pasos inválido: {steps}. Debe estar entre 1 y 4096."
        );
    }
}
```

---

### 4. Manejo de Errores del Firmware

```csharp
private string SendCommand(string command, int timeout = 5000)
{
    if (_serialPort == null || !_serialPort.IsOpen) {
        throw new InvalidOperationException("Puerto serial no está abierto");
    }

    try {
        _serialPort.DiscardInBuffer();
        _serialPort.WriteLine(command);
        _serialPort.ReadTimeout = timeout;
        string response = _serialPort.ReadLine().Trim();

        if (response.StartsWith("ERROR:")) {
            string errorMsg = response.Substring(6);
            throw new FirmwareException(errorMsg);
        }

        return response;
    }
    catch (TimeoutException) {
        throw new TimeoutException(
            $"El dispositivo no respondió en {timeout}ms.\n" +
            $"Comando: {command}"
        );
    }
    catch (Exception ex) when (!(ex is FirmwareException)) {
        throw new CommunicationException(
            $"Error de comunicación: {ex.Message}\n" +
            $"Comando: {command}",
            ex
        );
    }
}

public class FirmwareException : Exception
{
    public FirmwareException(string message) : base($"Error del firmware: {message}") { }
}

public class CommunicationException : Exception
{
    public CommunicationException(string message, Exception inner)
        : base(message, inner) { }
}
```

---

## Ejemplos de Código

### Ejemplo Completo: Clase CalibrationTab

```csharp
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Windows.Forms;

namespace ASCOMFilterWheelDriver
{
    public partial class CalibrationTab : UserControl
    {
        private SerialPort _serialPort;
        private int filterCount = 0;
        private System.Windows.Forms.Timer updateTimer;

        public CalibrationTab(SerialPort serialPort)
        {
            InitializeComponent();
            _serialPort = serialPort;

            InitializeTimer();
            InitializeUI();
        }

        private void InitializeUI()
        {
            try {
                filterCount = GetFilterCount();
                LoadFilterNames();
                InitializePositionsGrid();
                RefreshAllAngles();
                UpdateButtonStates();
            }
            catch (Exception ex) {
                MessageBox.Show($"Error al inicializar: {ex.Message}");
            }
        }

        private void InitializeTimer()
        {
            updateTimer = new System.Windows.Forms.Timer();
            updateTimer.Interval = 500;
            updateTimer.Tick += UpdateTimer_Tick;
            updateTimer.Start();
        }

        private void UpdateTimer_Tick(object sender, EventArgs e)
        {
            if (!IsConnected()) return;

            try {
                float angle = GetCurrentEncoderAngle();
                lblEncoderAngle.Text = $"Ángulo: {angle:F2}°";

                int position = GetCurrentPosition();
                string filterName = GetFilterName(position);
                lblCurrentPosition.Text = $"Posición: {position} ({filterName})";
            }
            catch { }
        }

        private void btnApplyCurrentAngle_Click(object sender, EventArgs e)
        {
            try {
                ValidateConnectionBeforeAction();
                ValidateEncoderForCalibration();

                int position = (int)cmbTargetPosition.SelectedValue;
                float currentAngle = GetCurrentEncoderAngle();

                DialogResult result = MessageBox.Show(
                    $"¿Establecer posición {position} al ángulo actual?\n\n" +
                    $"Ángulo: {currentAngle:F2}°\n" +
                    $"Filtro: {GetFilterName(position)}",
                    "Confirmar Calibración",
                    MessageBoxButtons.YesNo,
                    MessageBoxIcon.Question
                );

                if (result == DialogResult.Yes) {
                    SetCustomAngle(position, currentAngle);
                }
            }
            catch (Exception ex) {
                MessageBox.Show($"Error: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void SetCustomAngle(int position, float angle)
        {
            ValidatePosition(position);
            ValidateAngle(angle);

            string response = SendCommand($"#SETANG{position}:{angle:F2}");
            LogMessage($"✓ Posición {position} calibrada a {angle:F2}°");
            RefreshAllAngles();
        }

        private void btnClearCalibration_Click(object sender, EventArgs e)
        {
            DialogResult result = MessageBox.Show(
                "¿Borrar toda la calibración personalizada?\n\nSe volverá a distribución uniforme.",
                "Confirmar",
                MessageBoxButtons.YesNo,
                MessageBoxIcon.Warning
            );

            if (result == DialogResult.Yes) {
                try {
                    string response = SendCommand("#CLEARANG");
                    LogMessage("Calibración borrada");
                    RefreshAllAngles();
                }
                catch (Exception ex) {
                    MessageBox.Show($"Error: {ex.Message}");
                }
            }
        }

        private void btnStepForward10_Click(object sender, EventArgs e)
        {
            ExecuteStepCommand("#SF10", "Avanzar 10 pasos");
        }

        private void btnStepBackward1_Click(object sender, EventArgs e)
        {
            ExecuteStepCommand("#SB1", "Retroceder 1 paso");
        }

        private void ExecuteStepCommand(string command, string description)
        {
            DisableMovementButtons();
            try {
                string response = SendCommand(command, timeout: 10000);
                LogMessage($"{description}: {response}");
                UpdateEncoderAngle();
            }
            finally {
                EnableMovementButtons();
            }
        }

        private void RefreshAllAngles()
        {
            var angles = GetAllCustomAngles();
            foreach (DataGridViewRow row in dgvPositions.Rows) {
                int position = row.Index + 1;
                if (angles.ContainsKey(position)) {
                    row.Cells["CurrentAngle"].Value = $"{angles[position]:F2}° (custom)";
                    row.Cells["DesiredAngle"].Value = angles[position].ToString("F2");
                }
            }
        }

        private int GetFilterCount()
        {
            string response = SendCommand("#GF");
            return int.Parse(response.Substring(1));
        }

        private int GetCurrentPosition()
        {
            string response = SendCommand("#GP");
            return int.Parse(response.Substring(1));
        }

        private float GetCurrentEncoderAngle()
        {
            string response = SendCommand("#ENCSTATUS");
            if (response.Contains("Not connected")) {
                throw new Exception("Encoder no disponible");
            }

            string data = response.Substring(10);
            string[] pairs = data.Split(',');

            foreach (string pair in pairs) {
                string[] parts = pair.Split('=');
                if (parts.Length == 2 && parts[0] == "Angle") {
                    return float.Parse(parts[1]);
                }
            }

            throw new Exception("No se pudo leer el ángulo");
        }

        private Dictionary<int, float> GetAllCustomAngles()
        {
            string response = SendCommand("#GETANG");
            var angles = new Dictionary<int, float>();

            if (response.Contains("No custom angles")) {
                return angles;
            }

            string anglesPart = response.Substring(7);
            string[] pairs = anglesPart.Split(',');

            foreach (string pair in pairs) {
                string[] parts = pair.Split('=');
                int position = int.Parse(parts[0]);
                string angleStr = parts[1].TrimEnd('°');
                float angle = float.Parse(angleStr);
                angles[position] = angle;
            }

            return angles;
        }

        private void LogMessage(string message)
        {
            string timestamp = DateTime.Now.ToString("HH:mm:ss");
            txtLog.AppendText($"{timestamp} - {message}\r\n");
            txtLog.ScrollToCaret();
        }

        private string SendCommand(string command, int timeout = 5000)
        {
            // Implementación...
            // (Ver sección anterior)
        }
    }
}
```

---

## Resumen de Requisitos

### ✅ Funcionalidades Obligatorias

1. **Visualización en tiempo real** del ángulo del encoder (500ms refresh)
2. **Botones de movimiento manual** (-100, -50, -10, -1, +1, +10, +50, +100 pasos)
3. **Control de pasos personalizados** (NumericUpDown + botones)
4. **Panel de calibración** dinámico (se adapta a 3-9 filtros)
5. **Campos de entrada** para ángulos deseados en cada posición
6. **Botón "Aplicar Ángulo Actual"** para calibración interactiva
7. **Log de eventos** con timestamp
8. **Validaciones** de conexión, encoder, y parámetros
9. **Confirmaciones** para acciones destructivas
10. **Importar/Exportar** calibración (opcional)

### ✅ Comandos que Debe Implementar

| Comando | Propósito | Frecuencia |
|---------|-----------|------------|
| `#GF` | Obtener número de filtros | Al iniciar |
| `#GN` | Obtener nombres de filtros | Al iniciar |
| `#GETANG` | Obtener todos los ángulos | Al iniciar, después de cambios |
| `#ENCSTATUS` | Obtener ángulo actual | Cada 500ms (timer) |
| `#GP` | Obtener posición actual | Cada 500ms (timer) |
| `#SF[steps]` | Avanzar motor | Botones de movimiento |
| `#SB[steps]` | Retroceder motor | Botones de movimiento |
| `#SETANG[pos]:[angle]` | Establecer ángulo | Al hacer clic "Calibrar" |
| `#CLEARANG` | Borrar calibración | Al hacer clic "Limpiar Calibración" |

---

## Notas Finales

- El firmware **ya está implementado** - solo necesitas la UI
- Todos los comandos están **probados y funcionando**
- La calibración se guarda **automáticamente** en EEPROM con cada `#SETANG`
- El sistema es **plug-and-play** - funciona con 3 a 9 filtros sin cambios
- La precisión es **<0.8°** con PID activado
- Los ángulos personalizados son **permanentes** hasta que se borren con `#CLEARANG`

---

**Flujo de trabajo simplificado:**
1. Usuario ajusta rueda con botones de movimiento
2. Usuario verifica ángulo en tiempo real
3. Usuario hace clic "Aplicar Ángulo Actual a Posición X" o ingresa ángulo manualmente
4. Sistema guarda inmediatamente en EEPROM
5. Repetir para todas las posiciones necesarias

**¡Listo para implementar!** 🚀
