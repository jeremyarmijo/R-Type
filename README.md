# **R-Type: A Networked Shoot'em Up Game Engine**

---

## **Description**
**R-Type** is a **multiplayer networked horizontal shoot'em up** game, inspired by the 1990s classic. This project implements a **modular game engine**, a **multithreaded server**, and a **graphical client**, with a focus on real-time network synchronization and reusable architecture.

**Key Features**:
- **Custom game engine** (ECS architecture, decoupled systems).
- **Binary network protocol** (UDP and TCP, optimized for real-time gameplay).
- **Game design tools** for easy content creation.

---

## **Prerequisites**
### **System Requirements**
- **OS**: Linux, MacOS.
- **Compiler**: GCC 14+.
- **Tools**: CMake 3.30+, Git, Conan 2.0+.

### **Dependencies**
*(Automatically managed via Conan – see [conanfile.txt](conanfile.txt))*
- **Core**: C++20, SDL2 `sdl/2.28.5`.
- **Networking**: ASIO (for UDP/TCP).
- **Audio**: SDL_mixer or SFML Audio.

---

## **Build Instructions**
### **1. Clone the Repository**
```bash
git clone https://github.com/YOUR_USERNAME/R-Type.git
cd R-Type
```

**2. Install Dependencies**:
```bash
./install_dependencies_client.sh
./install_dependencies_server.sh
```

### **3. Configure with CMake**
```bash
mkdir build && cd build
cmake ..
```

### **4. Build the Project**
```bash
make
```

### **5. Run the Game**
#### **Server**
```bash
./r-type_server [PORT] [HOST]
# Example:
./r-type_server 4242 192.x.x.x
```

#### **Client**
```bash
./r-type_client
# Example:
./r-type_client
```

---

## **Usage Instructions**
### **Keyboard Controls (Client)**
| Key          | Action                     |
|---------------|----------------------------|
| **↑/↓/←/→**  | Move spaceship             |
| **Space**     | Fire                       |
| **Esc**       | Quit                       |

### **Gameplay**
- **Objective**: Destroy enemies (Bydos) and survive waves.
- **Elements**:
  - **Starfield**: Scrolling background.
  - **Spaceship**: Up to 4 players, distinct colors.
  - **Enemies**: Random spawn on the right, varied movement patterns.


<p align="center">
   <img src="documentation/loby.png" alt="Lobby 1" width="400"/>
   <img src="documentation/loby2.png" alt="Lobby 2" width="400"/>
</p>


---

## **Authors**
- **Team**:
  - [jeremy](https://github.com/jeremyarmijo/)
  - [jeremy](https://github.com/jeremyarmijo/)
  - [jeremy](https://github.com/jeremyarmijo/)
  - [jeremy](https://github.com/jeremyarmijo/)
  - [jeremy](https://github.com/jeremyarmijo/)

---

## **Technical Documentation**

### **Core Documentation**
- **[Server Technical Documentation](documentation/SERVER_TECHNICAL_DOC.md)** - Complete server architecture, networking.
- **[Doxygen](https://jeremyarmijo.github.io/R-Type/)** - Detailed class and method documentation
- **[Contributing Guide](documentation/CONTRIBUTING.md)** - How to contribute to the project

### **Network Documentation**
- **[Network Protocol](documentation/NetworkClient.md)** - Client-server communication protocol
- **[ASIO Implementation](documentation/ASIO_IMPLEMENTATION.md)** - Network library usage guide

### **Quick Links**
- [Build Instructions](#build-instructions)
- [Usage Instructions](#usage-instructions)
- [How to Contribute](documentation/CONTRIBUTING.md)
- [Full class Documentation](https://jeremyarmijo.github.io/R-Type/)
