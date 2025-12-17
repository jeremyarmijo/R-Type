# Contributing to R-Type

Thank you for your interest in contributing to **R-Type**! This guide will help you get started with the project structure, coding standards, and contribution workflow.

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Project Structure](#project-structure)
3. [Development Setup](#development-setup)
4. [Coding Standards](#coding-standards)
5. [Branching Strategy](#branching-strategy)
6. [Commit Guidelines](#commit-guidelines)
7. [Pull Request Process](#pull-request-process)
8. [Testing Guidelines](#testing-guidelines)
9. [Documentation](#documentation)
10. [Community Guidelines](#community-guidelines)

---

## Getting Started

### Prerequisites

Before contributing, ensure you have:
- **C++17** compiler (GCC 14+ or Clang 15+)
- **CMake** 3.30+
- **Conan** 2.0+
- **Git** for version control
- Basic understanding of ECS architecture and network programming

### First-Time Setup

1. **Fork the repository** on GitHub
2. **Clone your fork**:
   ```bash
   git clone https://github.com/YOUR_USERNAME/R-Type.git
   cd R-Type
   ```

3. **Add upstream remote**:
   ```bash
   git remote add upstream https://github.com/jeremyarmijo/R-Type.git
   ```

4. **Install dependencies**:
   ```bash
   ./install_dependencies_client.sh
   ./install_dependencies_server.sh
   ```

5. **Build the project**:
   ```bash
   # Client
   cd build
   cmake -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake ..
   make
   
   # Server
   cd Server/build
   cmake -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake ..
   make
   ```

---

## Project Structure

```
R-Type/
├── Client/                 # Client-side code
│   ├── src/
│   │   ├── Main.cpp       # Entry point
│   │   ├── engine/        # Game engine (ECS, rendering, input)
│   │   ├── scenes/        # Game scenes (menu, lobby, game)
│   │   └── ui/            # UI components
│   └── assets/            # Sprites, sounds, fonts
│
├── Server/                # Server-side code
│   ├── src/
│   │   ├── main.cpp       # Server entry point
│   │   ├── ServerGame.cpp # Main game logic
│   │   └── network/       # Network implementation
│   └── include/           # Server headers
│       ├── ServerGame.hpp
│       └── network/       # Network headers
│
├── Shared/                # Shared code (ECS, protocol)
│   ├── components/        # ECS components
│   ├── systems/           # ECS systems
│   └── ecs/               # ECS registry
│
├── documentation/         # Project documentation
│   ├── SERVER_TECHNICAL_DOC.md
│   ├── NetworkClient.md
│   └── ASIO_IMPLEMENTATION.md
│
├── CMakeLists.txt         # Root CMake configuration
├── conanfile.txt          # Conan dependencies
└── README.md              # Project overview
```

---

## Development Setup

### Building for Development

#### Client
```bash
cd R-Type
mkdir -p build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

#### Server
```bash
cd Server
mkdir -p build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

---

## Coding Standards

### C++ Style Guide - Google lint

We follow a **modern C++17** approach with the following guidelines:

#### Naming Conventions
- **Classes/Structs**: `PascalCase` (e.g., `ServerGame`, `TCPServer`)
- **Functions/Methods**: `PascalCase` (e.g., `Initialize`, `SendPacket`)
- **Variables**: `snake_case` (e.g., `client_id`, `tcp_port`)
- **Private members**: `snake_case_` with trailing underscore (e.g., `socket_`, `is_authenticated_`)
- **Constants**: `UPPER_CASE` (e.g., `MAX_CLIENTS`, `HEADER_SIZE`)

see all details in https://google.github.io/styleguide/cppguide.html

#### Code Formatting
```cpp
// Good
class MyClass {
 public:
  void MyMethod(int param);
  
 private:
  int my_variable_;
};

// Function formatting
void MyFunction(int param1, std::string param2) {
  if (condition) {
    // Code here
  }
}
```

#### Best Practices
- **Use `const`** wherever possible
- **Prefer `std::unique_ptr` and `std::shared_ptr`** over raw pointers
- **Avoid `using namespace std`** in headers
- **Use RAII** for resource management
- **Document public APIs** with Doxygen comments

### Doxygen Documentation

All public classes and methods must have Doxygen comments:

```cpp
/**
 * @class ServerGame
 * @brief Main server game logic controller
 * 
 * Manages the game loop, ECS registry, and network communication.
 */
class ServerGame {
 public:
  /**
   * @brief Initialize the server
   * @param tcpPort TCP port for client connections
   * @param udpPort UDP port for game data
   * @param diff Game difficulty level
   * @param host Host IP address (default: "0.0.0.0")
   * @return true if initialization succeeded
   */
  bool Initialize(uint16_t tcpPort, uint16_t udpPort, int diff, 
                  const std::string& host = "0.0.0.0");
};
```

---

## Branching Strategy

We use **GitFlow** with the following branches:

### Main Branches
- **`main`**: Stable production releases
- **`dev`**: Development integration branch

### Supporting Branches
Generate branch with issue

### Workflow
```bash
# Create a feature branch
git checkout dev
git pull upstream dev
git checkout -b ID_ISSUE-title-issue

# Make changes and commit
git add .
git commit -m "feat(SCOPE): add new enemy type"
# scopes are in .commitlint.js

# Push to your fork
git push origin ID_ISSUE-title-issue

# Open a Pull Request on GitHub
```

---

## Commit Guidelines

We use **Conventional Commits** for clear commit history:

### Format
```
<type>(<scope>): <subject>

```

### Types
- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes
- **style**: Code style (formatting, no logic change)
- **refactor**: Code refactoring
- **test**: Adding or updating tests
- **chore**: Maintenance tasks

### Examples
```bash
# Good commits
feat(gamedesign): add UDP heartbeat mechanism
fix(network): resolve sprite flickering issue
docs(jobsystem): update build instructions
refactor(engine): simplify component registration

# Bad commits (avoid these)
git commit -m "fixed stuff"
git commit -m "WIP"
git commit -m "changes"
```

### Commit Message Rules
- Use **imperative mood** ("add" not "added" or "adds")
- Keep subject line **under 72 characters**
- Separate subject from body with blank line
- Reference issues when applicable: `closes #42`

---

## Pull Request Process

### Before Submitting

1. **Update your branch** with latest `dev`:
   ```bash
   git checkout dev
   git pull upstream dev
   git checkout my-feature
   git rebase dev
   ```

2. **Build and test** locally:
   ```bash
   cmake --build build
   ./build/r-type_client
   ./Server/build/r-type_server
   ```

3. **Check for warnings**:
   ```bash
   cmake --build build 2>&1 | grep -i warning
   ```

4. **Update documentation** if needed

### Creating a Pull Request

1. **Push to your fork**:
   ```bash
   git push origin my-feature
   ```

2. **Open PR on GitHub** targeting `dev` branch

3. **Fill out the PR template**:
   ```markdown
   ## Description
   Brief description of changes
   
   ## Type of Change
   - [ ] Bug fix
   - [ ] New feature
   - [ ] Breaking change
   - [ ] Documentation update
   
   ## Testing
   - [ ] Manual testing performed
   - [ ] Builds successfully
   - [ ] No new warnings
   
   ## Related Issues
   Closes #42
   ```

### PR Review Process

- **At least 1 approval** required before merge
- **All CI checks** must pass
- Address **review comments** promptly
- **Squash commits** if requested
- Maintainers will merge when ready

---


## Documentation

### When to Update Docs

- **Class changes**: Update Doxygen comments
- **New features**: Add to README and relevant docs
- **Protocol changes**: Update `protocole.md`

### Building Doxygen

Doxygen automated deploy when your code is in main branch

### Documentation Locations

- **README.md**: Project overview, quick start
- **documentation/SERVER_TECHNICAL_DOC.md**: Server architecture
- **documentation/CONTRIBUTING.md**: This file
- **Doxygen**: https://jeremyarmijo.github.io/R-Type/

---

## Community Guidelines

### Code of Conduct

- Be **respectful** and **inclusive**
- Provide **constructive feedback**
- Help others learn and grow
- Report unacceptable behavior to maintainers

### Communication Channels

- **GitHub Issues**: Bug reports, feature requests
- **Pull Requests**: Code contributions, discussions
- **Discussions**: General questions, ideas

### Getting Help

- **First**, check existing documentation
- **Search** closed issues and PRs
- **Ask** in GitHub Discussions
- **Provide context** when asking for help

---

## Common Contribution Areas

### Easy First Issues

Good for newcomers:
- Documentation improvements
- Code comments and Doxygen
- Bug fixes with clear reproduction steps
- Test coverage expansion

### Medium Difficulty

- New enemy types
- UI improvements
- Sound effects integration
- Performance optimizations

### Advanced

- Network protocol enhancements
- ECS architecture improvements
- Cross-platform support (Windows)
- Replay system implementation

---
