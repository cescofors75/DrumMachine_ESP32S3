# Track Volume Feature - RED808 DrumMachine

## Descripción

Sistema de volúmenes independientes por track en el secuenciador RED808. Cada uno de los 8 tracks puede tener su propio nivel de volumen (0-100%), permitiendo mezclas más dinámicas y expresivas.

---

## Arquitectura

### 1. Backend (C++)

#### Sequencer (`Sequencer.h/cpp`)
- **Array de volúmenes**: `uint8_t trackVolume[MAX_TRACKS]` (8 tracks)
- **Valor por defecto**: 100% para todos los tracks
- **Rango**: 0-100%

**Métodos públicos**:
```cpp
void setTrackVolume(int track, uint8_t volume);  // Establecer volumen (0-100)
uint8_t getTrackVolume(int track);               // Obtener volumen
```

**Callback actualizado**:
```cpp
typedef void (*StepCallback)(int track, uint8_t velocity, uint8_t trackVolume);
```
El callback ahora pasa 3 parámetros: track, velocity del step, y volumen del track.

#### AudioEngine (`AudioEngine.h/cpp`)

**Cálculo de volumen final**:
```cpp
voices[voiceIndex].volume = (sequencerVolume * trackVolume) / 100;
```

- `sequencerVolume`: Volumen global del sequencer (0-150%)
- `trackVolume`: Volumen del track individual (0-100%)
- **Resultado**: Multiplicación normalizada

**Ejemplo**: 
- Sequencer Volume = 80%
- Track Volume = 50%
- Volumen Final = (80 * 50) / 100 = 40%

---

### 2. Frontend (Web Interface)

#### UI Changes

**Botón de Volumen** (reemplaza botón Mute):
- Icono: **"V"** (verde)
- Posición: Esquina inferior izquierda de cada track label
- Click: Abre menú vertical con slider

**Menú de Volumen**:
```
┌────────────┐
│   75%      │  ← Valor actual
│            │
│    ║       │  ← Slider vertical
│    ║█      │     (120px altura)
│    ║       │
│    ║       │
│            │
└────────────┘
```

#### Estilos CSS

```css
.volume-btn {
    border: 1px solid rgba(76, 175, 80, 0.5);
    background: rgba(76, 175, 80, 0.15);
    color: #4caf50;
}

.volume-menu {
    position: fixed;
    background: rgba(20, 20, 20, 0.95);
    border: 2px solid #4caf50;
    padding: 10px;
    backdrop-filter: blur(10px);
}
```

#### JavaScript (`app.js`)

**Variables globales**:
```javascript
let trackVolumes = new Array(8).fill(100); // Default 100%
let activeVolumeMenu = null;
```

**Funciones principales**:
```javascript
showVolumeMenu(track, button)      // Mostrar menú de volumen
updateTrackVolume(track, volume)   // Actualizar volumen localmente
closeVolumeMenuOnClickOutside(e)   // Cerrar menú al click fuera
```

---

### 3. Comunicación (WebSocket & UDP)

#### WebSocket Commands

**Establecer volumen de track**:
```json
{
  "cmd": "setTrackVolume",
  "track": 0,
  "volume": 75
}
```

**Respuesta broadcast a todos los clientes**:
```json
{
  "type": "trackVolumeSet",
  "track": 0,
  "volume": 75
}
```

**Obtener volumen de un track**:
```json
{
  "cmd": "getTrackVolume",
  "track": 0
}
```

**Respuesta**:
```json
{
  "type": "trackVolume",
  "track": 0,
  "volume": 75
}
```

**Obtener todos los volúmenes**:
```json
{
  "cmd": "getTrackVolumes"
}
```

**Respuesta**:
```json
{
  "type": "trackVolumes",
  "volumes": [100, 75, 90, 100, 80, 100, 100, 100]
}
```

**Estado inicial (incluido en "state")**:
```json
{
  "type": "state",
  "trackVolumes": [100, 75, 90, 100, 80, 100, 100, 100],
  ...
}
```

#### UDP Commands (idénticos a WebSocket)

Los mismos comandos funcionan vía UDP (puerto 8888):

```cpp
// Ejemplo en Arduino/ESP32
void setTrackVolume(int track, int volume) {
  StaticJsonDocument<128> doc;
  doc["cmd"] = "setTrackVolume";
  doc["track"] = track;
  doc["volume"] = volume;
  
  String json;
  serializeJson(doc, json);
  udp.beginPacket(masterIP, 8888);
  udp.print(json);
  udp.endPacket();
}
```

---

## Casos de Uso

### 1. Mezcla Dinámica durante Performance
```javascript
// Bajar kick progresivamente
for (let vol = 100; vol >= 50; vol -= 10) {
  sendWebSocket({cmd: 'setTrackVolume', track: 0, volume: vol});
  await sleep(500);
}
```

### 2. Destacar Elementos Rítmicos
```json
// Énfasis en snare y hi-hat
{"cmd":"setTrackVolume","track":1,"volume":120}  // Snare
{"cmd":"setTrackVolume","track":2,"volume":110}  // Hi-hat
{"cmd":"setTrackVolume","track":0,"volume":70}   // Kick (menos)
```

### 3. Fade Out de Tracks Individuales
```cpp
// Arduino: Fade out gradual
for (int vol = 100; vol >= 0; vol -= 5) {
  setTrackVolume(0, vol);  // BD
  delay(100);
}
```

### 4. Sincronización Multi-Dispositivo
```json
// Dispositivo SLAVE lee volúmenes del MASTER
{"cmd":"getTrackVolumes"}

// Respuesta con estado actual
{"type":"trackVolumes","volumes":[100,75,90,100,80,100,100,100]}

// SLAVE aplica localmente o reenvía cambios
```

---

## Interacción con Otros Sistemas

### Volumen Sequencer Global
- **Relación**: Multiplicativo
- **Ejemplo**: 
  - Sequencer Volume = 80%
  - Track 0 Volume = 50%
  - **Volumen Final Track 0 = 40%**

### Velocity per Step
- **Relación**: Multiplicativo también
- **Cálculo completo**:
  ```
  Volumen Final = (MasterVolume * SequencerVolume * TrackVolume * Velocity) / (100 * 100 * 127)
  ```

### Filtros por Track
- **Independiente**: Los filtros se aplican DESPUÉS del volumen
- Los filtros no afectan el volumen, solo el timbre

---

## Migración de Mute a Volume

### Antes (Mute)
```javascript
// Botón mute (on/off binario)
muteBtn.addEventListener('click', () => {
  setTrackMuted(track, !trackMutedState[track]);
});
```

### Ahora (Volume)
```javascript
// Botón volume (control continuo 0-100)
volumeBtn.addEventListener('click', (e) => {
  showVolumeMenu(track, e.target);
});
```

**Simulación de Mute**:
```javascript
// Para silenciar completamente: setTrackVolume(track, 0)
// Para reactivar: setTrackVolume(track, 100)
```

---

## Debugging & Testing

### Serial Monitor (ESP32)

```
[AudioEngine] *** SEQ TRACK 0 -> Voice 0, Vel: 127, Vol: 40% (Seq:80% x Track:50%) ***
Track 0 volume set to 50%
```

### Console Web (Chrome DevTools)

```javascript
// Ver volúmenes actuales
console.log(trackVolumes); // [100, 75, 90, 100, 80, 100, 100, 100]

// Cambiar volumen
sendWebSocket({cmd: 'setTrackVolume', track: 0, volume: 50});

// Verificar cambio
Track 0 volume set to 50%
```

### UDP Test (Python)

```python
import socket
import json

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Establecer volumen
data = {"cmd": "setTrackVolume", "track": 0, "volume": 75}
sock.sendto(json.dumps(data).encode(), ("192.168.4.1", 8888))

# Leer respuesta
response, addr = sock.recvfrom(1024)
print(json.loads(response))  # {"status":"ok"}
```

---

## Notas de Implementación

1. **Rango 0-100**: Más intuitivo que 0-127 (MIDI) para usuarios
2. **Persistencia**: Los volúmenes NO se guardan entre reinicios (solo durante sesión)
3. **Broadcast**: Cambios se propagan a TODOS los clientes WebSocket conectados
4. **UDP**: Sin garantía de entrega, pero respuesta inmediata
5. **Slider vertical**: Mejor UX para control de volumen (como faders físicos)
6. **Click fuera cierra**: Menú se cierra automáticamente al hacer click fuera

---

## Futuras Mejoras

- [ ] Persistencia en EEPROM/SPIFFS
- [ ] Curvas de volumen (lineal/logarítmica/exponencial)
- [ ] Grupos de tracks (control simultáneo)
- [ ] Automation de volumen (envelopes)
- [ ] MIDI CC mapping para control externo
- [ ] Indicador visual de volumen en track label
- [ ] Volumen por pattern (diferentes volúmenes por patrón)

---

## Archivos Modificados

```
src/Sequencer.h           ✓ Array trackVolume, métodos getter/setter
src/Sequencer.cpp         ✓ Implementación, inicialización, callback
src/AudioEngine.h         ✓ Firma triggerSampleSequencer actualizada
src/AudioEngine.cpp       ✓ Aplicación de volumen en mixeo
src/main.cpp              ✓ Callback onStepTrigger con 3 parámetros
src/WebInterface.cpp      ✓ Comandos setTrackVolume, getTrackVolume, getTrackVolumes
data/web/app.js           ✓ Menú volumen, estado, handlers WebSocket
data/web/style.css        ✓ Estilos volume-btn, volume-menu, slider
UDP_PROTOCOL_*.md         ✓ Documentación actualizada
```

---

**Fecha**: 3 de febrero de 2026  
**Versión**: 1.0  
**Autor**: RED808 Development Team
