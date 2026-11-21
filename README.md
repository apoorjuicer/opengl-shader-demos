# OpenGL Shader Demos

Nine interactive real-time rendering demos in GLSL and C++ (OpenGL 4.3),
built over a trimester of assignments for CSD2150 (Graphics) at DigiPen
Institute of Technology Singapore, on an instructor-provided framework.

## What's in it
- **Ray casting and ray tracing** in shaders (including a compute-shader ray caster)
- **Blinn-Phong lighting** with orbiting camera and lights
- **Environment mapping**: skyboxes, HDR cube maps, reflection and refraction
- **Night sky scene** with PBR-style shading, cartoon mode, fog and discard
- **Multi-pass cartoon pipeline**: cel shading, Gaussian blur and edge detection,
  with runtime toggles for blur, edges and threshold
- **Portal**: projected-texture portal with a procedural animated surface
- **Deferred shading**: G-buffer pass and normal mapping on an ogre model,
  with views of the G-buffer, light direction and view direction

## My part
- Wrote the GLSL shaders for all the demos (`Shaders/Shaders/`)
- Wrote the scene and render-pass setup and controls for each demo in `main.cpp`

## Credits
- Rendering framework (window manager, camera, FBO, program, texture, mesh
  and other C++ classes) by **Prof. Vadim Surov**, DigiPen Institute of
  Technology. Used with his permission; his file headers are kept as-is.
- `Portal.frag.glsl` adapts ideas from Shadertoy
  [4l2cDm](https://www.shadertoy.com/view/4l2cDm) and
  [ll2GD3](https://www.shadertoy.com/view/ll2GD3).
- `stb_image` by Sean Barrett.
- Models, textures and cube maps were provided with the course framework.

## Build
Visual Studio solution (`Shaders.sln`, x64). Needs GLFW, GLEW and GLM;
library paths are set in `Shaders/Shaders.vcxproj`.
