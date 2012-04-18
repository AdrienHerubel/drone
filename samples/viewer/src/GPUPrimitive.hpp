#ifndef __VIEWER_GPU_PRIMITIVE_HPP__
#define __VIEWER_GPU_PRIMITIVE_HPP__

#include "GLCommon.hpp"

namespace viewer
{
	struct GPUPrimitive
	{
		GLuint vertexArray;
		GLuint indexVBO;
		uint64_t indexCount;
		union
		{
			struct
			{
				GLuint vertexVBO;
				GLuint normalVBO;
				GLuint texcoordVBO;
			};
			GLuint vbo[3];
		};
	};
	int32_t GPUPrimitive_setup(GPUPrimitive * primitive);
	int32_t GPUPrimitive_delete( GPUPrimitive * primitive);
	int32_t GPUPrimitive_updateIndexBuffer(GPUPrimitive * primitive, int64_t bufferSize, const int32_t * indices);
	int32_t GPUPrimitive_updateBuffer(GPUPrimitive * primitive, int32_t indice, int64_t bufferSize, const float * data);
	int32_t GPUPrimitive_render(GPUPrimitive * primitive);

}; // namespace viewer

#endif // __VIEWER_GPU_PRIMITIVE_HPP__
