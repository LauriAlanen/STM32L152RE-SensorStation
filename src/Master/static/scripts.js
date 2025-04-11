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
  } catch (err) {
    console.error("Failed to fetch sensor metadata:", err);
    // Fallback to empty object if metadata fetch fails
    sensorMetadata = {};
  }
}

// Initial fetch of sensor metadata and then start the dashboard updates
async function initializeDashboard() {
  try {
    await fetchSensorMetadata();
    console.log("Sensor metadata loaded successfully");
    console.log(sensorMetadata); // Log the metadata for debugging
    
    // Initial and periodic update
    updateDashboard(); // Do first update immediately
    setInterval(updateDashboard, 2000); // Then every 2 seconds
    
    // Hide loading message if exists
    const loadingMessage = document.getElementById('loadingMessage');
    if (loadingMessage) loadingMessage.style.display = 'none';
    
  } catch (err) {
    console.error("Failed to initialize dashboard:", err);
    // Show error message to user
    const errorElement = document.getElementById('errorMessage') || document.createElement('div');
    errorElement.id = 'errorMessage';
    errorElement.textContent = "Failed to load sensor information. Some units may not display correctly.";
    errorElement.style.color = 'red';
    document.body.prepend(errorElement);
    
    // Continue with dashboard updates even if metadata failed
    updateDashboard();
    setInterval(updateDashboard, 2000);
  }
}
// Start the initialization
initializeDashboard();

function getUnitsForSensor(sensor) {
  const units = [];
  
  const channelMatch = sensor.match(/(\d+)$/);
  const channel = channelMatch ? channelMatch[1] : null;
  sensor = sensor.replace(/_.*$/, ""); // Remove any suffix after underscore for matching
  
  if (sensorMetadata[sensor] && sensorMetadata[sensor].channels) {
      if (channel && sensorMetadata[sensor].channels[channel]) {
          const unit = sensorMetadata[sensor].channels[channel].unit;
          units.push(unit);
      } else {
          // If no channel or channel not found, push all units from all channels
          Object.values(sensorMetadata[sensor].channels).forEach(channelData => {
              const unit = channelData.unit;
              units.push(unit);
          });
      }
  }
  
  // If no units found in metadata, try to match by common patterns
  if (units.length === 0) {
    if (sensor.toLowerCase().includes('humidity')) units.push('%');
    if (sensor.toLowerCase().includes('temp') || sensor.toLowerCase().includes('temperature')) units.push('°C');
    if (sensor.toLowerCase().includes('light') || sensor.toLowerCase().includes('lux')) units.push('Lux');
    if (sensor.toLowerCase().includes('co2')) units.push('ppm');
    if (sensor.toLowerCase().includes('voc')) units.push('ppb');
  }
  
  return units.length > 0 ? units : [''];
}

function getColor(index) {
  const color = fallbackColors[index % fallbackColors.length];
  return {
    border: color,
    background: color.replace('1)', '0.2)')
  };
}

function createChart(canvas, label, colorConfig, units = []) {
  const datasets = units.map((unit, i) => ({
    label: `${label} ${units.length > 1 ? `(Channel ${i+1} ${unit})` : `(${unit})`}`,
    data: [],
    borderColor: colorConfig.border.replace(')', `${0.5 + (i * 0.3)})`), // Adjust opacity for multiple lines
    backgroundColor: colorConfig.background.replace(')', `${0.2 + (i * 0.2)})`),
    borderWidth: 1.5,
    pointRadius: 1,
    fill: false,
    tension: 0.1
  }));

  return new Chart(canvas, {
    type: 'line',
    data: {
      labels: [],
      datasets: datasets
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
        legend: { 
          display: units.length > 1, // Only show legend if multiple channels
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
  
  // Handle single value array or array of arrays (multiple channels)
  const isMultiChannel = Array.isArray(values[0]);
  
  if (isMultiChannel) {
    return values.map(channelValues => {
      if (channelValues.length === 0) return null;
      const min = Math.min(...channelValues);
      const max = Math.max(...channelValues);
      const avg = channelValues.reduce((sum, val) => sum + val, 0) / channelValues.length;
      return {
        min: min.toFixed(2),
        max: max.toFixed(2),
        avg: avg.toFixed(2)
      };
    });
  } else {
    if (values.length === 0) return null;
    const min = Math.min(...values);
    const max = Math.max(...values);
    const avg = values.reduce((sum, val) => sum + val, 0) / values.length;
    return [{
      min: min.toFixed(2),
      max: max.toFixed(2),
      avg: avg.toFixed(2)
    }];
  }
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

      // Get units for this sensor
      const units = getUnitsForSensor(sensor);
      
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

        const colorConfig = getColor(index);
        charts[sensor] = createChart(canvasId, sensor, colorConfig, units);
      }

      // Update chart data
      const chart = charts[sensor];
      let values = data[sensor].values || [];
      let times = data[sensor].times || [];

      // Check if this is a multi-channel sensor (values is array of arrays)
      const isMultiChannel = Array.isArray(values[0]);
      
      // Trim data to MAX_POINTS
      if (times.length > MAX_POINTS) {
        times = times.slice(-MAX_POINTS);
        if (isMultiChannel) {
          values = values.map(channel => channel.slice(-MAX_POINTS));
        } else {
          values = values.slice(-MAX_POINTS);
        }
      }

      // Update chart
      chart.data.labels = times;
      
      if (isMultiChannel) {
        // For multi-channel sensors, update each dataset
        values.forEach((channelValues, i) => {
          if (chart.data.datasets[i]) {
            chart.data.datasets[i].data = channelValues;
          }
        });
      } else {
        // For single channel sensors, update the first dataset
        if (chart.data.datasets[0]) {
          chart.data.datasets[0].data = values;
        }
      }
      
      chart.update();

      // Update card info
      const stats = calculateStats(values);
      
      if (isMultiChannel) {
        // For multi-channel sensors, show all channels' current values
        let currentValuesHtml = '';
        stats.forEach((stat, i) => {
          if (stat && values[i] && values[i].length > 0) {
            const currentValue = values[i][values[i].length - 1];
            currentValuesHtml += `<div>Channel ${i+1}: ${currentValue.toFixed(2)} ${units[i] || ''}</div>`;
          }
        });
        card.querySelector('.current-value').innerHTML = currentValuesHtml || 'No data';
      } else {
        // For single channel sensors
        const currentValue = values.length > 0 ? values[values.length - 1] : null;
        if (currentValue !== null) {
          card.querySelector('.current-value').textContent = `${currentValue.toFixed(2)} ${units[0] || ''}`;
        }
      }
      
      const lastTime = times.length > 0 ? times[times.length - 1] : '--:--:--';
      card.querySelector('.last-update').textContent = `Last update: ${lastTime}`;
      
      // Update stats
      if (stats) {
        if (isMultiChannel) {
          let statsHtml = '';
          stats.forEach((stat, i) => {
            if (stat) {
              statsHtml += `
                <div><strong>Channel ${i+1}:</strong></div>
                <div>Min: ${stat.min} ${units[i] || ''}</div>
                <div>Max: ${stat.max} ${units[i] || ''}</div>
                <div>Avg: ${stat.avg} ${units[i] || ''}</div>
                <br>
              `;
            }
          });
          card.querySelector('.stats-container').innerHTML = statsHtml;
        } else if (stats[0]) {
          card.querySelector('.min-value').textContent = `${stats[0].min} ${units[0] || ''}`;
          card.querySelector('.max-value').textContent = `${stats[0].max} ${units[0] || ''}`;
          card.querySelector('.avg-value').textContent = `${stats[0].avg} ${units[0] || ''}`;
        }
      }
    });
  } catch (err) {
    console.error("Dashboard update failed:", err);
  }
}

// Initial fetch of sensor metadata and then start the dashboard updates
fetchSensorMetadata().then(() => {
  // Initial and periodic update
  setInterval(updateDashboard, 2000);
  updateDashboard();
});