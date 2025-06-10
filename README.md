# LibGuardian - Smart Library Monitoring System

## 📚 Introduction

LibGuardian is an intelligent Wireless Sensor Network (WSN) designed to monitor noise levels and occupancy in library environments. The system provides real-time insights into library conditions, helping staff maintain optimal study environments and enabling students to find quieter spaces.

### Why LibGuardian?

Libraries are meant to be quiet, focused spaces for learning and research. However, managing noise levels and occupancy across different areas can be challenging for library staff. LibGuardian addresses this by:

- **Real-time Monitoring**: Continuous tracking of noise levels and people count
- **Data-Driven Insights**: Historical analysis to identify patterns and peak usage times
- **Staff Alerts**: Automated notifications when areas become too noisy or crowded
- **Student Benefits**: Live data access to help students find optimal study spots
- **Scalable Design**: Easily expandable to monitor multiple rooms and floors

### Key Features

- 🔊 **Noise Level Monitoring**: Real-time sound measurement with decibel conversion
- 👥 **Occupancy Tracking**: Smart entry/exit detection using ultrasonic sensors
- 📊 **Interactive Dashboard**: Beautiful Grafana visualizations with live data updates
- 🌐 **Wireless Communication**: Multiple protocols (Wi-Fi, MQTT, NullNet)
- 💾 **Data Storage**: MySQL database for historical analysis
- 🔄 **Real-time Processing**: Node-RED for seamless data flow management

## 🏗️ System Architecture

LibGuardian consists of several interconnected components:

### Hardware Components
- **Grove Sound Sensor**: Measures ambient noise levels
- **HC-SR04 Ultrasonic Sensors**: Detect people entering/exiting (2 sensors per door)
- **Arduino Board**: Main processing unit for sound sensor
- **ESP32-S2**: Wireless data transmission for proximity sensors
- **OpenMote**: Wireless bridge for Arduino sensor data
- **Zolertia Board**: Central receiver and data aggregator

### Software Stack
- **Node-RED**: Data flow orchestration and processing
- **MySQL**: Persistent data storage and analytics
- **HiveMQ**: MQTT broker for real-time communication
- **Grafana**: Interactive dashboards and data visualization

### Communication Flow
1. Sensors collect noise and proximity data
2. Data transmitted wirelessly via MQTT/NullNet protocols
3. Node-RED processes and stores data in MySQL
4. Grafana displays real-time dashboards
5. System calculates occupancy and noise trends

## 🚀 Deployment Guide

### Prerequisites

Before starting the deployment, ensure you have:

- **Hardware**: All sensor components and microcontrollers listed above
- **Software**: 
  - Node.js (v14+)
  - MySQL Server (v8.0+)
  - Arduino IDE
  - Grafana (v9.0+)
  - Git

### Step 1: Clone the Repository

```bash
git clone https://github.com/yourusername/LibGuardian.git
cd LibGuardian
```

### Step 2: Database Setup

1. **Install MySQL** (if not already installed):
   ```bash
   # Ubuntu/Debian
   sudo apt update
   sudo apt install mysql-server
   
   # macOS
   brew install mysql
   
   # Windows
   # Download from https://dev.mysql.com/downloads/mysql/
   ```

2. **Create the database**:
   ```bash
   mysql -u root -p
   ```
   ```sql
   CREATE DATABASE libguardian;
   USE libguardian;
   ```

3. **Import the schema**:
   ```bash
   mysql -u root -p libguardian < database/schema.sql
   ```

### Step 3: Hardware Programming

#### Arduino Sound Sensor
1. Open Arduino IDE
2. Load `noise-arduino-code/noise-arduino-code.ino`
3. Install required libraries:
   - Grove Sound Sensor library
4. Connect the Grove Sound Sensor to your Arduino
5. Upload the code to your Arduino board

#### ESP32 Proximity Sensor
1. In Arduino IDE, add ESP32 board support:
   - File → Preferences → Additional Board Manager URLs
   - Add: `https://dl.espressif.com/dl/package_esp32_index.json`
2. Install ESP32 boards via Board Manager
3. Load `proximity-esp32-code/proximity-esp32-code.ino`
4. Update WiFi credentials in the code:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```
5. Connect HC-SR04 sensor to ESP32
6. Upload the code

#### OpenMote Sender
1. Navigate to the openmote-sender directory:
   ```bash
   cd openmote-sender
   ```
2. Compile and flash:
   ```bash
   make TARGET=openmote-cc2538 sender
   make TARGET=openmote-cc2538 sender.upload
   ```

#### Zolertia Receiver
1. Navigate to the zolertia-receiver directory:
   ```bash
   cd zolertia-reciver
   ```
2. Compile and flash:
   ```bash
   make TARGET=zoul BOARD=firefly reciver
   make TARGET=zoul BOARD=firefly reciver.upload
   ```

### Step 4: MQTT Broker Setup

You can use HiveMQ Cloud (free tier) or set up your own broker:

#### Option A: HiveMQ Cloud (Recommended for testing)
1. Visit https://www.hivemq.com/mqtt-cloud-broker/
2. Create a free account
3. Note your broker URL and credentials

#### Option B: Local Mosquitto Broker
```bash
# Ubuntu/Debian
sudo apt install mosquitto mosquitto-clients

# macOS
brew install mosquitto

# Start the broker
mosquitto -v
```

### Step 5: Node-RED Setup

1. **Install Node-RED**:
   ```bash
   npm install -g node-red
   ```

2. **Install required nodes**:
   ```bash
   npm install -g node-red-node-mysql
   npm install -g node-red-contrib-mqtt-broker
   npm install -g node-red-node-serialport
   ```

3. **Start Node-RED**:
   ```bash
   node-red
   ```

4. **Import flows**:
   - Open http://localhost:1880
   - Go to Menu → Import
   - Copy and paste the content from `node-red/flows.json`
   - Click Import

5. **Configure MySQL connection**:
   - Double-click any MySQL node in the flow
   - Enter your database credentials:
     - Host: localhost
     - Port: 3306
     - Database: libguardian
     - Username: your_mysql_username
     - Password: your_mysql_password

6. **Configure MQTT nodes**:
   - Update MQTT broker settings in all MQTT nodes
   - Enter your HiveMQ or local broker credentials

7. **Deploy the flows**:
   - Click the red "Deploy" button in the top-right corner

### Step 6: Grafana Setup

1. **Install Grafana**:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install -y software-properties-common
   sudo add-apt-repository "deb https://packages.grafana.com/oss/deb stable main"
   wget -q -O - https://packages.grafana.com/gpg.key | sudo apt-key add -
   sudo apt-get update
   sudo apt-get install grafana
   
   # macOS
   brew install grafana
   
   # Start Grafana
   sudo systemctl start grafana-server  # Linux
   brew services start grafana          # macOS
   ```

2. **Access Grafana**:
   - Open http://localhost:3000
   - Default login: admin/admin
   - Change password when prompted

3. **Add MySQL data source**:
   - Go to Configuration → Data Sources
   - Click "Add data source"
   - Select MySQL
   - Configure connection:
     - Host: localhost:3306
     - Database: libguardian
     - User: your_mysql_username
     - Password: your_mysql_password
   - Test connection and save

4. **Import dashboard**:
   - Go to Create → Import
   - Copy the content from `grafana/dashboards.json`
   - Paste and import
   - Select your MySQL data source when prompted

### Step 7: Physical Installation

1. **Sensor Placement**:
   - Install sound sensor in a central location of each room
   - Mount two HC-SR04 sensors on door frames (one for entry, one for exit)
   - Ensure sensors have clear line of sight and proper power supply

2. **Power Setup**:
   - Connect power banks or USB power supplies to all devices
   - Ensure continuous power for 24/7 operation

3. **Network Configuration**:
   - Verify all wireless devices can connect to your network
   - Test MQTT connectivity from ESP32 devices

### Step 8: System Testing

1. **Test individual components**:
   ```bash
   # Check Node-RED logs
   # View in browser at http://localhost:1880
   
   # Check Grafana dashboard
   # View at http://localhost:3000
   
   # Verify database data
   mysql -u root -p libguardian -e "SELECT * FROM sensor_events ORDER BY timestamp DESC LIMIT 10;"
   ```

2. **Test sensor functionality**:
   - Walk through monitored doors to test occupancy detection
   - Generate noise to test sound level monitoring
   - Verify data appears in Grafana dashboard within 2-3 seconds

3. **Validate data accuracy**:
   - Compare sensor readings with manual measurements
   - Check timestamp accuracy
   - Verify occupancy counting logic

## 📊 Dashboard Usage

Once deployed, you can access your LibGuardian dashboard at `http://localhost:3000`. The dashboard provides:

### Real-time Panels
- **Current Occupancy**: Live count of people in each monitored room
- **Noise Level Gauge**: Current decibel readings with color-coded alerts
- **Historical Trends**: Time-series graphs showing patterns over time
- **Comparative Analysis**: Side-by-side comparison of multiple rooms

### Key Metrics
- People count with entry/exit tracking
- Noise levels in decibels (dB SPL)
- Peak usage times and quiet periods
- Historical data for trend analysis

## 🔧 Troubleshooting

### Common Issues

**Sensors not reporting data:**
- Check power connections
- Verify WiFi/network connectivity
- Review Node-RED debug messages
- Ensure MQTT broker is accessible

**Database connection errors:**
- Verify MySQL service is running
- Check database credentials in Node-RED
- Confirm firewall settings allow connections

**Grafana dashboard not updating:**
- Check data source configuration
- Verify MySQL queries in panel settings
- Review Grafana logs for errors

**Inaccurate occupancy counting:**
- Calibrate ultrasonic sensor positioning
- Adjust detection thresholds in code
- Verify sensor triggering sequence

### Debug Commands

```bash
# Check Node-RED status
sudo systemctl status node-red

# View MySQL logs
sudo tail -f /var/log/mysql/error.log

# Check Grafana logs
sudo tail -f /var/log/grafana/grafana.log

# Test MQTT connectivity
mosquitto_pub -h your-broker-host -t test/topic -m "test message"
mosquitto_sub -h your-broker-host -t libguardian/sensors/+
```

## 📈 System Monitoring and Maintenance

### Regular Maintenance Tasks
- Monitor sensor battery levels (if using battery power)
- Clean ultrasonic sensors for accurate readings
- Review and archive old database records
- Update dashboard queries as needed
- Backup database regularly

### Performance Optimization
- Index database tables for faster queries
- Adjust sensor reading intervals based on requirements
- Optimize Node-RED flows for better performance
- Monitor system resource usage

## 🤝 Contributing

We welcome contributions to LibGuardian! Please feel free to:
- Report bugs and issues
- Suggest new features
- Submit pull requests
- Improve documentation

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙋‍♂️ Support

For questions, issues, or support:
- Open an issue on GitHub
- Check the troubleshooting section above
- Review Node-RED and Grafana documentation
- Contact the development team

---

**Built with ❤️ for smarter library management**
