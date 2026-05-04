# COMP3015-CW2

## Video Demonstration
YouTube link: https://youtu.be/H8lirmq60nk

## Project Overview
This project is an OpenGL-based graphics application developed for COMP3015. It builds upon the initial prototype from Coursework 1 and extends it into a more complete rendering pipeline using multiple GLSL shader techniques.

The aim of this project is to demonstrate advanced real-time graphics techniques within an interactive game-style environment, combining visual effects with simple gameplay elements.

## Features

The project implements the following shader techniques:

- Shadow Mapping (Depth-based lighting)
- Bloom Post-Processing (Gaussian Blur)
- Procedural Noise Texture
- Text Rendering (HUD instructions)

The application also includes basic game elements such as player movement, object interaction, and a simple objective.

## How to Run

1. Open the Visual Studio solution file (.sln)
2. Build the project using Release or Debug mode
3. Run the executable

Alternatively, the compiled executable is included in the project folder and can be run directly.

Controls:
- W / A / S / D: Move player
- Left Shift: Run
- Mouse Movement: Control camera direction
- E: interact 
- ESC: Exit

## System Architecture

The project follows the structure provided in the COMP3015 template.

- [main.cpp](https://github.com/JacksonHau/COMP3015-CW2/blob/main/COMP3015-CW2/COMP3015-CW2/main.cpp) – Entry point of the application
- [scenerunner.h](https://github.com/JacksonHau/COMP3015-CW2/blob/main/COMP3015-CW2/COMP3015-CW2/scenebasic_uniform.h) – Handles window creation and render loop
- [scenebasic_uniform.cpp](https://github.com/JacksonHau/COMP3015-CW2/blob/main/COMP3015-CW2/COMP3015-CW2/scenebasic_uniform.cpp) – Core scene logic including:
  - Initialisation
  - Shader setup
  - Object rendering
  - Framebuffer pipeline
  - Input handling

Shader files are separated by purpose:
- [basic_uniform.vert](https://github.com/JacksonHau/COMP3015-CW2/blob/main/COMP3015-CW2/COMP3015-CW2/shader/basic_uniform.vert)/[frag](https://github.com/JacksonHau/COMP3015-CW2/blob/main/COMP3015-CW2/COMP3015-CW2/shader/basic_uniform.frag) – Main object rendering
- [shadow_depth.vert](https://github.com/JacksonHau/COMP3015-CW2/blob/main/COMP3015-CW2/COMP3015-CW2/shader/shadow_depth.vert)/[frag](https://github.com/JacksonHau/COMP3015-CW2/blob/main/COMP3015-CW2/COMP3015-CW2/shader/shadow_depth.frag) – Shadow map generation
- [bloom_blur.frag](https://github.com/JacksonHau/COMP3015-CW2/blob/main/COMP3015-CW2/COMP3015-CW2/shader/bloom_blur.frag) – Gaussian blur pass
- [bloom_final.frag](https://github.com/JacksonHau/COMP3015-CW2/blob/main/COMP3015-CW2/COMP3015-CW2/shader/bloom_final.frag) – Final compositing
- [text.vert](https://github.com/JacksonHau/COMP3015-CW2/blob/main/COMP3015-CW2/COMP3015-CW2/shader/text.vert)/[frag](https://github.com/JacksonHau/COMP3015-CW2/blob/main/COMP3015-CW2/COMP3015-CW2/shader/text.frag) – On-screen text rendering

### Shadow Mapping

Shadow mapping is implemented using a depth framebuffer.

The scene is rendered from the light’s perspective to generate a depth map. During the main render pass, each fragment is compared against this depth map to determine whether it is in shadow.

This improves depth perception and prevents objects from appearing disconnected from the environment.

### Bloom (Image Processing)

Bloom is implemented using a multi-pass framebuffer pipeline.

Bright fragments are extracted from the scene, blurred using a Gaussian blur (ping-pong technique), and then combined with the original scene.

This enhances bright light sources and improves the overall visual quality of the scene.

### Procedural Noise

A procedural noise texture is generated using the `noisetex` class.

This allows variation in surface appearance without relying entirely on static textures, adding subtle realism to the scene.

## AI Usage Declaration

AI tools were used during the development of this project as a support mechanism for debugging, technical clarification, and documentation structuring.

[AI Declaration Form](https://github.com/JacksonHau/COMP3015-CW2/blob/main/AI%20declaration%20form/Student%20Declaration%20of%20AI%20Tool.pdf)

Specifically, AI assistance was used for:
- Debugging and understanding OpenGL-related issues  
- Clarifying shader, lighting, and rendering pipeline concepts  
- Structuring and refining the README documentation  
- Improving clarity and organisation of written explanations  

All code, shader implementation, and system integration were developed independently. AI suggestions were critically evaluated, tested, and adapted before use.

AI tools were used strictly as a support resource and not as a replacement for independent development.

## Assets used:
Grass: https://opengameart.org/content/seamless-grass-texture-ii

Wood: https://www.cadnav.com/textures/5517.html

Rock: https://skfb.ly/6twon

Mini Reactor: https://skfb.ly/oTWwW

Energy Cell: https://skfb.ly/oxKTx
