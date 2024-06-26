# Tailored

A very basic and simple C/OpenGL game/app framework/engine being developed for **learning purposes**. (Work in progress) (Windows/macOS)  
\+ an example 3 screen setup for a simple character creator: Splash, Title, Game (Work in progress)  

**A lot of the code is incomplete or just temporary as im rushing to implement features and then improve and clean up as this is just a project I'm working on as I am learning and getting used to C and OpenGL.**

## Libraries
 - [GLFW](https://github.com/glfw/glfw): A multi-platform library for OpenGL, OpenGL ES, Vulkan, window and input.
 - [CGLM](https://github.com/recp/cglm): Highly Optimized 2D / 3D Graphics Math (glm) for C.
 - [CGLTF](https://github.com/jkuhlmann/cgltf): Single-file/stb-style C glTF loader and writer.
 - [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h): Image loading/decoding from file/memory: JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC.
 - [stb_truetype.h](https://github.com/nothings/stb/blob/master/stb_truetype.h): Parse, decode, and rasterize characters from truetype fonts.
 - [Miniaudio](https://github.com/mackron/miniaudio): Audio playback and capture library written in C, in a single source file.

## Features
 - Sprites: Rendering of textures with additional settings such as 9-slice scaling.
 - UI system: A base implementation of UI elements used throughout the examples.
 - Text: A basic font system implementation for text rendering using .ttf font files.
 - 3D models/scenes: Rendering of 3D models and scenes loaded from .gltf files. 

## Examples
Example UI featuring buttons, scrollable areas and a simple particle system.  
The example screen setup also features a very simple loading of assets on a different thread while displaying a loading bar. (and a fake loading bar between the title and game screen for an added effect)

Some assets used in the examples are from [https://kenney.nl/](https://kenney.nl/)  
Font used is [Eczar](https://fonts.google.com/specimen/Eczar)

https://github.com/georgiitonchev/tailored/assets/16121911/423b0212-5c20-407b-9a24-078f7ea5eaa5  

https://github.com/georgiitonchev/tailored/assets/16121911/a6de7976-8721-4bb6-9a53-d25453806578  

https://github.com/georgiitonchev/tailored/assets/16121911/ac70810f-ad23-4788-a6d5-023dd0b7980b  







