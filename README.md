Systematic implementations of core computer-graphics concepts across three offline labs.

---

## 🧭 Contents

1. [Lab 1: OpenGL Basics (Offline1)](#lab-1-opengl-basics-offline1)  
2. [Lab 2: Rasterization Pipeline (Offline2)](#lab-2-rasterization-pipeline-offline2)  
3. [Lab 3: Ray Tracing (Offline3)](#lab-3-ray-tracing-offline3)  
4. [Setup & Build](#setup--build)

---

## Lab 1: OpenGL Basics (Offline1)

Basic GLUT demonstrations and interactive 3D exercises.

- **Bouncing Ball** (`2005018_Ball.cpp`)  
  – Simulate a sphere bouncing in a 3D box under gravity, with realistic collisions and camera controls.  
- **Analog Clock** (`2005018_Clock.cpp`)  
  – Draw a real-time, anti-aliased clock face with hour, minute, second hands; demonstrate 2D primitives and timer callbacks.  

---

## Lab 2: Rasterization Pipeline (Offline2)

Implement stages of the classic software rasterizer.

1. **Stage 1 – Coordinate Transformation & Clipping**  
   - Read scene configuration, apply model/view/projection transforms.  
   - Output transformed vertex coordinates (`stage1.txt`).  
2. **Stage 2 – Edge & Triangle Setup**  
   - Implement Bresenham’s line algorithm; set up triangle edge tables.  
   - Output edge lists (`stage2.txt`).  
3. **Stage 3 – Triangle Filling & Shading**  
   - Scanline rasterization, barycentric interpolation for color or depth.  
   - Output filled‐triangle spans (`stage3.txt`).  
4. **Z-Buffer (Hidden-Surface Removal)**  
   - Maintain per-pixel depth buffer to handle overlapping primitives.  
   - Output final z-buffer values (`z_buffer.txt`).  
5. **Image Output**  
   - Write rendered scene to BMP (`out.bmp`).  
6. **Test Cases**  
   - Validate each stage on four predefined scenes under `Test Cases/`.  

---

## Lab 3: Ray Tracing (Offline3)

Build a basic ray tracer with shading, shadows, and reflections.

- **Scene Parsing** (`scene.txt`, `scene_test.txt`)  
  – Read recursion depth, image size, object/light lists.  
- **Object Hierarchy** (`2005018_classes.hpp/.cpp`)  
  – Abstract `Object` base plus `Sphere`, `Triangle`, `General` quadric.  
- **Ray‐Object Intersection**  
  – Implement geometric tests for each primitive, return hit distance.  
- **Lighting & Shading**  
  - Ambient, diffuse (Lambert), specular (Phong) via point lights (`PointLight`) and spotlights (`SpotLight`).  
  - Shadow rays to determine occlusion.  
- **Reflection & Recursion**  
  – Cast reflection rays up to user‐defined recursion limit.  
- **Textured Floor** (`Floor` class, `bitmap_image.hpp`, `stb_image.h`)  
  – Checkerboard vs. image‐mapped tiling.  
- **Capture & Controls** (`2005018_main.cpp`, `commands.txt`)  
  – Keyboard to capture & save BMP, change floor textures, toggle checkerboard.  

---

## Setup & Build

_All labs require a GLUT/OpenGL development environment._

```bash

# (On linux for setup) :
sudo apt update
sudo apt install build-essential freeglut3-dev libglu1-mesa-dev libglew-dev 

# Lab 1 examples: (On windows)
g++ 2005018_Ball.cpp -o main -lopengl32 -lglu32 -lfreeglut

# On linux :
g++ 2005018_Ball.cpp -o demo -lglut -lGLU -lGL

# Lab 2 rasterizer:
g++ Offline2/image_drawing.cpp Offline2/Parser.h Offline2/Matrix.h -o Rasterizer

# Lab 3 ray tracer:
g++ Offline3/2005018_main.cpp Offline3/2005018_classes.cpp Offline3/vector.hpp -o RayTracer -lglut -lGLU -lGL
