# Golf Phase Space Simulator

A golf simulator built with reactphysics3d that generates phase space diagrams based on which angles and launch velocities successfully land balls in the hole. Uses OpenGL to render the golf course and Dear ImGui with ImPlot for the user interface. Only runs on Windows.

Built on top of Cherno's OpenGL-Core library that uses premake for generating project files.

## Generating Visual Studio 2019 Project on Windows

Run `scripts/Win-Premake.bat` to generate the `Golf-Sim.sln` Visual Studio 2019 file.

## Dependencies

### Comes with Cherno's OpenGL-Core

A `*` next to the version number means it was updated from Cherno's OpenGL-Core library
| Library Name                   | Version | Purpose                                                          |
|--------------------------------|---------|------------------------------------------------------------------|
| Glad                           | 0.1.28  | Loading OpenGL Core 4.6 functions                                |
| GLFW                           | 3.4     | Creating windows, reading input, handling events, etc.           |
| GLM                            | 0.9.9   | Doing math with matrices and vectors in a format similar to GLSL |
| Dear ImGui                     | 1.84*   | GUI for adjusting settings and displaying info                   |
| spdlog                         | 1.5.0   | Logging                                                          |
| stb_image                      | 2.23    | Image loader                                                     |

### Other Libraries

| Library Name                   | Version | Purpose                                                          |
|--------------------------------|---------|------------------------------------------------------------------|
| reactphysics3d                 | 0.8.0   | Handling 3d physics collisions                                   |
| ImPlot                         | 0.12    | Addon to Dear ImGui that adds plotting functionality             |