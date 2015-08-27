# Introduction #

This text is copied from the readme in the download.


bsp-renderer is an open source application for displaying levels stored in the quake3 bsp map format. It allows the user to move around in the levels using a first person camera and rudimentary collision detection. It is released under the GNU General Public License version 3 (http://www.gnu.org) The purpose of this document is to give an overview of the bsp-renderer application, including how to use it, which third party libraries and third party code it uses and other technical information. This distribution of the program contains the source code and prebuilt executables. It also contains the libraries necessary to build the source, and a level called "The Forlorn Hope", which can be found at http://www.users.on.net/justonefix/. bsp-renderer is currently in version 0.7 and is hosted at code.google.com/p/bsp-renderer


bsp-renderer is intended as a starting point for implementing first person shooters, or similar games. The reason I created it is that I wanted to porgram an FPS-like game from scratch (well almost anyway, the map format was created by Id Software). Of course, there is a long way to go before bsp-renderer can be called an FPS, hence the name. It's about 4000 lines of code, and the GPL notices at the beginning of each file account for 500 of these.


## How to run the application ##

The project contains two configurations, debug and release. The prebuilt executable is a release build, which has the flag RELEASE\_BUILD set. This flag adjusts were the application looks for the default map. If the application finds a bsp.cfg file in its running directory, it will parse this file to set which map to load. The bsp.cfg file in this distribution contains an explanation of the syntax. If no file is present, or no file or map is set in the file, the default map will be loaded (found in the 'media' folder). The application will exit if it cannot find the map or data file bsp.cfg specifies. Also, the runtime library zlib1.dll needs to be in the run directory, if it does not exist in the default Windows dll directories.


### Keys ###

Use WASD and mouse to move around.

F1: disable collision detection.

F2: enable collision detection without gravity

F3: enable collision detection with gravity

F8: toggle full screen mode

## Building bsp-renderer ##

You will need a DirectX SDK, probably version 9.0c, it is not tested with earlier versions. You will also need MS Visual 2008 (the free express version will do). To build bsp-renderer Visual studio needs to have the paths of the DirectX libraries and include files in the VC++ directories settings (click 'options' in the 'tools' menu). Zziplib.lib, the external library used by bsp-renderer is included in the distribution and will be linked automatically when building. The usage of hardware vertex processing can't be adjusted dynamically in the current state of bsp-renderer. Use the NO\_HW\_VERTEXPROCESSING preprocessor definition to toggle this. Originally, bsp-renderer always used HW vertex processing, but I had to work on a laptop which didn't support it, hence this option.

## Technical overview ##
The levels are rendered using the visibility algorithm briefly described here. For a more in depth description see reference (1) (where I received this information).

The BSP tree for the level is traversed using the camera position to find a cluster index. This index is used to test the potentially visible clusters from the current one. The cluster visibility information is in the map data. To find the visible faces we interate over the leafs in the BSP tree (the leafs contain face indices). First, the cluster of each leaf is tested against the camera cluster. If this test is positive, its bounding box is then culled against the camera's view frustum. If the leaf is not completely outside the frustum, the faces in the leaf are added to the list of faces to render while avoiding to add the same face twice. The list of faces are then sent to the drawing routines.

The sky is a cube mapped texture which was created using Terragen (http://www.planetside.co.uk/terragen/). The same sky is always used, not the ones defined in the .bsp files.

## Limitations ##

Quake III uses "shader files" for the moving and animated parts of the map, these files are not used by bsp-renderer in its present state.

The collision detection does not seem to work well for all curved surfaces.

The "physics" (i.e. the gravity) is only rudimentary and quite rough

Although it is quite fast, it could be faster. The drawing of the bezier patches is most likely not optimal. It seems they were designed for the OpenGL command glMultiDrawElementsEXT() In DirectX, I had to replace this with a loop and multiple calls to DrawIndexedPrimitive().

The check for the capabilities of the graphics card is a bit limited. The use of hardware vertex processing is not dynamic, meaning that if you run the executable using it on a graphics card where it is not supported, the program will exit.

See also http://code.google.com/p/bsp-renderer/issues/list

# References #

The information on following web sites were used during the development.

**(1) Rendering Quake 3 Maps**

Author: Morgan McGuire

Website: http://graphics.cs.brown.edu/games/quake/quake3.html

**(2) Unofficial Quake 3 Map Specs**

Author: Kekoa Proudfoot

Website: http://graphics.stanford.edu/~kekoa/q3

**(3) Quake 3 BSP Collision Detection**

Author: Nathan Ostgard

Website: http://www.devmaster.net/articles/quake3collision

For the DirectX specific parts the book Introduction to 3D Game Programming with DirectX 9.0c: A Shader Approach by Frank Luna (http://www.moon-labs.com/), and the DirectX SDK documentation were useful.

# Licensing #

bsp-renderer is released under GPL version 3. However, it relies on third party libraries, as well as pieces of source code from other open source programs. An overview of these libraries and source code is given here. The licenses mentioned can be found in the 'licenses' folder.

## Source code licenses ##

Two parts of the source code are based on algorithms originally from other open source applications.

- The source for the collision detection (but not the collision response) is based on code from reference (3) above, which in turn comes from the Quake III source.

- The tessellation of the Bezier patches is based on code from reference (1). The code originally comes from the Octagon Engine.

**Quake III**

Original Author: Id Software, Inc.

Website: http://www.idsoftware.com

Licensed Under: GNU General Public License, version 2

**Octagon Engine**

Original Author: Paul Baker

Website: http://www.paulsprojects.net/opengl/octagon/octagon.html

Licensed Under: New BSD License

## Third party libraries ##

**Zziplib**

Original Author: Guido Draheim

Website: http://zziplib.sourceforge.net

Licensed Under: Lesser GNU Lesser Public License (called the GNU Library Public License in the 'licenses' folder)

**Zlib**

Original Authors: Jean-loup Gailly and Mark Adler

Website: http://www.zlib.net

Licensed Under: Zlib License
Zlib is used as a runtime library by Zziplib and not by bsp-renderer directly.

## Media content ##

**The Forlorn Hope** (a Quake III level)

Original Author: JustOneFiX

Website: http://www.users.on.net/justonefix/






Copyright © 2008 Daniel Örstadius