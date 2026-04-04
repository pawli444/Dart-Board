# Simple OpenGL Mini Engine

A small C++ OpenGL project providing basic building blocks for a 3D scene: an FPS-style camera, texture loading, shader handling, and simple VAO/VBO abstractions. This repository contains minimal reusable classes so you can quickly render textured geometry and control a camera.

## Features
- FPS-style camera (W/A/S/D movement, vertical movement, mouse look while holding left mouse button)
- Texture loading via stb_image
- Shader loader/manager with compile and link error output
- Simple abstractions: VAO, VBO, Texture, Shader, Camera
- Easy to integrate example: drop your shaders and texture files and run

## Requirements
- C++ compiler with C++11 support (or newer)
- OpenGL 3.3+ (or appropriate for your environment)
- GLFW (windowing + input)
- GLAD or GLEW (OpenGL function loader)
- GLM (math)
- stb_image.h (image loading)
- CMake (recommended) or manual build tools

### Recommended development packages (Linux examples)
- libglfw3-dev
- libglm-dev
- GLAD (include as source or via package)
- stb_image.h (single header)

## Files overview
- Camera.h / Camera.cpp - FPS camera, input handling, view/projection upload to shader
- shaderClass.h / shaderClass.cpp - shader file loader, compiler, linker, Activate/Delete, setInt
- Texture.h / Texture.cpp - loads images, creates OpenGL textures, Bind/Unbind/Delete, texUnit
- VAO.h / VAO.cpp - create VAO and link VBO attribute pointers
- VBO.h / VBO.cpp - create VBO and upload vertex data
- main.cpp - typical entry point: init GLFW/GLAD, create objects, render loop
- shaders/ - vertex and fragment shader files (vertex.glsl, fragment.glsl)
- assets/ - textures and other resources

## Building with CMake

1. Place GLAD, GLM and stb_image in your include path or configure CMake to find them.

2. Example `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.10)
project(SimpleOpenGL)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS} /path/to/glad /path/to/glm /path/to/stb)

add_executable(app
    main.cpp
    Camera.cpp
    Texture.cpp
    VAO.cpp
    VBO.cpp
    shaderClass.cpp
)

target_link_libraries(app PRIVATE ${OPENGL_LIBRARIES} glfw)
```

3. Build:

```bash
mkdir build
cd build
cmake ..
make
```

## Manual build (g++)

```bash
g++ -std=c++11 main.cpp Camera.cpp Texture.cpp VAO.cpp VBO.cpp shaderClass.cpp -o app -lglfw -lGL -ldl -pthread
```

## Running

```bash
./app
```

Make sure shader files and textures are accessible via correct relative paths.

## Controls
- W - move forward  
- S - move backward  
- A - strafe left  
- D - strafe right  
- Space - move up  
- Left Ctrl - move down  
- Hold Left Mouse Button - capture cursor and rotate camera  
- Left Shift - toggle movement speed  

## Notes
- Shader compilation and linking errors are printed to the console.
- Texture loading uses stb_image.
- Camera captures the cursor only while holding the left mouse button.
- Movement is not frame-rate independent (no deltaTime).
