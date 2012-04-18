#include "ShaderGLSL.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <cstring>

namespace viewer
{
	ShaderGLSL::ShaderGLSL() :
		_initialized(false)
	{
		for (unsigned int i =0; i < NUM_SHADER_TYPES; i++) 
		{
			_shaderSourceUse[i] = false;
		}
	}

	ShaderGLSL::~ShaderGLSL() 
	{
		if (_initialized) {
			for (unsigned int i =0; i < NUM_SHADER_TYPES; i++) 
			{
				if (_shaderSourceUse[i])
				{
					delete[] _shaderSource[i];
					glDeleteShader(_shaderSourceObjects[i]);
				}
			}
			glDeleteProgram(_shaderProgramObject);
		}
	}

	ShaderGLSL::ShaderGLSL(const ShaderGLSL & shaderglsl)
	{
		copy(shaderglsl);
	}		

	ShaderGLSL & ShaderGLSL::operator=(const ShaderGLSL & shaderglsl)
	{
		copy(shaderglsl);
		return *this;
	}
	
	void ShaderGLSL::copy(const ShaderGLSL & shaderglsl)
	{
		_initialized = false;
		for (unsigned int i =0; i < NUM_SHADER_TYPES; i++) 
		{
			_shaderSourceUse[i] = shaderglsl._shaderSourceUse[i];
			if (_shaderSourceUse[i])
			{
				unsigned int bufferSize = strlen(shaderglsl._shaderSource[i]);
				_shaderSource[i] = new char[bufferSize];
				memcpy(_shaderSource[i], shaderglsl._shaderSource[i], bufferSize  * sizeof (char));		
			}
		}
	}

	void ShaderGLSL::initGPU(void) 
	{		
		if (_shaderSourceUse[VERTEX_SHADER])
		{
			compileShader(VERTEX_SHADER, GL_VERTEX_SHADER);
		}
		if (_shaderSourceUse[FRAGMENT_SHADER])
		{
			compileShader(FRAGMENT_SHADER, GL_FRAGMENT_SHADER);
		}
		linkShaders();
		_initialized = true;
	}
	
	void ShaderGLSL::setSource(ShaderType shaderType, unsigned int bufferSize, const char * source)
	{
		_shaderSource[shaderType] = new char[bufferSize+1];
		memcpy(_shaderSource[shaderType], source, bufferSize  * sizeof (char));		
		_shaderSource[shaderType][bufferSize-1] = '\0';
		_shaderSourceUse[shaderType] = true;
	}

	void ShaderGLSL::compileShader(ShaderType shaderType, unsigned int glShaderType)
	{
		_shaderSourceObjects[shaderType] = glCreateShader(glShaderType);
		glShaderSource(_shaderSourceObjects[shaderType], 
					   1, 
					   (const char**) & _shaderSource[shaderType],
					   NULL);
		glCompileShader(_shaderSourceObjects[shaderType]);
		int logLength = getShaderLogLength(_shaderSourceObjects[shaderType]);
		if (logLength > 1)
			std::cerr << getShaderLog(_shaderSourceObjects[shaderType], logLength) << std::endl;
		int status;
		glGetShaderiv(_shaderSourceObjects[shaderType], GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
			throw std::runtime_error(std::string("ShaderGLSL : Impossible to compile shader. \n"));
	}
	
	void ShaderGLSL::linkShaders()
	{
		_shaderProgramObject = glCreateProgram();
		for (unsigned int i = 0; i < NUM_SHADER_TYPES; ++i)
		{
			if (_shaderSourceUse[i] == true)
			{
				glAttachShader(_shaderProgramObject, _shaderSourceObjects[i]);
			}
		}
		glLinkProgram(_shaderProgramObject);

		int logLength = getProgramLogLength(_shaderProgramObject);
		if (logLength > 1)
		 	std::cerr << getProgramLog(_shaderProgramObject, logLength) << std::endl;
		int status;
		glGetProgramiv(_shaderProgramObject, GL_LINK_STATUS, &status);		
		if (status == GL_FALSE)
			throw std::runtime_error("ShaderGLSL : Impossible to link shader. \n");
	}

	int getShaderLogLength(unsigned int object)
	{
		int logLength;
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &logLength);
		return logLength;
	}

	std::string getShaderLog(unsigned int object, int logLength)
	{
		if (logLength)				
		{
			char * log = new char[logLength];
			glGetShaderInfoLog(object, logLength, &logLength, log);
			std::string logString(log);
			delete log;
			return logString;
		}
		return "";
	}

	int getProgramLogLength(unsigned int object)
	{
		int logLength;
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &logLength);
		return logLength;
	}

	std::string getProgramLog(unsigned int object, int logLength)
	{
		if (logLength)				
		{
			char * log = new char[logLength];
			glGetProgramInfoLog(object, logLength, &logLength, log);
			std::string logString(log);
			delete log;
			return logString;
		}
		return "";
	}

	void ShaderGLSL::enable(void) const
	{
		assert(_initialized);
		glUseProgram(_shaderProgramObject);
	}

	void ShaderGLSL::disable(void) const
	{
		assert(_initialized);
		glUseProgram(0);
	}
}


