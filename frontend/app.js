const WS_URL = 'ws://localhost:8765';
const MAX_DATA_POINTS = 60; // 1 minute of history at 1Hz

// Chart Configuration
const ctx = document.getElementById('historyChart').getContext('2d');
Chart.defaults.color = '#94a3b8';
Chart.defaults.font.family = 'Inter';

const historyChart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: [],
        datasets: [
            {
                label: 'eCO₂ (ppm)',
                borderColor: '#3b82f6',
                backgroundColor: 'rgba(59, 130, 246, 0.1)',
                borderWidth: 2,
                tension: 0.4,
                pointRadius: 0,
                fill: true,
                yAxisID: 'y',
                data: []
            },
            {
                label: 'TVOC (ppb)',
                borderColor: '#10b981',
                backgroundColor: 'rgba(16, 185, 129, 0.1)',
                borderWidth: 2,
                tension: 0.4,
                pointRadius: 0,
                fill: true,
                yAxisID: 'y1',
                data: []
            }
        ]
    },
    options: {
        responsive: true,
        maintainAspectRatio: false,
        animation: { duration: 0 }, // Smooth realtime updates
        interaction: { mode: 'index', intersect: false },
        scales: {
            x: {
                grid: { color: 'rgba(255,255,255,0.05)' },
                ticks: { maxRotation: 0, autoSkip: true, maxTicksLimit: 6 }
            },
            y: {
                type: 'linear', display: true, position: 'left',
                grid: { color: 'rgba(255,255,255,0.05)' },
                title: { display: true, text: 'ppm' }
            },
            y1: {
                type: 'linear', display: true, position: 'right',
                grid: { drawOnChartArea: false },
                title: { display: true, text: 'ppb' }
            }
        }
    }
});

// UI Elements
const statusBadge = document.getElementById('connection-status');
const alertBanner = document.getElementById('alert-banner');
const staleOverlay = document.getElementById('stale-overlay');
const els = {
    eco2: document.getElementById('val-eco2'),
    tvoc: document.getElementById('val-tvoc'),
    gas: document.getElementById('val-gas'),
    cardEco2: document.getElementById('card-eco2')
};

let ws;
let reconnectTimer;

function connect() {
    ws = new WebSocket(WS_URL);
    
    ws.onopen = () => {
        statusBadge.textContent = 'Connected';
        statusBadge.className = 'status-badge connected';
        staleOverlay.classList.add('hidden');
    };
    
    ws.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);
            handleData(data);
        } catch (e) {
            console.error('Failed to parse WebSocket message', e);
        }
    };
    
    ws.onclose = () => {
        statusBadge.textContent = 'Disconnected';
        statusBadge.className = 'status-badge disconnected';
        staleOverlay.classList.remove('hidden');
        clearTimeout(reconnectTimer);
        reconnectTimer = setTimeout(connect, 3000); // 3s reconnect loop (satisfies SC-002)
    };
    
    ws.onerror = (err) => {
        console.error('WebSocket error', err);
        ws.close();
    };
}

function handleData(payload) {
    if (payload.status === 'ERROR') {
        alertBanner.classList.remove('hidden');
        document.getElementById('alert-message').textContent = "Sensor Error detected!";
        return;
    }

    const { metrics, timestamp, alerts } = payload;
    const timeLabel = new Date(timestamp).toLocaleTimeString([], {hour: '2-digit', minute:'2-digit', second:'2-digit'});

    // Update DOM texts
    if (metrics.eco2 !== undefined) els.eco2.textContent = metrics.eco2;
    if (metrics.tvoc !== undefined) els.tvoc.textContent = metrics.tvoc;
    if (metrics.gasResistance !== undefined) els.gas.textContent = (metrics.gasResistance / 1000).toFixed(1);

    // Alerts Threshold Logic
    if (alerts && alerts.length > 0) {
        alertBanner.classList.remove('hidden');
        document.getElementById('alert-message').textContent = alerts.join(" | ");
        els.cardEco2.classList.add('danger');
    } else {
        alertBanner.classList.add('hidden');
        els.cardEco2.classList.remove('danger');
    }

    // Update Chart
    historyChart.data.labels.push(timeLabel);
    historyChart.data.datasets[0].data.push(metrics.eco2 || null);
    historyChart.data.datasets[1].data.push(metrics.tvoc || null);

    if (historyChart.data.labels.length > MAX_DATA_POINTS) {
        historyChart.data.labels.shift();
        historyChart.data.datasets[0].data.shift();
        historyChart.data.datasets[1].data.shift();
    }
    
    historyChart.update();
}

// Start connection
connect();
