#include "GPUPrimitive.hpp"
#include <cstring>

namespace viewer
{
	int32_t GPUPrimitive_updateIndexBuffer(GPUPrimitive * primitive, int64_t bufferSize, const int32_t * indices)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive->indexVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
		int32_t * mappedIndices = (int32_t *) glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, bufferSize,
															   GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
		memcpy(mappedIndices, indices, bufferSize);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		return 1;
	}

	int32_t GPUPrimitive_updateBuffer(GPUPrimitive * primitive, int32_t indice, int64_t bufferSize, const float * data)
	{
		glBindBuffer(GL_ARRAY_BUFFER, primitive->vbo[indice]);
#if 1
		glBufferData(GL_ARRAY_BUFFER, bufferSize, data, GL_STATIC_DRAW);
#else
		glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
		float * mappedData = (float *) glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize,
														GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

		memcpy(mappedData, data, bufferSize);
		glUnmapBuffer(GL_ARRAY_BUFFER);
#endif
		return 1;
	}

	int32_t GPUPrimitive_setup(GPUPrimitive * primitive)
	{
		glGenBuffers(1, &(primitive->indexVBO));
		glGenBuffers(1, &(primitive->vertexVBO));
		glGenBuffers(1, &primitive->normalVBO);
		glGenBuffers(1, &primitive->texcoordVBO);
		glGenVertexArrays(1, &primitive->vertexArray);
		glBindVertexArray(primitive->vertexArray);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive->indexVBO);
		glBindBuffer(GL_ARRAY_BUFFER, primitive->vertexVBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, primitive->normalVBO);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*3, (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, primitive->texcoordVBO);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT)*2, (void*)0);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return 1;
	}

	int32_t GPUPrimitive_delete( GPUPrimitive * primitive)
	{
		glDeleteVertexArrays(1, &primitive->vertexArray);
		glDeleteBuffers(1, &primitive->indexVBO);
		glDeleteBuffers(1, &primitive->vertexVBO);
		glDeleteBuffers(1, &primitive->normalVBO);
		return 1;
	}

	int32_t GPUPrimitive_render(GPUPrimitive * primitive)
	{
		glBindVertexArray(primitive->vertexArray);
		glDrawElements(GL_TRIANGLES, primitive->indexCount*3, GL_UNSIGNED_INT, (void*)0);
		return 1;
	}

}; // namespace viewer
