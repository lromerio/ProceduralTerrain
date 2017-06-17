# Procedural Terrain

![Water](/screenshot/Water_Screenshot.jpg)

## Description
An infinite procedural terrain, done in C++ with OpenGL.
Developed in the context of the course _CS-341 Introduction to Computer Graphics_ at École polytechinque fédérale de Lausanne (EPFL).

[Here](https://youtu.be/0pw7K84S4LY) you can find a short video demo of the project.

## Getting Started
You should have at least OpenGL 4.1 installed on your computer. Then all needed libraries are included in the `external` folder.

To build and run the project execute the following commands:

```
mkdir build
cd build
cmake ..
make
cd project
./project
```

## Features

### Terrain
- Infinite generation
- Ridged multifractal perlin noise
- Tessellation shaders for level of detail rendering
- Distance fog
### Texturing
- Distance and normal based blend
- Mimap textures
### Sky
- Cubemap texture
- Day and night cycle
### Water
- Normal mapping for wave simulation
- Sky and terrain reflection
- Simple distorsion

## Controls
The project is provided with a simple GUI done using the [imgui](https://github.com/ocornut/imgui) library. It allows to tweak parameters and changing camera mode between the six available, which are described here below.

### Custom
Use `W` `A` `S` `D` keys to move around the terrain, drag with the mouse to change the view.

### Flythrough
Use `W` and `S` to move forward and backwards, `A` `S` `Q` `E`to move rotate the camera respectively left, right, up and down.
When in flythrough mode all camera movements have an inertial behaviour.

### FPS
The commands are the same as for the _Flythrough_ camera, but in this camera mode you are always on the terrain level.

### Prerecorded Bezier
This camera mode follows a prerecorded Bezier path. You can increase/decrease the camera speed pressing `W`/`S`.

### Record Bezier
Move the camera as in _Custom_ mode, press `R` to record a Bezier control point.

### Bezier
After recording a Bezier path in the _Record Bezier_ mode, this camera follows it. Again you can increase/decrease the camera speed pressing `W`/`S`.

## License
MIT licensed, details in [LICENSE.md](/LICENSE.md)

## Authors
- Albergoni Tobia - [aTobyWanKenobi](https://github.com/aTobyWanKenobi)
- Invernizzi Hakim - [inverniz](https://github.com/inverniz)
- Romerio Lucio - [lromerio](https://github.com/lromerio)

## Other Screenshots
![Tessellation](/screenshot/Tessellation_Screenshot.jpg)
![Texture](/screenshot/Texture_Screenshot.jpg)
