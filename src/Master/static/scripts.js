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
let sensorMetadata = {}; // This will store the metadata from the API

// Fetch sensor metadata from API
async function fetchSensorMetadata() {
  try {
    const response = await fetch('/sensor_metadata');
    sensorMetadata = await response.json();
    console.log("Sensor metadata loaded:", sensorMetadata);
  } catch (err) {
    console.error("Failed to fetch sensor metadata:", err);
    // Fallback to empty object if metadata fetch fails
    sensorMetadata = {};
  }
}

// Get both name and unit for a sensor with prefix
function getSensorDetails(sensorName) {
  // Extract channel number if present (e.g., "Makuuhuone_1" → channel "1")
  const channelMatch = sensorName.match(/(\d+)$/);
  const channel = channelMatch ? channelMatch[1] : null;
  
  // Remove any suffix after underscore for matching the base sensor name
  const baseSensorName = sensorName.replace(/_.*$/, "");

  // Check if sensor exists in metadata
  if (!sensorMetadata[baseSensorName] || !sensorMetadata[baseSensorName].channels) {
    console.warn(`Sensor "${baseSensorName}" not found in metadata`);
    
    // Fallback to common patterns if metadata not found
    const lowerName = sensorName.toLowerCase();
    if (lowerName.includes('humidity')) return { 
      name: `${baseSensorName} Humidity`, 
      unit: "%",
      fullName: `${baseSensorName} Humidity`
    };
    if (lowerName.includes('temp') || lowerName.includes('temperature')) return { 
      name: `${baseSensorName} Temperature`, 
      unit: "°C",
      fullName: `${baseSensorName} Temperature`
    };
    if (lowerName.includes('light') || lowerName.includes('lux')) return { 
      name: `${baseSensorName} Light`, 
      unit: "Lux",
      fullName: `${baseSensorName} Light`
    };
    if (lowerName.includes('co2')) return { 
      name: `${baseSensorName} CO₂`, 
      unit: "ppm",
      fullName: `${baseSensorName} CO₂`
    };
    if (lowerName.includes('voc')) return { 
      name: `${baseSensorName} VOC`, 
      unit: "ppb",
      fullName: `${baseSensorName} VOC`
    };
    
    return { 
      name: sensorName, 
      unit: "",
      fullName: sensorName
    };
  }

  const channels = sensorMetadata[baseSensorName].channels;

  // If a specific channel was requested
  if (channel && channels[channel]) {
    return {
      name: `${baseSensorName} ${channels[channel].name}`,
      unit: channels[channel].unit,
      fullName: `${baseSensorName} ${channels[channel].name}`
    };
  }

  // If no specific channel was requested, return first channel's data
  const firstChannel = Object.values(channels)[0];
  return {
    name: `${baseSensorName} ${firstChannel.name}`,
    unit: firstChannel.unit,
    fullName: `${baseSensorName} ${firstChannel.name}`
  };
}

async function initializeDashboard() {
  try {
    // FIRST ensure metadata is loaded
    await fetchSensorMetadata(); 
    
    console.log("Sensor metadata loaded successfully");
    
    // THEN start updates
    updateDashboard(); // First update with guaranteed metadata
    setInterval(updateDashboard, 2000);
    
    if (document.getElementById('loadingMessage')) {
      document.getElementById('loadingMessage').style.display = 'none';
    }
    
  } catch (err) {
    console.error("Initialization failed:", err);
    // Error handling remains the same
  }
}

function getColor(index) {
  const color = fallbackColors[index % fallbackColors.length];
  return {
    border: color,
    background: color.replace('1)', '0.2)')
  };
}

function createChart(canvas, sensorName, colorConfig) {
  const details = getSensorDetails(sensorName);
  
  return new Chart(canvas, {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: `${details.fullName} (${details.unit})`,
        data: [],
        borderColor: colorConfig.border,
        backgroundColor: colorConfig.background,
        borderWidth: 1.5,
        pointRadius: 1,
        fill: false,
        tension: 0.1
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
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
            text: details.fullName,
            color: '#a0a0a0'
          },
          grid: {
            color: 'rgba(255, 255, 255, 0.05)'
          },
          ticks: {
            color: '#a0a0a0',
            padding: 5
          },
          beginAtZero: !sensorName.toLowerCase().includes('light')
        }
      },
      plugins: {
        legend: { 
          display: true,
          position: 'top',
          labels: {
            color: '#a0a0a0'
          }
        },
        tooltip: {
          backgroundColor: '#2d2d2d',
          titleColor: '#ffffff',
          bodyColor: '#e0e0e0',
          borderColor: '#444',
          borderWidth: 1,
          mode: 'index',
          intersect: false
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

    Object.keys(data).forEach((sensor, index) => {
      let card = dashboard.querySelector(`.sensor-card[data-sensor="${sensor}"]`);
      const canvasId = `${sensor.replace(/\s+/g, '')}Chart`; // Remove spaces for ID

      // Create new card if it doesn't exist
      if (!card) {
        const clone = template.content.cloneNode(true);
        card = clone.querySelector('.sensor-card');
        card.dataset.sensor = sensor;

        // Set up card elements
        const details = getSensorDetails(sensor);
        card.querySelector('.sensor-title').textContent = details.fullName;
        const canvas = card.querySelector('canvas');
        canvas.id = canvasId;

        dashboard.appendChild(card);

        const colorConfig = getColor(index);
        charts[sensor] = createChart(canvasId, sensor, colorConfig);
      }

      // Update chart data
      const chart = charts[sensor];
      let values = data[sensor].values || [];
      let times = data[sensor].times || [];

      // Trim data to MAX_POINTS
      if (times.length > MAX_POINTS) {
        times = times.slice(-MAX_POINTS);
        values = values.slice(-MAX_POINTS);
      }

      // Update chart
      chart.data.labels = times;
      chart.data.datasets[0].data = values;
      chart.update();

      // Update card info
      const details = getSensorDetails(sensor);
      const currentValue = values.length > 0 ? values[values.length - 1] : null;
      if (currentValue !== null) {
        card.querySelector('.current-value').textContent = `${currentValue.toFixed(2)} ${details.unit}`;
      }
      
      const lastTime = times.length > 0 ? times[times.length - 1] : '--:--:--';
      card.querySelector('.last-update').textContent = `Last update: ${lastTime}`;
      
      // Update stats
      const stats = calculateStats(values);
      if (stats) {
        card.querySelector('.min-value').textContent = `${stats.min} ${details.unit}`;
        card.querySelector('.max-value').textContent = `${stats.max} ${details.unit}`;
        card.querySelector('.avg-value').textContent = `${stats.avg} ${details.unit}`;
      }
    });

    // Show message if no sensors found
    const noChartMessage = document.getElementById('noChartMessage');
    if (noChartMessage) {
      noChartMessage.style.display = Object.keys(data).length === 0 ? 'block' : 'none';
    }
  } catch (err) {
    console.error("Dashboard update failed:", err);
  }
}

// Start the initialization
initializeDashboard();