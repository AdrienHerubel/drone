drone
================================

Building
---------------------------------
### Linux
* Dependencies
To build the samples you only need a standard glut install, and up-to-date OpenGL 3.X or better drivers.

* Compiling in release mode
```bash
cd drone
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS_RELEASE="-Wall -O2"  -DCMAKE_C_FLAGS_RELEASE="-Wall -O2" ..
```
* Compiling in debug mode
```bash
cd drone
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_RELEASE="-Wall -ggdb"  -DCMAKE_C_FLAGS_RELEASE="-Wall -ggdb" ..
```

* Compiling Maya plugin
Add "-DWITH_MAYA=True" to cmake command line. Verify that maya is available at /usr/autodesk/maya/

### Windows

### MacOSX

Running
---------------------------------
### Linux
* samples/viewer
The viewer take both cache files and directories as command line arguments. Directories are scanned for ".drn" files. Use the 'alt' key and the 3 mouse buttons to navigate in the scene. Press '0' to switch to the stored camera. Press 'space' 'home' and arrows to navigate time.
```bash
viewer <cache> <cache_dir>
```

* tools/drnread
```bash
drnread -hlv <cache> 
```

* tools/drnmaya
Add the build path to $MAYA_PLUG_IN_PATH. Then load the plugin in Window->SettingsPreferences->Plugin-Manager or in the Script Editor
```mel
loadPlugin drnmaya
```