async function addSensor(event) {
    event.preventDefault();
  
    const sensorType = document.getElementById("sensor_type").value;
    const sensorName = document.getElementById("sensor_name").value;
    const address = document.getElementById("address").value;
  
    const payload = {
      sensor_type: sensorType,
      sensor_name: sensorName,
      address: address
    };
  
    try {
      const response = await fetch('/add_sensor', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
      });
  
      const result = await response.json();
      document.getElementById("result").innerText = JSON.stringify(result, null, 2);
      loadSensors(); // Refresh list after adding
    } catch (err) {
      document.getElementById("result").innerText = "Error: " + err;
    }
  }
  
  async function loadSensors() {
    try {
      const response = await fetch('/active_sensors');
      const sensors = await response.json();
  
      const tbody = document.getElementById("sensorTable").querySelector("tbody");
      tbody.innerHTML = ""; // Clear old rows
  
      sensors.forEach(sensor => {
        const row = document.createElement("tr");
        row.innerHTML = `
          <td>${sensor.name}</td>
          <td>${sensor.type}</td>
          <td>${sensor.address}</td>
        `;
        tbody.appendChild(row);
      });
    } catch (err) {
      const tbody = document.getElementById("sensorTable").querySelector("tbody");
      tbody.innerHTML = `<tr><td colspan="3">Failed to load sensors</td></tr>`;
    }
  }
  
  // Initial load
  loadSensors();
  