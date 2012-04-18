#include "GLUtils.hpp"
#include "GLCommon.hpp"

#include <cstdio>

namespace viewer
{

	void drawAxes(const float * view)
	{
		// Render the 3D axes.
		glViewport(0, 0, 75, 75);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix(); // Modelview.
		glLoadIdentity();
		glTranslatef(0.f, 0.f, -4.f);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix(); // Projection
		glLoadIdentity();
		glFrustum(-0.5, 0.5, -0.5, 0.5, 1., 10.);
		//glOrtho(-0.5, 0.5, -0.5, 0.5, 1., 10.);

		glLineWidth(2.f);
		glBegin(GL_LINES);
		glColor3f(1.f, 0.f, 0.f);   // X-axis.
		glVertex3f(0.f, 0.f, 0.f);
		glVertex3f(view[0], view[1], view[2]);

		glColor3f(0.f, 1.f, 0.f);   // Y-axis.
		glVertex3f(0.f, 0.f, 0.f);
		glVertex3f(view[4], view[5], view[6]);

		glColor3f(0.f, 0.f, 1.f);   // Z-axis.
		glVertex3f(0.f, 0.f, 0.f);
		glVertex3f(view[8], view[9], view[10]);
		glEnd();

		glPopMatrix(); // Projection

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();  // Modelview
	}


	void drawSpot(const float wpos[4], const float wdir[4])
	{
		glLineWidth(2.f);
		glBegin(GL_LINES);
		glColor3f(1.f, 0.f, 0.f);
		glVertex3fv(wpos);
		glColor3f(1.f, 1.f, 0.f);
		glVertex3f(wpos[0]+wdir[0],wpos[1]+wdir[1],wpos[2]+wdir[2]);
		glEnd();
	}
	
	void saveFrameBufferPPM(const char * fileName, unsigned int width, unsigned int height)
	{
		unsigned char * data = new unsigned char[width*height*4];
		unsigned char * dataRGB = new unsigned char[width*height*3];
		glReadPixels( 0,
					  0,
					  width,
					  height,
					  GL_RGBA,
					  GL_UNSIGNED_BYTE,
					  data);
		for (unsigned int i = 0; i < height; ++i)
		{
			for (unsigned int j = 0; j < width; ++j)
			{
				dataRGB[((height - i-1) * width+j)*3] =	 data[(i*width+j)*4];
				dataRGB[((height - i-1) * width+j)*3+1] = data[(i*width+j)*4+1];
				dataRGB[((height - i-1) * width+j)*3+2] =  data[(i*width+j)*4+2];
			}
		}

		FILE * file = fopen(fileName, "w");
		fprintf(file, "P6\n");
		fprintf(file, "%d %d\n", width, height);
		fprintf(file, "255\n");
		fwrite(dataRGB, width*height*3, sizeof(unsigned char), file);
		fclose(file);
		delete[] data;
	}

	void saveTexturePPM(const char * fileName, unsigned int textureIdGPU)
	{
		int width = 0;
		int height = 0;
		int internalFormat = 0;
		glBindTexture(GL_TEXTURE_2D, textureIdGPU);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
		switch (internalFormat)
		{
		case GL_RGBA16F :
		{
			short * data = new short[width*height*4];
			unsigned char * dataRGB = new unsigned char[width*height*3];
			glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_SHORT, data);
			RGBA16ToRGB(data, dataRGB, width, height);
			savePPMP6(fileName, width, height, dataRGB);
			delete[] data;
			delete[] dataRGB;
			break;
		}
		case GL_RGBA16 :
		{
			short * data = new short[width*height*4];
			unsigned char * dataRGB = new unsigned char[width*height*3];
			glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_SHORT, data);
			RGBA16ToRGB(data, dataRGB, width, height);
			savePPMP6(fileName, width, height, dataRGB);
			delete[] data;
			delete[] dataRGB;
			break;
		}
		case GL_RGBA8 :
		{
			unsigned char * data = new unsigned char[width*height*4];
			unsigned char * dataRGB = new unsigned char[width*height*3];
			glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			RGBA8ToRGB(data, dataRGB, width, height);
			savePPMP6(fileName, width, height, dataRGB);
			delete[] data;
			delete[] dataRGB;
			break;
		}
		case GL_DEPTH_COMPONENT32 :
		{
			float * data = new float[width*height];
			unsigned char * dataRGB = new unsigned char[width*height*3];
			glGetTexImage( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
			D32ToRGB(data, dataRGB, width, height);
			savePPMP6(fileName, width, height, dataRGB);
			delete[] data;
			delete[] dataRGB;
			break;
		}
		default : break;
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}


	void D32ToRGB(const float * source, unsigned char * dest, unsigned int width, unsigned int height)
	{
		for (unsigned int i = 0; i < height; ++i)
		{
			for (unsigned int j = 0; j < width; ++j)
			{
				dest[((height - i-1) * width+j)*3] =  source[(i*width+j)] * 255;
				dest[((height - i-1) * width+j)*3+1] = source[(i*width+j)] * 255;
				dest[((height - i-1) * width+j)*3+2] =	source[(i*width+j)] * 255;
			}
		}
	}

	void RGBA8ToRGB(const unsigned char * source, unsigned char * dest, unsigned int width, unsigned int height)
	{
		for (unsigned int i = 0; i < height; ++i)
		{
			for (unsigned int j = 0; j < width; ++j)
			{
				dest[((height - i-1) * width+j)*3] =  source[(i*width+j)*4];
				dest[((height - i-1) * width+j)*3+1] = source[(i*width+j)*4+1];
				dest[((height - i-1) * width+j)*3+2] =	source[(i*width+j)*4+2];
			}
		}
	}

	void RGBA16ToRGB(const short * source, unsigned char * dest, unsigned int width, unsigned int height)
	{
		for (unsigned int i = 0; i < height; ++i)
		{
			for (unsigned int j = 0; j < width; ++j)
			{
				dest[((height - i-1) * width+j)*3] =  source[(i*width+j)*4] * 255 / 32767;
				dest[((height - i-1) * width+j)*3+1] = source[(i*width+j)*4+1] * 255 / 32767;
				dest[((height - i-1) * width+j)*3+2] =	source[(i*width+j)*4+2] * 255 / 32767;
			}
		}
	}

	void savePPMP6(const char * fileName, unsigned int width, unsigned int height, const unsigned char * data)
	{
		FILE * file = fopen(fileName, "w");
		fprintf(file, "P6\n");
		fprintf(file, "%d %d\n", width, height);
		fprintf(file, "255\n");
		fwrite(data, width*height*3, sizeof(unsigned char), file);
		fclose(file);
	}

}
