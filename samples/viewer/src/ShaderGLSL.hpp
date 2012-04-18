#ifndef __VIEWER_SHADERGLSL_HPP__
#define __VIEWER_SHADERGLSL_HPP__

#include "GLCommon.hpp"
#include <vector>
#include <string>

namespace viewer
{
	class ShaderGLSL
	{
	public :
		enum ShaderType
		{
			VERTEX_SHADER = 0,
			FRAGMENT_SHADER = 1,
			GEOMETRY_SHADER = 2
		};

		/*! \brief Default constructor
		 */
		ShaderGLSL();
		/*! \brief Destructor
		 */
		~ShaderGLSL();
		/*! \brief Copy constructor
		 */
		ShaderGLSL(const ShaderGLSL &);
		/*! \brief Assignation operator
		 */
		ShaderGLSL & operator=(const ShaderGLSL &);
				
		/*! Compile source and link shaders */
		void initGPU();
		/*! Enable shader usage */
		void enable() const;
		/*! Disable shader usage */
		void disable() const;
		/*! Handle accessor */
		inline GLhandleARB  shaderProgramObject() const {return _shaderProgramObject;}
		/*! Sets source data for given type of shader */
		void setSource(ShaderType shaderType, unsigned int bufferSize, const char * source);
	private :
		void copy(const ShaderGLSL &);

		/*! Compile a shader. Throws exception in case of error */
		void compileShader(ShaderType shaderType, unsigned int glShaderType);
		/*! Attach and link used shaders. Throws exception in case of error */
		void linkShaders();
		bool _initialized;
		static const unsigned int NUM_SHADER_TYPES = 3;
		char * _shaderSource[NUM_SHADER_TYPES];
		unsigned int _shaderSourceObjects[NUM_SHADER_TYPES];
		bool _shaderSourceUse[NUM_SHADER_TYPES];
		unsigned int _shaderProgramObject;		
	};
	
	/*! Retrieve shader compilation error/warning log length */
	int getShaderLogLength(unsigned int object);
	/*! Retrieve error/warning log from shader compilation */
	std::string getShaderLog(unsigned int object, int logLength);
	/*! Retrieve program link error/warning log length */
	int getProgramLogLength(unsigned int object);
	/*! Retrieve error/warning log from program link */
	std::string getProgramLog(unsigned int object, int logLength);
} // namespace viewer

#endif // __VIEWER_SHADERGLSL_HPP__
