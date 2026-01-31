# Comandos de Filtros y Velocidad - RED808 DrumMachine

## 🎛️ Sistema de Filtros (10 Tipos Clásicos)

### Tipos de Filtros Disponibles

| ID | Nombre | Descripción | Preset Cutoff | Preset Q |
|----|--------|-------------|---------------|----------|
| 0 | None | Sin filtro | - | - |
| 1 | Low Pass | Paso bajo clásico | 1000 Hz | 1.0 |
| 2 | High Pass | Paso alto | 1000 Hz | 1.0 |
| 3 | Band Pass | Paso banda | 1000 Hz | 2.0 |
| 4 | Notch | Rechazo de banda | 1000 Hz | 2.0 |
| 5 | All Pass | Fase shift | 1000 Hz | 1.0 |
| 6 | Peaking EQ | EQ pico | 1000 Hz | 2.0 |
| 7 | Low Shelf | EQ graves | 500 Hz | 1.0 |
| 8 | High Shelf | EQ agudos | 4000 Hz | 1.0 |
| 9 | Resonant | Resonante (high Q) | 1000 Hz | 10.0 |

---

## 📡 Comandos WebSocket/UDP

### 1. Filtros por Pista (Sequencer Tracks)

**Máximo**: 8 filtros activos simultáneos para todas las pistas

#### Aplicar Filtro a Pista
```json
{
  "cmd": "setTrackFilter",
  "track": 0,          // 0-15
  "filterType": 1,     // 0-9 (ver tabla arriba)
  "cutoff": 1000.0,    // 100-16000 Hz (opcional)
  "resonance": 2.0,    // 0.5-20.0 (opcional)
  "gain": 6.0          // -12 a +12 dB (opcional, para EQ)
}
```

**Respuesta**:
```json
{
  "type": "trackFilterSet",
  "track": 0,
  "success": true,
  "activeFilters": 3
}
```

#### Limpiar Filtro de Pista
```json
{
  "cmd": "clearTrackFilter",
  "track": 0
}
```

**Respuesta**:
```json
{
  "type": "trackFilterCleared",
  "track": 0,
  "activeFilters": 2
}
```

---

### 2. Filtros por Pad (Live Pads)

**Máximo**: 8 filtros activos simultáneos para todos los pads

#### Aplicar Filtro a Pad
```json
{
  "cmd": "setPadFilter",
  "pad": 0,            // 0-15
  "filterType": 2,     // 0-9
  "cutoff": 2000.0,    // 100-16000 Hz (opcional)
  "resonance": 1.5,    // 0.5-20.0 (opcional)
  "gain": 0.0          // -12 a +12 dB (opcional)
}
```

**Respuesta**:
```json
{
  "type": "padFilterSet",
  "pad": 0,
  "success": true,
  "activeFilters": 1
}
```

#### Limpiar Filtro de Pad
```json
{
  "cmd": "clearPadFilter",
  "pad": 0
}
```

---

### 3. Obtener Presets de Filtros
```json
{
  "cmd": "getFilterPresets"
}
```

**Respuesta**:
```json
{
  "type": "filterPresets",
  "presets": [
    {
      "id": 0,
      "name": "None",
      "cutoff": 0.0,
      "resonance": 1.0,
      "gain": 0.0
    },
    {
      "id": 1,
      "name": "Low Pass",
      "cutoff": 1000.0,
      "resonance": 1.0,
      "gain": 0.0
    }
    // ... resto de presets
  ]
}
```

---

## 🎵 Edición de Velocidad por Step

### Establecer Velocidad de un Step
```json
{
  "cmd": "setStepVelocity",
  "track": 0,          // 0-15
  "step": 4,           // 0-15
  "velocity": 80       // 1-127
}
```

**Respuesta** (broadcast a todos los clientes):
```json
{
  "type": "stepVelocitySet",
  "track": 0,
  "step": 4,
  "velocity": 80
}
```

### Obtener Velocidad de un Step
```json
{
  "cmd": "getStepVelocity",
  "track": 0,
  "step": 4
}
```

**Respuesta**:
```json
{
  "type": "stepVelocity",
  "track": 0,
  "step": 4,
  "velocity": 80
}
```

### Obtener Patrón con Velocidades
```json
{
  "cmd": "getPattern"
}
```

**Respuesta** (incluye steps Y velocidades):
```json
{
  "type": "pattern",
  "index": 0,
  "0": [false, false, true, ...],  // Steps track 0
  "1": [false, false, false, ...], // Steps track 1
  // ... tracks 2-15
  "velocities": {
    "0": [127, 127, 90, ...],      // Velocidades track 0
    "1": [127, 127, 127, ...],     // Velocidades track 1
    // ... tracks 2-15
  }
}
```

---

## ⌨️ Control por Teclado (Propuesta de Implementación)

### Modo Edición de Velocidad
1. **Seleccionar celda**: Click en step del sequencer
2. **Ajustar velocidad**: 
   - `↑` / `↓`: Incrementar/Decrementar de 10 en 10
   - `Shift + ↑/↓`: Incrementar/Decrementar de 1 en 1
   - Teclas numéricas `1-9`: Velocidad fija (1=14, 2=28, ..., 9=127)
   - `0`: Velocidad 127 (máxima)
   - `Q`: Velocity 40 (ghost note)
   - `W`: Velocity 70 (suave)
   - `E`: Velocity 100 (medio)
   - `R`: Velocity 127 (acento)

### Visualización de Velocidad
- **Step activo**: Color base del instrumento
- **Velocidad baja (1-50)**: Opacidad ~0.3 (alpha degradado)
- **Velocidad media (51-90)**: Opacidad ~0.6
- **Velocidad alta (91-127)**: Opacidad 1.0 (sólido)

### Modo Filtros Rápidos por Teclado
1. **Seleccionar pista/pad**: Click
2. **Aplicar filtro**: 
   - `F1`: Low Pass 500Hz Q:2
   - `F2`: High Pass 1000Hz Q:2
   - `F3`: Band Pass 1000Hz Q:4
   - `F4`: Resonant 800Hz Q:10
   - `F5`: Low Shelf 300Hz Gain:+6dB
   - `F6`: High Shelf 5000Hz Gain:+6dB
   - `F7`: Peaking EQ 2000Hz Q:3 Gain:+6dB
   - `F8`: Notch 1000Hz Q:5
   - `F9`: All Pass 1000Hz
   - `F10`: Limpiar filtro
   - `Shift + F1-F10`: Aplicar a pad seleccionado

---

## 🎨 Ejemplo de Uso Completo

### Crear un Kick con Low Pass
```javascript
// JavaScript en la web
// 1. Aplicar filtro al kick (track 0)
ws.send(JSON.stringify({
  cmd: "setTrackFilter",
  track: 0,
  filterType: 1,      // Low Pass
  cutoff: 800.0,
  resonance: 3.0
}));

// 2. Configurar velocidades dinámicas
ws.send(JSON.stringify({
  cmd: "setStepVelocity",
  track: 0,
  step: 0,
  velocity: 127       // Acento fuerte
}));

ws.send(JSON.stringify({
  cmd: "setStepVelocity",
  track: 0,
  step: 4,
  velocity: 90        // Medio
}));

ws.send(JSON.stringify({
  cmd: "setStepVelocity",
  track: 0,
  step: 12,
  velocity: 70        // Ghost note
}));
```

### Hi-Hat con High Pass
```javascript
// Aplicar high pass al hi-hat (track 2)
ws.send(JSON.stringify({
  cmd: "setTrackFilter",
  track: 2,
  filterType: 2,      // High Pass
  cutoff: 3000.0,
  resonance: 1.5
}));
```

### Pad con Resonant Filter (efectos especiales)
```javascript
// Aplicar filtro resonante al pad 5
ws.send(JSON.stringify({
  cmd: "setPadFilter",
  pad: 5,
  filterType: 9,      // Resonant
  cutoff: 1200.0,
  resonance: 12.0
}));
```

---

## 📊 Límites y Reglas

1. **Filtros por pista**: Máximo 8 activos simultáneos
2. **Filtros por pad**: Máximo 8 activos simultáneos
3. **Velocidad**: Rango 1-127 (0 = step inactivo)
4. **Cutoff**: 100 - 16000 Hz
5. **Resonance (Q)**: 0.5 - 20.0
6. **Gain (EQ)**: -12 dB a +12 dB

---

## 🔧 Implementación Recomendada en Web UI

### HTML - Selector de Velocidad
```html
<div class="velocity-editor" style="display:none;">
  <label>Velocity: <span id="vel-value">127</span></label>
  <input type="range" id="vel-slider" min="1" max="127" value="127">
  <div class="velocity-presets">
    <button onclick="setVel(40)">Ghost</button>
    <button onclick="setVel(70)">Soft</button>
    <button onclick="setVel(100)">Medium</button>
    <button onclick="setVel(127)">Accent</button>
  </div>
</div>
```

### CSS - Visualización de Velocidad
```css
.step.active {
  background-color: var(--track-color);
  /* Opacidad basada en velocidad */
  opacity: calc(var(--velocity) / 127);
}

.step.active[data-velocity="127"] {
  opacity: 1.0;
  box-shadow: 0 0 8px var(--track-color);
}

.step.active[data-velocity="40"] {
  opacity: 0.3;
}
```

### JavaScript - Control de Teclado
```javascript
let selectedStep = null;

document.addEventListener('keydown', (e) => {
  if (!selectedStep) return;
  
  const track = selectedStep.track;
  const step = selectedStep.step;
  let currentVel = getCurrentVelocity(track, step);
  
  switch(e.key) {
    case 'ArrowUp':
      currentVel = Math.min(127, currentVel + (e.shiftKey ? 1 : 10));
      break;
    case 'ArrowDown':
      currentVel = Math.max(1, currentVel - (e.shiftKey ? 1 : 10));
      break;
    case 'q': currentVel = 40; break;  // Ghost
    case 'w': currentVel = 70; break;  // Soft
    case 'e': currentVel = 100; break; // Medium
    case 'r': currentVel = 127; break; // Accent
  }
  
  setStepVelocity(track, step, currentVel);
});

function setStepVelocity(track, step, velocity) {
  ws.send(JSON.stringify({
    cmd: 'setStepVelocity',
    track: track,
    step: step,
    velocity: velocity
  }));
  
  // Actualizar UI
  updateStepOpacity(track, step, velocity);
}
```

---

## 🎛️ Panel de Filtros (UI Propuesto)

```html
<div class="filter-panel">
  <h3>Track Filters (Max 8)</h3>
  <select id="filter-type">
    <option value="0">None</option>
    <option value="1">Low Pass</option>
    <option value="2">High Pass</option>
    <option value="3">Band Pass</option>
    <option value="4">Notch</option>
    <option value="5">All Pass</option>
    <option value="6">Peaking EQ</option>
    <option value="7">Low Shelf</option>
    <option value="8">High Shelf</option>
    <option value="9">Resonant</option>
  </select>
  
  <label>Cutoff: <span id="cutoff-val">1000</span> Hz</label>
  <input type="range" id="cutoff" min="100" max="16000" value="1000">
  
  <label>Resonance: <span id="res-val">1.0</span></label>
  <input type="range" id="resonance" min="0.5" max="20" step="0.1" value="1.0">
  
  <label>Gain: <span id="gain-val">0</span> dB</label>
  <input type="range" id="gain" min="-12" max="12" step="0.5" value="0">
  
  <button onclick="applyTrackFilter()">Apply to Track</button>
  <button onclick="applyPadFilter()">Apply to Pad</button>
  <button onclick="clearFilter()">Clear</button>
  
  <div class="filter-status">
    Active Track Filters: <span id="track-filter-count">0</span>/8<br>
    Active Pad Filters: <span id="pad-filter-count">0</span>/8
  </div>
</div>
```

---

## 🚀 Casos de Uso Avanzados

### 1. Kick Profundo con Low Pass
- Track 0 (BD)
- Filter: Low Pass 400Hz Q:2.5
- Velocities: 127, 90, 127, 80 (pattern bombo)

### 2. Hi-Hat Brillante con High Shelf
- Track 2 (CH)
- Filter: High Shelf 8000Hz Gain:+8dB
- Velocities: Alternadas 110, 70, 110, 70 (swing)

### 3. Snare con Notch (quitar frecuencia molesta)
- Track 1 (SD)
- Filter: Notch 800Hz Q:6
- Velocities: Ghost notes 50, acentos 127

### 4. Pad Atmosférico con Band Pass
- Pad 8
- Filter: Band Pass 2000Hz Q:8
- Reproducción manual con velocidad dinámica

---

## 📝 Notas Técnicas

- Los filtros se procesan en tiempo real usando algoritmos Biquad
- La latencia es mínima (<2ms) gracias a optimización DSP
- Los filtros por pista se aplican solo al audio del sequencer
- Los filtros por pad se aplican al audio de los live pads
- Las velocidades se almacenan por patrón (16 patrones × 16 tracks × 16 steps)
- La visualización de velocidad usa opacidad CSS (alpha channel)

---

## ✅ Estado de Implementación

- ✅ Backend: 10 tipos de filtros implementados
- ✅ Backend: Filtros por pista (max 8)
- ✅ Backend: Filtros por pad (max 8)
- ✅ Backend: Edición de velocidad por step
- ✅ Backend: Comandos WebSocket/UDP
- ⏳ Frontend: UI de edición de velocidad (pendiente)
- ⏳ Frontend: Panel de filtros (pendiente)
- ⏳ Frontend: Control de teclado (pendiente)
- ⏳ Frontend: Visualización con opacidad (pendiente)

---

Compilado exitosamente: **959,373 bytes** (22.9% Flash usado)
