# SoftwarRenderer
Experiments in software rendering techniques

## Compiling
To compile, change directory to the src directory , enable the MSVC command line tools by calling shell.bat, then run build.bat. 

\src> ..\misc\shell.bat

\src> build.bat

## Controls
Use WASD to move up, left, down, and right. To move in and out, use up arrow and down arrow, respectively. There are three virtual cameras. To switch between them press 1, 2, or 3.

## Features
- Hot Reloading
  -   Edits to the simulation and rendering code can be made, then recompiled, all without needing to kill the main executable (see demo 002.gif).
  -   This does not work in all cases. For exmple, if a change to a struct is made then the app will need to be closed and recompiled.

- Virtual Camera
- Barycentric Coordinates
- Color Interpolation
    
## Resources

https://www.amazon.com/3D-Computer-Graphics-Mathematical-Introduction/dp/0521821037

https://my.eng.utah.edu/~cs5600/slides/

https://www.songho.ca/opengl/index.html
