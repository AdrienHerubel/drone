#ifndef __VIEWER_SHADERS_HPP__
#define __VIEWER_SHADERS_HPP__

namespace viewer
{
	static const char VSFWD[] =
	"#version 330 core\n"
	"uniform mat4 Projection;\n"
	"uniform mat4 View;\n"
	"uniform mat4 Object;\n"
	"in vec3 VertexPosition;\n"
	"in vec3 VertexNormal;\n"
	"in vec2 VertexTexCoord;\n"
	"out vec2 TexCoord;\n"
	"out vec3 Normal;\n"
	"void main(void)\n"
	"{\n"
	"	 vec4 ecPosition = View * Object * vec4(VertexPosition, 1.0);\n"
	"	 TexCoord = VertexTexCoord;\n"
	"    Normal = VertexNormal;"
	"	 gl_Position =	 Projection * ecPosition;\n"
	"}\n";

	static const char FSFWD[] =
	"#version 330 core\n"
    "const float ScaleFactor = 2.0;\n"
    "const float C1 = 0.429043;\n"
    "const float C2 = 0.511664;\n"
    "const float C3 = 0.743125;\n"
    "const float C4 = 0.886227;\n"
    "const float C5 = 0.247708;\n"
    "// Constants for Old Town Square lighting \n"
    "const vec3 L00  = vec3( 0.3175995,  0.3571678,  0.3784286);"
    "const vec3 L1m1 = vec3( 0.3655063,  0.4121290,  0.4490332);"
    "const vec3 L10  = vec3(-0.0071628, -0.0123780, -0.0146215);"
    "const vec3 L11  = vec3(-0.1047419, -0.1183074, -0.1260049);"
    "const vec3 L2m2 = vec3(-0.1304345, -0.1507366, -0.1702497);"
    "const vec3 L2m1 = vec3(-0.0098978, -0.0155750, -0.0178279);"
    "const vec3 L20  = vec3(-0.0704158, -0.0762753, -0.0865235);"
    "const vec3 L21  = vec3( 0.0242531,  0.0279176,  0.0335200);"
    "const vec3 L22  = vec3(-0.2858534, -0.3235718, -0.3586478);"	
	"uniform sampler2D Diffuse;"
	"uniform vec4 DiffuseColor;"
	"uniform float SpecularPower;"
	"in vec2 TexCoord;\n"
	"in vec3 Normal;\n"
	"layout(location = 0) out vec4 FragmentColor_1;\n"
	"void main(void)\n"
	"{\n"
    "    vec3 shLight    = ScaleFactor * (C1 * L22 * (Normal.x * Normal.x - Normal.y * Normal.y) +"
    "                      C3 * L20 * Normal.z * Normal.z +"
    "                      C4 * L00 -"
    "                      C5 * L20 +"
    "                      2.0 * C1 * L2m2 * Normal.x * Normal.y +"
    "                      2.0 * C1 * L21  * Normal.x * Normal.z +"
    "                      2.0 * C1 * L2m1 * Normal.y * Normal.z +"
    "                      2.0 * C2 * L11  * Normal.x +"
    "                      2.0 * C2 * L1m1 * Normal.y +"
    "                      2.0 * C2 * L10  * Normal.z);\n"
	"    vec2 uv = vec2(TexCoord.x, 1.0 - TexCoord.y);\n"
	"	 vec4 color = texture( Diffuse, uv );\n"
	"	 FragmentColor_1 = vec4((color+DiffuseColor).rgb*shLight*(color.a+DiffuseColor.a), color.a+DiffuseColor.a);\n"
	"}\n";

} // namespace viewer

#endif // __VIEWER_SHADERS_HPP__
