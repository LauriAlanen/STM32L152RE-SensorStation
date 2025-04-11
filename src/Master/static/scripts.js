// Dark mode chart colors
const chartColors = {
  dht22_humidity: 'rgba(100, 181, 246, 1)',
  dht22_temp: 'rgba(244, 143, 177, 1)',
  lmt84: 'rgba(77, 208, 225, 1)',
  ns1l9m51: 'rgba(255, 183, 77, 1)',
  sgp30_co2: 'rgba(179, 157, 219, 1)',
  sgp30_voc: 'rgba(255, 213, 79, 1)'
};

// Initialize all charts with dark mode
const charts = {
  dht22_humidity: createChart('dht22HumidityChart', 'Humidity (%)', chartColors.dht22_humidity),
  dht22_temp: createChart('dht22TempChart', 'Temperature (°C)', chartColors.dht22_temp),
  lmt84: createChart('lmt84Chart', 'Temperature (°C)', chartColors.lmt84),
  ns1l9m51: createChart('ns1l9m51Chart', 'Light (Lux)', chartColors.ns1l9m51),
  sgp30_co2: createChart('sgp30Co2Chart', 'CO2 (ppm)', chartColors.sgp30_co2),
  sgp30_voc: createChart('sgp30VocChart', 'VOC (ppb)', chartColors.sgp30_voc)
};

function createChart(canvasId, label, color) {
  const ctx = document.getElementById(canvasId).getContext('2d');
  return new Chart(ctx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: label,
        data: [],
        borderColor: color,
        backgroundColor: color.replace('1)', '0.2)'),
        borderWidth: 1.5,
        pointRadius: 1,
        fill: true,
        tension: 0.1
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        x: {
          title: {
            display: true,
            text: 'Time',
            color: '#a0a0a0'
          },
          grid: {
            color: 'rgba(255, 255, 255, 0.05)'
          },
          ticks: {
            color: '#a0a0a0'
          }
        },
        y: {
          title: {
            display: true,
            text: label,
            color: '#a0a0a0'
          },
          grid: {
            color: 'rgba(255, 255, 255, 0.05)'
          },
          ticks: {
            color: '#a0a0a0'
          },
          beginAtZero: canvasId !== 'ns1l9m51Chart'
        }
      },
      plugins: {
        legend: { display: false },
        tooltip: {
          backgroundColor: '#2d2d2d',
          titleColor: '#ffffff',
          bodyColor: '#e0e0e0',
          borderColor: '#444',
          borderWidth: 1
        }
      }
    }
  });
}

// Calculate statistics
function calculateStats(values) {
  if (!values || values.length === 0) return null;

  const min = Math.min(...values);
  const max = Math.max(...values);
  const avg = values.reduce((sum, val) => sum + val, 0) / values.length;

  return {
    min: min.toFixed(2),
    max: max.toFixed(2),
    avg: avg.toFixed(2)
  };
}

// Update dashboard with new data
async function updateDashboard() {
  try {
    const response = await fetch('/data');
    const data = await response.json();

    // Process each sensor
    Object.keys(charts).forEach(sensor => {
      if (!data[sensor]) return;

      const values = data[sensor].values || [];
      const times = data[sensor].times || [];
      const currentValue = values.length > 0 ? values[values.length - 1] : null;
      const lastTime = times.length > 0 ? times[times.length - 1] : '--:--:--';
      const stats = calculateStats(values);

      // Update chart
      charts[sensor].data.labels = times;
      charts[sensor].data.datasets[0].data = values;
      charts[sensor].update();

      // Update UI elements
      if (currentValue !== null) {
        const unit = getUnitForSensor(sensor);
        document.getElementById(`${sensor}Current`).textContent = `${currentValue.toFixed(2)} ${unit}`;
        document.getElementById(`${sensor}Time`).textContent = `Last update: ${lastTime}`;

        if (stats) {
          document.getElementById(`${sensor}Min`).textContent = `${stats.min} ${unit}`;
          document.getElementById(`${sensor}Max`).textContent = `${stats.max} ${unit}`;
          document.getElementById(`${sensor}Avg`).textContent = `${stats.avg} ${unit}`;
        }
      }
    });
  } catch (error) {
    console.error('Error fetching sensor data:', error);
  }
}

function getUnitForSensor(sensor) {
  const units = {
    dht22_humidity: '%',
    dht22_temp: '°C',
    lmt84: '°C',
    ns1l9m51: 'Lux',
    sgp30_co2: 'ppm',
    sgp30_voc: 'ppb'
  };
  return units[sensor] || '';
}

// Update every 2 seconds
setInterval(updateDashboard, 2000);
updateDashboard(); // Initial update
