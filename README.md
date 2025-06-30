# Cluster Computing with ESP32 and MQTT

## 📌 Overview

This project demonstrates a distributed computing cluster using **ESP32 microcontrollers** communicating via **MQTT**, controlled by a Python-based master node. The system is designed for parallel task execution, making it ideal for IoT-based edge computing applications.

---

## 🛠️ Hardware & Software Stack

### **ESP32 Cluster Nodes** (`cluster-computing-esp/`)

- **Communication**: MQTT (Pub/Sub) over Wi-Fi
- **Key Components**:
  - `mqtt/`: Handles MQTT broker connection, message publishing/subscribing
  - `wifi/`: Manages Wi-Fi connectivity
  - `main.c`: Core logic for task distribution & execution
- **Build System**: ESP-IDF (CMake-based)

### **Master Controller** (`cluster-computing-master/`)

- **Python Scripts**:
  - `nodes.py`: Manages cluster nodes (task assignment, monitoring)
  - `publisher.py`: Sends tasks via MQTT

---

## 🚀 Features

✔ **Distributed Task Processing** – Parallel computation across ESP32 nodes  
✔ **Dynamic Node Management** – Auto-detection of active nodes  
✔ **Fault Tolerance** – Reassigns tasks if a node disconnects  
✔ **Lightweight MQTT Protocol** – Efficient communication for low-power devices

---

## 🔧 Setup & Usage

### **1. Flash ESP32 Nodes**

```bash
cd cluster-computing-esp
idf.py build flash monitor
```

(Configure Wi-Fi & MQTT broker in `main.c`)

### **2. Run the Master Node**

```bash
cd cluster-computing-master
python3 publisher.py  # Send tasks
python3 nodes.py      # Monitor cluster
```

---

## 🤖 Potential Use Cases

- **Edge AI Inference** – Distribute ML tasks across nodes
- **Sensor Data Aggregation** – Parallel processing for IoT networks
- **Robotics Swarms** – Coordinated control of multiple agents

---

## 🔗 Dependencies

- **ESP-IDF** (for ESP32 firmware)
- **Paho-MQTT** (Python & C libraries)

---

**🌟 Star the repo if you find this useful!**  
[GitHub Link](https://github.com/Gurupranav-tech/Cluster-Computing)
