#ifndef __VIEWER_GL_UTILS_HPP__
#define __VIEWER_GL_UTILS_HPP__

namespace viewer
{
	void drawAxes(const float * view);
	void drawSpot(const float wpos[4], const float wdir[4]);
	void drawAxis();
	void saveTexturePPM(const char * fileName, unsigned int textureIdGPU);
	void saveFrameBufferPPM(const char * fileName, unsigned int width, unsigned int height);
	void RGBA8ToRGB(const unsigned char * source, unsigned char * dest, unsigned int width, unsigned int height);
	void RGBA16ToRGB(const short * source, unsigned char * dest, unsigned int width, unsigned int height);
	void D32ToRGB(const float * source, unsigned char * dest, unsigned int width, unsigned int height);
	void savePPMP6(const char * fileName, unsigned int width, unsigned int height, const unsigned char * data);
}

#endif // __VIEWER_GL_UTILS_HPP__
