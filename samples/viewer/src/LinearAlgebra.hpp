#ifndef __VIEWER_LINEAR_ALGEBRA_HPP__
#define __VIEWER_LINEAR_ALGEBRA_HPP__

#include <string> // Printing

namespace viewer
{	
	//! Copy matS into matD
	void mat4fCopy(float * __restrict__ matD, const float * __restrict__ matS);
	//! Identity matrix
	static const float MAT4FIDENTITY[16] =  
	{
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
	//! Set mat to identity
	void mat4fToIdentity(float * mat);
	//! Transpose matA and store results in matB
	void mat4fTranspose(const float * __restrict__ matA, float * __restrict__ matB);
	//! Inverse matA and store results in matB
	void mat4fInverse(const float * __restrict__ matA, float * __restrict__ matB);
	//! Set column col at vecCol in matA
	void mat4fRMSetCol(float * __restrict__ matA, const float * __restrict__ vecCol, unsigned int col);
	//! Multiply matA by matB and store result in matC
	void mat4fMul(const float * __restrict__ matA, const float * __restrict__ matB, float * __restrict__ matC);
	//! Multiply matA by vecA and store result in vecB
	void mat4fMulV(const float * __restrict__ matA, const float * __restrict__ vecA, float * __restrict__ vecB);
	//! Multiply matA by vecA and store result in vecB
	void mat4fMulV3(const float * __restrict__ matA, const float * __restrict__ vecA, float * __restrict__ vecB);
	//! Transform mat4 into a std::string
	std::string mat4fToString(const float * matA);
	//! Transform mat4 into a std::string
	std::string mat4dToString(const double * matA);

	//! Null vector
	static const float NULLVEC4f[4] = { 0.f, 0.f, 0.f, 0.f};
	//! Copy vecS in vecD
	void vec4fCopy(float * __restrict__ vecD, const float * __restrict__ vecS);
	//! Add vecA with vecB and store result in vecC
	void vec4fAdd(const float * __restrict__ vecA, const float * __restrict__ vecB, float * __restrict__ vecC);
	//! Substract vecB to vecA and store result in vecC
	void vec4fSub(const float * __restrict__ vecA, const float * __restrict__ vecB, float * __restrict__ vecC);
	//! Convert vec4f vec to std::string
	std::string vec4fToString(const float * vec);
	//! Convert vecA from Homoheneous coords to cartesian coords
	void vec4fCCart(float * vecA);
	//! Convert vec4f vec to std::string
	std::string vec4dToString(const double * vec);
	
	//! Copy vecS in vecD
	void vec3fCopy(float * __restrict__ vecD, const float * __restrict__ vecS);
	//! Add vecA with vecB and store result in vecC
	void vec3fAdd(const float * __restrict__ vecA, const float * __restrict__ vecB, float * __restrict__ vecC);
	//! Compute norm of vecA
	float vec3fNorm(const float * vecA);
	//! Normalize vecA with norm
	void vec3fNormalize(float * vecA, float norm);
	//! Multiply vecA by scalar s
	void vec3fScale(float * vecA, float s);
	//! Compute cross product vecA^vecB and store result in vecC
	void vec3fCross(const float * __restrict__ vecA, const float * __restrict__ vecB, float * __restrict__ vecC);
	//! Compute dot product vecA vecB and return result
	float vec3fDot(const float * __restrict__ vecA, const float * __restrict__ vecB);

	//! Interpolate linearly vecA with vecB with ratio and store results in vecC
	void vecnfLerp(const float * __restrict__ vecA, const float * __restrict__ vecB, float * __restrict__ vecC, 
				   unsigned int size, float ratio);
	
	/** convert barycentric coordinates of a triangle in world-space coordinates
	 * 
	 * \param pointA First point of the triangle
	 * \param pointB Second point of the triangle
	 * \param pointC Third point of the triangle 
	 * \param bc Barycentric coordinates
	 * \return World-space coordinates
	 */
	void barycentricToCart(const float * __restrict__ a,
						   const float * __restrict__ b,
						   const float * __restrict__ c,
						   const float * __restrict__ bc,
						   float * __restrict__ coord);	
	
	//! Two dim matrix to array
	void vectorizeMatrix(const float in[][4], float* out);
} // namespace viewer

#endif // __VIEWER_LINEAR_ALGEBRA_HPP__
