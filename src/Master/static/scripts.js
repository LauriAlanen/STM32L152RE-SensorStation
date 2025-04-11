// Predefined fallback colors (extend as needed)
const fallbackColors = [
  'rgba(255, 99, 132, 1)',
  'rgba(54, 162, 235, 1)',
  'rgba(255, 206, 86, 1)',
  'rgba(75, 192, 192, 1)',
  'rgba(153, 102, 255, 1)',
  'rgba(255, 159, 64, 1)',
  'rgba(199, 199, 199, 1)'
];

const charts = {};
const units = {
  humidity: '%',
  temp: '°C',
  temperature: '°C',
  light: 'Lux',
  lux: 'Lux',
  co2: 'ppm',
  voc: 'ppb'
};

function getUnitForSensor(sensor) {
  for (const key in units) {
    if (sensor.toLowerCase().includes(key)) {
      return units[key];
    }
  }
  return '';
}

function getColor(index) {
  const color = fallbackColors[index % fallbackColors.length];
  return {
    border: color,
    background: color.replace('1)', '0.2)')
  };
}

function createChart(canvas, label, colorConfig) {
  return new Chart(canvas, {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: label,
        data: [],
        borderColor: colorConfig.border,
        backgroundColor: colorConfig.background,
        borderWidth: 1.5,
        pointRadius: 1,
        fill: true,
        tension: 0.1
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false, // This is already set correctly
      layout: {
        padding: {
          top: 5,
          right: 5,
          bottom: 5,
          left: 5
        }
      },
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
            color: '#a0a0a0',
            maxRotation: 45,
            minRotation: 45
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
            color: '#a0a0a0',
            padding: 5
          },
          beginAtZero: !label.toLowerCase().includes('light')
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

async function updateDashboard() {
  try {
    const response = await fetch('/data');
    const data = await response.json();
    const dashboard = document.getElementById('sensorDashboard');
    const template = document.getElementById('sensorCardTemplate');
    const MAX_POINTS = 100;
    const noChartMessage = document.getElementById('noChartMessage');

    Object.keys(data).forEach((sensor, index) => {
      let card = dashboard.querySelector(`.sensor-card[data-sensor="${sensor}"]`);
      const canvasId = `${sensor.replace(/\s+/g, '')}Chart`; // Remove spaces for ID

      // Create new card if it doesn't exist
      if (!card) {
        const clone = template.content.cloneNode(true);
        card = clone.querySelector('.sensor-card');
        card.dataset.sensor = sensor;

        // Set up card elements
        card.querySelector('.sensor-title').textContent = sensor;
        const canvas = card.querySelector('canvas');
        canvas.id = canvasId; // Set the canvas ID

        dashboard.appendChild(card);

        const unit = getUnitForSensor(sensor);
        const color = getColor(index).border; // Extract just the border color
        charts[sensor] = createChart(canvasId, `${sensor} (${unit})`, color);
      }

      // Update chart data
      const chart = charts[sensor];
      let values = data[sensor].values || [];
      let times = data[sensor].times || [];

      // Trim data to MAX_POINTS
      if (values.length > MAX_POINTS) {
        values = values.slice(-MAX_POINTS);
        times = times.slice(-MAX_POINTS);
      }

      // Update chart
      chart.data.labels = times;
      chart.data.datasets[0].data = values;
      chart.update();

      // Update card info
      const unit = getUnitForSensor(sensor);
      const currentValue = values.length > 0 ? values[values.length - 1] : null;
      const lastTime = times.length > 0 ? times[times.length - 1] : '--:--:--';
      const stats = calculateStats(values);

      if (currentValue !== null) {
        card.querySelector('.current-value').textContent = `${currentValue.toFixed(2)} ${unit}`;
        card.querySelector('.last-update').textContent = `Last update: ${lastTime}`;
        if (stats) {
          card.querySelector('.min-value').textContent = `${stats.min} ${unit}`;
          card.querySelector('.max-value').textContent = `${stats.max} ${unit}`;
          card.querySelector('.avg-value').textContent = `${stats.avg} ${unit}`;
        }
      }
    });
  } catch (err) {
    console.error("Dashboard update failed:", err);
  }
}

// Initial and periodic update
setInterval(updateDashboard, 2000);
updateDashboard();
