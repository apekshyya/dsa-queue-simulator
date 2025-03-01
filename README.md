# DSA Queue Simulator

##  Traffic Light Simulation using Queues
This project is a **traffic simulation** implemented in **C** using the **SDL2** library. It simulates a **four-way intersection** with vehicles moving in different directions, traffic lights, and queue-based vehicle management. The simulation is visualized in real-time using **SDL2's rendering capabilities**.

---
##  Table of Contents
- [Installation](#installation)
- [Dependencies](#dependencies)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Key Features](#key-features)
- [References for SDL2](#references-for-sdl2)
- [Demo](#demo)
- [Contributing](#contributing)
- [License](#license)

---
## Installations

### Prerequisites
Before running the simulation, ensure you have the following installed:
- A **C compiler** (e.g., `gcc` or `clang`)
- **SDL2** library and **SDL2 development headers**
- **Make** (for building the project)

### Steps to Install and Run
#### 1️⃣ Clone the Repository
```sh
$ git clone https://github.com/apekshyya/dsa-queue-simulator.git
$ cd dsa-queue-simulator
```

#### 2️⃣ Install Required Libraries
**For macOS (Homebrew):**
```sh
$ brew install sdl2 sdl2_ttf
```
**For Ubuntu/Debian:**
```sh
$ sudo apt-get install libsdl2-dev libsdl2-ttf-dev
```

#### 3️⃣ Compile and Run the Project
```sh
$ make          # Compile the project
$ make run      # Run both the generator and simulator
$ make clean    # Remove compiled files and data
```
To run them separately:
```sh
$ make run-generator  # Run the traffic generator
$ make run-simulator  # Run the traffic simulator
```

---
## Dependencies
- **SDL2**: Used for rendering the simulation and handling window management.
- **Standard C Libraries**: Used for I/O, memory management, and queue operations.
- **Make**: Automates the build process.

---
## Usage
The simulation runs in a **window** and displays **vehicles moving through a four-way intersection**. Traffic lights control the flow of vehicles, ensuring organized movement and realistic queuing behavior.

###  Controls
- **Close the Window**: Click the close button or press **ESC**.
- **Traffic Light Timing**: Traffic lights automatically switch every few seconds.

---
## 📂 Project Structure
```
dsa-queue-simulator/
├── simulator.c         # Main simulation program
├── queue.c             # Queue data structure implementation
├── traffic_generator.c # Traffic pattern generator
├── Makefile            # Build script
├── README.md           # Project documentation
├── LICENSE             # License information
│
├── assets/
│   └── ARIAL.TTF       # Font for SDL2 rendering
│
├── data/
│   ├── laneA.txt       # Traffic data for lane A
│   ├── laneB.txt       # Traffic data for lane B
│   ├── laneC.txt       # Traffic data for lane C
│   ├── laneD.txt       # Traffic data for lane D
```

---
## Key Features
### 1. Vehicle Management
- Vehicles are represented as structures with properties like **position**, **speed**, and **lane**.
- A **queue system** manages vehicles entering and leaving the simulation.

### 2. Traffic Light Control
- Traffic lights switch states automatically at fixed intervals.
- Vehicles **stop** at red lights and **move** on green signals.

### 3. SDL2-Based Rendering
- **Real-time traffic visualization** with SDL2.
- Roads, lanes, and traffic lights are drawn dynamically.

### 4. Lane Prioritization
- Some lanes have higher priority depending on traffic rules.
- Vehicles in **non-priority lanes** must wait longer if there is congestion.

---
## 5. References for SDL2
- [Official SDL2 Documentation](https://wiki.libsdl.org/)
- [SDL2 GitHub Repository](https://github.com/libsdl-org/SDL)

---
##  Demo


---

## License
This project is licensed under the **MIT License**. See the `LICENSE` file for details.

---



