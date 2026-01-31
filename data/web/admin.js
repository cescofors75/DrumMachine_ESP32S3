// RED808 Admin Dashboard JavaScript

let ws = null;
let reconnectTimer = null;
let dataHistory = {
  heap: [],
  psram: [],
  timestamps: []
};
const MAX_HISTORY = 50;

// Initialize
document.addEventListener('DOMContentLoaded', () => {
  console.log('RED808 Admin Dashboard initializing...');
  connectWebSocket();
  startDataPolling();
  initChart();
});

// WebSocket Connection
function connectWebSocket() {
  const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
  const wsUrl = `${protocol}//${window.location.hostname}/ws`;
  
  console.log(`Connecting to ${wsUrl}...`);
  ws = new WebSocket(wsUrl);
  
  ws.onopen = () => {
    console.log('WebSocket connected!');
    updateConnectionStatus(true);
    refreshData();
  };
  
  ws.onclose = () => {
    console.log('WebSocket disconnected');
    updateConnectionStatus(false);
    scheduleReconnect();
  };
  
  ws.onerror = (error) => {
    console.error('WebSocket error:', error);
    updateConnectionStatus(false);
  };
  
  ws.onmessage = (event) => {
    try {
      const data = JSON.parse(event.data);
      handleWebSocketMessage(data);
    } catch (e) {
      console.error('Error parsing WebSocket message:', e);
    }
  };
}

function scheduleReconnect() {
  if (reconnectTimer) clearTimeout(reconnectTimer);
  reconnectTimer = setTimeout(() => {
    console.log('Attempting to reconnect...');
    connectWebSocket();
  }, 3000);
}

function updateConnectionStatus(connected) {
  const statusBadge = document.getElementById('connectionStatus');
  if (connected) {
    statusBadge.textContent = 'CONNECTED';
    statusBadge.className = 'status-badge connected';
  } else {
    statusBadge.textContent = 'DISCONNECTED';
    statusBadge.className = 'status-badge disconnected';
  }
}

function handleWebSocketMessage(data) {
  if (data.type === 'state') {
    updateSequencerStatus(data);
  }
}

// Data Polling
function startDataPolling() {
  // Poll system info every 2 seconds
  setInterval(() => {
    fetchSystemInfo();
  }, 2000);
}

async function fetchSystemInfo() {
  try {
    const response = await fetch('/api/sysinfo');
    const data = await response.json();
    updateDashboard(data);
  } catch (error) {
    console.error('Error fetching system info:', error);
  }
}

function updateDashboard(data) {
  // System Info
  document.getElementById('uptime').textContent = formatUptime(data.uptime);
  document.getElementById('ipAddress').textContent = data.ip || '-';
  document.getElementById('wifiChannel').textContent = data.channel || '-';
  document.getElementById('txPower').textContent = data.txPower || '-';
  document.getElementById('connectedStations').textContent = data.connectedStations || '0';
  
  // Memory - Heap
  const heapUsed = data.heapSize - data.heapFree;
  const heapPercent = ((heapUsed / data.heapSize) * 100).toFixed(1);
  document.getElementById('heapUsed').textContent = formatBytes(heapUsed);
  document.getElementById('heapTotal').textContent = formatBytes(data.heapSize);
  document.getElementById('heapPercent').textContent = `${heapPercent}%`;
  document.getElementById('heapProgress').style.width = `${heapPercent}%`;
  
  // Memory - PSRAM
  const psramUsed = data.psramSize - data.psramFree;
  const psramPercent = ((psramUsed / data.psramSize) * 100).toFixed(1);
  document.getElementById('psramUsed').textContent = formatBytes(psramUsed);
  document.getElementById('psramTotal').textContent = formatBytes(data.psramSize);
  document.getElementById('psramPercent').textContent = `${psramPercent}%`;
  document.getElementById('psramProgress').style.width = `${psramPercent}%`;
  
  // Memory - Samples
  document.getElementById('samplesLoaded').textContent = data.samplesLoaded || '0';
  document.getElementById('samplesMemory').textContent = formatBytes(data.memoryUsed || 0);
  const samplesPercent = ((data.memoryUsed / data.psramSize) * 100).toFixed(1);
  document.getElementById('samplesProgress').style.width = `${samplesPercent}%`;
  
  // Flash
  document.getElementById('flashSize').textContent = formatBytes(data.flashSize);
  
  // WebSocket Clients
  document.getElementById('wsClientCount').textContent = data.wsClients || '0';
  updateClientsList(data.wsClientList || []);
  
  // UDP Clients
  document.getElementById('udpClientCount').textContent = data.udpClients || '0';
  updateUdpClientsList(data.udpClientList || []);
  
  // Sequencer
  updateSequencerInfo(data);
  
  // Update chart
  updateChart(heapPercent, psramPercent);
}

function updateClientsList(clients) {
  const container = document.getElementById('wsClientsContainer');
  
  if (clients.length === 0) {
    container.innerHTML = '<div class="empty-state">No clients connected</div>';
    return;
  }
  
  container.innerHTML = clients.map(client => `
    <div class="client-card">
      <div class="client-id">Client #${client.id}</div>
      <div class="client-info">
        <div>IP: ${client.ip}</div>
        <div>Status: ${getClientStatus(client.status)}</div>
      </div>
    </div>
  `).join('');
}

function updateUdpClientsList(clients) {
  const container = document.getElementById('udpClientsContainer');
  
  if (clients.length === 0) {
    container.innerHTML = '<div class="empty-state">No UDP clients detected</div>';
    return;
  }
  
  container.innerHTML = clients.map(client => `
    <div class="client-card udp">
      <div class="client-id">📡 ${client.ip}:${client.port}</div>
      <div class="client-info">
        <div>Last seen: ${client.lastSeen}s ago</div>
        <div>Packets: ${client.packets}</div>
      </div>
    </div>
  `).join('');
}

function getClientStatus(status) {
  const statuses = {
    0: 'Disconnected',
    1: 'Connected',
    2: 'Disconnecting',
    3: 'Reconnecting'
  };
  return statuses[status] || 'Unknown';
}

function updateSequencerInfo(data) {
  document.getElementById('seqTempo').textContent = `${data.tempo || '-'} BPM`;
  document.getElementById('seqPattern').textContent = `Pattern ${(data.pattern || 0) + 1}`;
  
  const statusEl = document.getElementById('seqStatus');
  if (data.playing) {
    statusEl.innerHTML = '<span class="dot playing"></span> PLAYING';
  } else {
    statusEl.innerHTML = '<span class="dot stopped"></span> STOPPED';
  }
}

function updateSequencerStatus(data) {
  if (data.tempo) document.getElementById('seqTempo').textContent = `${data.tempo} BPM`;
  if (data.pattern !== undefined) document.getElementById('seqPattern').textContent = `Pattern ${data.pattern + 1}`;
  
  const statusEl = document.getElementById('seqStatus');
  if (data.playing) {
    statusEl.innerHTML = '<span class="dot playing"></span> PLAYING';
  } else {
    statusEl.innerHTML = '<span class="dot stopped"></span> STOPPED';
  }
}

// Chart
let chartCanvas, chartCtx;

function initChart() {
  chartCanvas = document.getElementById('memoryChart');
  chartCtx = chartCanvas.getContext('2d');
  drawChart();
}

function updateChart(heapPercent, psramPercent) {
  const now = new Date();
  dataHistory.heap.push(parseFloat(heapPercent));
  dataHistory.psram.push(parseFloat(psramPercent));
  dataHistory.timestamps.push(now.getSeconds());
  
  if (dataHistory.heap.length > MAX_HISTORY) {
    dataHistory.heap.shift();
    dataHistory.psram.shift();
    dataHistory.timestamps.shift();
  }
  
  drawChart();
}

function drawChart() {
  const width = chartCanvas.width;
  const height = chartCanvas.height;
  
  // Clear
  chartCtx.fillStyle = 'rgba(0, 0, 0, 0.3)';
  chartCtx.fillRect(0, 0, width, height);
  
  // Grid
  chartCtx.strokeStyle = 'rgba(255, 255, 255, 0.1)';
  chartCtx.lineWidth = 1;
  for (let i = 0; i <= 4; i++) {
    const y = (height / 4) * i;
    chartCtx.beginPath();
    chartCtx.moveTo(0, y);
    chartCtx.lineTo(width, y);
    chartCtx.stroke();
  }
  
  if (dataHistory.heap.length < 2) return;
  
  const pointSpacing = width / MAX_HISTORY;
  
  // Draw Heap line
  chartCtx.strokeStyle = '#00ff88';
  chartCtx.lineWidth = 2;
  chartCtx.beginPath();
  dataHistory.heap.forEach((value, i) => {
    const x = i * pointSpacing;
    const y = height - (value / 100) * height;
    if (i === 0) chartCtx.moveTo(x, y);
    else chartCtx.lineTo(x, y);
  });
  chartCtx.stroke();
  
  // Draw PSRAM line
  chartCtx.strokeStyle = '#ff00ff';
  chartCtx.lineWidth = 2;
  chartCtx.beginPath();
  dataHistory.psram.forEach((value, i) => {
    const x = i * pointSpacing;
    const y = height - (value / 100) * height;
    if (i === 0) chartCtx.moveTo(x, y);
    else chartCtx.lineTo(x, y);
  });
  chartCtx.stroke();
  
  // Labels
  chartCtx.fillStyle = '#00ff88';
  chartCtx.font = '12px monospace';
  chartCtx.fillText('Heap', 10, 20);
  
  chartCtx.fillStyle = '#ff00ff';
  chartCtx.fillText('PSRAM', 10, 35);
}

// Utility Functions
function formatBytes(bytes) {
  if (bytes === 0) return '0 B';
  const k = 1024;
  const sizes = ['B', 'KB', 'MB', 'GB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return Math.round((bytes / Math.pow(k, i)) * 100) / 100 + ' ' + sizes[i];
}

function formatUptime(ms) {
  const seconds = Math.floor(ms / 1000);
  const minutes = Math.floor(seconds / 60);
  const hours = Math.floor(minutes / 60);
  const days = Math.floor(hours / 24);
  
  if (days > 0) return `${days}d ${hours % 24}h`;
  if (hours > 0) return `${hours}h ${minutes % 60}m`;
  if (minutes > 0) return `${minutes}m ${seconds % 60}s`;
  return `${seconds}s`;
}

function refreshData() {
  fetchSystemInfo();
  console.log('Data refreshed');
}

function rebootESP() {
  if (confirm('¿Seguro que quieres reiniciar el ESP32?')) {
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({ cmd: 'reboot' }));
    }
    alert('Comando de reinicio enviado. El ESP32 se reiniciará en unos segundos.');
  }
}

function clearSamples() {
  if (confirm('¿Seguro que quieres descargar todos los samples de la memoria?')) {
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({ cmd: 'clearSamples' }));
    }
    alert('Comando enviado. Los samples se descargarán de la RAM.');
  }
}
