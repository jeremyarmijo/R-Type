# Benchmark of 2D Graphical Libraries for R-TYPE Client

## Introduction

The goal of this bunchmark is to decide which of the 2D graphical are the most approprite for this project for the development of the R-TYPE client. The bunchMark foncuses on diffrent perfomances on ease of use, rendering capabilities, audio support, input handling, cross-platform compatibility.

This benchmark aims to help choose the most suitable library for building a **networked 2D shoot-em-up game**.

## Libraries Overview

### SFML (Simple and Fast Multimedia Library)

 SFML is a freely available and open-source software development library. Its purpose is to offer a straightforward and efficient interface to several elements of a personal computer, including graphics, audio, network, and input devices. It is implemented using the C++ programming language and provides developers with an object-oriented application programming interface (API).

 ### SDL2 (Simple DirectMedia Layer 2)

SDL (Simple DirectMedia Layer) is a cross-platform development library designed to provide low level access to audio, keyboard, mouse, joystick, and graphics hardware via OpenGL and Direct3D. SDL is written in C, works natively with C++, and there are bindings available for several other languages, including C# and Python.

 ### Raylib

Raylib is a cross-platform open-source software development library. The library was made to create graphical applications and games. The library is designed to be suited for prototyping, tooling, graphical applications, embedded systems, and education. The source code is written in the C programming language.

## Comparison Table
| Criterion | SFML | SDL2 | Raylib |
|-----------|------|------|--------|
| Performance / FPS | 5702 | 20 406 | 5400 |
| Ease of API | OOP, easy with C++ | C, basic | simple API, easy |
| Rendering 2D | very easy, sprites, shapes, text | low-level, more manual | very easy, shapes, textures |
| Audio | easy (sf::Sound, sf::Music) | SDL_mixer needed | simple API, basic support |
| Input / Keyboard / Joystick | easy, high-level | low-level, manual polling | easy, supports keyboard, mouse, gamepads |
| Cross-platform | Windows, Linux, macOS | Windows, Linux, macOS | Windows, Linux, macOS |
| License | zlib | zlib | zlib |
| Community / Documentation | large, active | large, active | medium, active |
| Ease of Build / CMake integration | easy | medium | easy |
| Additional Features | 2D graphics + networking, windowing, threads | low-level graphics, minimal extras | shapes, collisions, simple 3D support |


## Conclusion
For the R‑Type project, both SFML and SDL2 are suitable choices. SFML offers an easy to use,  C++ API that simplifies development and rapid prototyping, while SDL2 provides high performance, precise control, and greater flexibility for real-time gameplay. By combining the strengths of both libraries, the project can benefit from rapid development and efficient execution, making it ideal for a 2D shooter like R‑Type.