cmake_minimum_required (VERSION 2.6)
project(drone)
set (drone_VERSION_MAJOR 1)
set (drone_VERSION_MINOR 0)

#
# drone lib file set
#
set(drone_lib_files lib/drone/src/drone/
	lib/drone/src/drone/drn_reader.c
	lib/drone/src/drone/drn_types.h
	lib/drone/src/drone/drn_writer.h
	lib/drone/src/drone/drn_writer.c
	lib/drone/src/drone/drn_hash.c
	lib/drone/src/drone/drn_hash.h
	lib/drone/src/drone/drn_reader.h
)

#
# drone_scene lib file set
#
set(drone_scene_lib_files lib/drone_scene/src/drone_scene/
	lib/drone_scene/src/drone_scene/drone_scene.hpp
	lib/drone_scene/src/drone_scene/drone_scene.cpp
 )

#
# stbi lib file set
#
set(stbi_lib_files lib/stbi/
	lib/stbi/
	lib/stbi/stb.h
	lib/stbi/stb_image.c
	lib/stbi/stb_image_write.c
	lib/stbi/stb_image_write.h
	lib/stbi/stb_image.h
)

include_directories(lib/drone/src/)
include_directories(lib/)
include_directories(lib/drone_scene/src/)
include_directories(/usr/autodesk/maya/include/)
link_directories(/usr/autodesk/maya/lib/)

set(EXECUTABLE_OUTPUT_PATH bin)

#
# viewer
#
add_executable(viewer ${drone_lib_files} ${drone_scene_lib_files} ${stbi_lib_files}
	samples/viewer/src/main.cpp
	samples/viewer/src/HighResolutionTimer.hpp
	samples/viewer/src/Transform.hpp
	samples/viewer/src/GLUtils.hpp
	samples/viewer/src/GLUtils.cpp
	samples/viewer/src/Camera.hpp
	samples/viewer/src/LinearAlgebra.hpp
	samples/viewer/src/GLCommon.hpp
	samples/viewer/src/Transform.cpp
	samples/viewer/src/Camera.cpp
	samples/viewer/src/LinearAlgebra.cpp
	samples/viewer/src/glew/glew.c
	samples/viewer/src/glew/glew.h
	samples/viewer/src/glew/glxew.h
	samples/viewer/src/GPUPrimitive.hpp
	samples/viewer/src/GPUPrimitive.cpp
	samples/viewer/src/ShaderGLSL.hpp
	samples/viewer/src/ShaderGLSL.cpp
)
target_link_libraries(viewer rt glut GLU GL)

#
# drnread
#
add_executable(drnread ${drone_lib_files} ${drone_scene_lib_files} tools/drnread/drnread.cpp tools/drnread/getopt.c tools/drnread/getopt_long.c)
add_definitions( -D_BOOL -DUNAME -DREQUIRE_IOSTREAM )

#
# drnmaya
#
if(WITH_MAYA)
	set(CMAKE_SHARED_MODULE_PREFIX "")
	add_library(drnmaya MODULE ${drone_lib_files} ${drone_scene_lib_files} ${stbi_lib_files}
		tools/drnmaya/src/Utils.cpp
		tools/drnmaya/src/CameraExport.hpp
		tools/drnmaya/src/SpotlightExport.hpp
		tools/drnmaya/src/drnMayaPlugin.hpp
		tools/drnmaya/src/drnTranslator.cpp
		tools/drnmaya/src/SpotlightExport.cpp
		tools/drnmaya/src/Utils.hpp
		tools/drnmaya/src/CameraExport.cpp
		tools/drnmaya/src/drnMayaPlugin.cpp
		tools/drnmaya/src/MeshExport.hpp
		tools/drnmaya/src/MeshExport.cpp
		tools/drnmaya/src/drnTranslator.hpp
	)
	target_link_libraries(drnmaya OpenMaya Foundation OpenMayaUI  rt glut GLU GL)
endif(WITH_MAYA)