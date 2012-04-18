#include "LinearAlgebra.hpp"

#include <cmath>
#include <sstream>

namespace viewer
{

	void mat4fToIdentity(float * mat)
	{
		mat4fCopy(mat, MAT4FIDENTITY);
	}

	void mat4fTranspose(const float * __restrict__ matA, float * __restrict__ matB)
	{
		matB[0] = matA[0]; matB[1] = matA[4]; matB[2] = matA[8]; matB[3] = matA[12];
		matB[4] = matA[1]; matB[5] = matA[5]; matB[6] = matA[9]; matB[7] = matA[13];
		matB[8] = matA[2]; matB[9] = matA[6]; matB[10] = matA[10]; matB[11] = matA[14];
		matB[12] = matA[3]; matB[13] = matA[7]; matB[14] = matA[11]; matB[15] = matA[15];
	}

	void mat4fInverse(const float * __restrict__ matA, float * __restrict__ matB)
	{
		float det;
		matB[0] =	matA[5]*matA[10]*matA[15] - matA[5]*matA[11]*matA[14] - matA[9]*matA[6]*matA[15]
			+ matA[9]*matA[7]*matA[14] + matA[13]*matA[6]*matA[11] - matA[13]*matA[7]*matA[10];
		matB[4] =  -matA[4]*matA[10]*matA[15] + matA[4]*matA[11]*matA[14] + matA[8]*matA[6]*matA[15]
			- matA[8]*matA[7]*matA[14] - matA[12]*matA[6]*matA[11] + matA[12]*matA[7]*matA[10];
		matB[8] =	matA[4]*matA[9]*matA[15] - matA[4]*matA[11]*matA[13] - matA[8]*matA[5]*matA[15]
			+ matA[8]*matA[7]*matA[13] + matA[12]*matA[5]*matA[11] - matA[12]*matA[7]*matA[9];
		matB[12] = -matA[4]*matA[9]*matA[14] + matA[4]*matA[10]*matA[13] + matA[8]*matA[5]*matA[14]
			- matA[8]*matA[6]*matA[13] - matA[12]*matA[5]*matA[10] + matA[12]*matA[6]*matA[9];
		matB[1] =  -matA[1]*matA[10]*matA[15] + matA[1]*matA[11]*matA[14] + matA[9]*matA[2]*matA[15]
			- matA[9]*matA[3]*matA[14] - matA[13]*matA[2]*matA[11] + matA[13]*matA[3]*matA[10];
		matB[5] =	matA[0]*matA[10]*matA[15] - matA[0]*matA[11]*matA[14] - matA[8]*matA[2]*matA[15]
			+ matA[8]*matA[3]*matA[14] + matA[12]*matA[2]*matA[11] - matA[12]*matA[3]*matA[10];
		matB[9] =  -matA[0]*matA[9]*matA[15] + matA[0]*matA[11]*matA[13] + matA[8]*matA[1]*matA[15]
			- matA[8]*matA[3]*matA[13] - matA[12]*matA[1]*matA[11] + matA[12]*matA[3]*matA[9];
		matB[13] =	matA[0]*matA[9]*matA[14] - matA[0]*matA[10]*matA[13] - matA[8]*matA[1]*matA[14]
			+ matA[8]*matA[2]*matA[13] + matA[12]*matA[1]*matA[10] - matA[12]*matA[2]*matA[9];
		matB[2] =	matA[1]*matA[6]*matA[15] - matA[1]*matA[7]*matA[14] - matA[5]*matA[2]*matA[15]
			+ matA[5]*matA[3]*matA[14] + matA[13]*matA[2]*matA[7] - matA[13]*matA[3]*matA[6];
		matB[6] =  -matA[0]*matA[6]*matA[15] + matA[0]*matA[7]*matA[14] + matA[4]*matA[2]*matA[15]
			- matA[4]*matA[3]*matA[14] - matA[12]*matA[2]*matA[7] + matA[12]*matA[3]*matA[6];
		matB[10] =	matA[0]*matA[5]*matA[15] - matA[0]*matA[7]*matA[13] - matA[4]*matA[1]*matA[15]
			+ matA[4]*matA[3]*matA[13] + matA[12]*matA[1]*matA[7] - matA[12]*matA[3]*matA[5];
		matB[14] = -matA[0]*matA[5]*matA[14] + matA[0]*matA[6]*matA[13] + matA[4]*matA[1]*matA[14]
			- matA[4]*matA[2]*matA[13] - matA[12]*matA[1]*matA[6] + matA[12]*matA[2]*matA[5];
		matB[3] =  -matA[1]*matA[6]*matA[11] + matA[1]*matA[7]*matA[10] + matA[5]*matA[2]*matA[11]
			- matA[5]*matA[3]*matA[10] - matA[9]*matA[2]*matA[7] + matA[9]*matA[3]*matA[6];
		matB[7] =	matA[0]*matA[6]*matA[11] - matA[0]*matA[7]*matA[10] - matA[4]*matA[2]*matA[11]
			+ matA[4]*matA[3]*matA[10] + matA[8]*matA[2]*matA[7] - matA[8]*matA[3]*matA[6];
		matB[11] = -matA[0]*matA[5]*matA[11] + matA[0]*matA[7]*matA[9] + matA[4]*matA[1]*matA[11]
			- matA[4]*matA[3]*matA[9] - matA[8]*matA[1]*matA[7] + matA[8]*matA[3]*matA[5];
		matB[15] =	matA[0]*matA[5]*matA[10] - matA[0]*matA[6]*matA[9] - matA[4]*matA[1]*matA[10]
			+ matA[4]*matA[2]*matA[9] + matA[8]*matA[1]*matA[6] - matA[8]*matA[2]*matA[5];

		det = matA[0]*matB[0] + matA[1]*matB[4] + matA[2]*matB[8] + matA[3]*matB[12];
		for (unsigned int i = 0; i < 16; ++i)
			matB[i] = matB[i] / det;
	}

	void mat4fRMSetCol(float * __restrict__ matA, const float * __restrict__ vecCol, unsigned int col)
	{
		matA[col]	 = vecCol[0];
		matA[4+col]	 = vecCol[1];
		matA[8+col]	 = vecCol[2];
		matA[12+col] = vecCol[3];
	}

	void mat4fCopy(float * __restrict__ matD, const float * __restrict__ matS)
	{
		matD[0] = matS[0]; matD[1] = matS[1]; matD[2] = matS[2]; matD[3] = matS[3];
		matD[4] = matS[4]; matD[5] = matS[5]; matD[6] = matS[6]; matD[7] = matS[7];
		matD[8] = matS[8]; matD[9] = matS[9]; matD[10] = matS[10]; matD[11] = matS[11];
		matD[12] = matS[12]; matD[13] = matS[13]; matD[14] = matS[14]; matD[15] = matS[15];
	}

	void mat4fMul(const float * __restrict__ matA, const float * __restrict__ matB, float * __restrict__ matC)
	{
		matC[0]	 = matA[0]*matB[0]	+ matA[1]*matB[4]  + matA[2]*matB[8]   + matA[3]*matB[12];
		matC[1]	 = matA[0]*matB[1]	+ matA[1]*matB[5]  + matA[2]*matB[9]   + matA[3]*matB[13];
		matC[2]	 = matA[0]*matB[2]	+ matA[1]*matB[6]  + matA[2]*matB[10] + matA[3]*matB[14];
		matC[3]	 = matA[0]*matB[3] + matA[1]*matB[7] + matA[2]*matB[11] + matA[3]*matB[15];

		matC[4]	 = matA[4]*matB[0]	+ matA[5]*matB[4]  + matA[6]*matB[8]   + matA[7]*matB[12];
		matC[5]	 = matA[4]*matB[1]	+ matA[5]*matB[5]  + matA[6]*matB[9]   + matA[7]*matB[13];
		matC[6]	 = matA[4]*matB[2]	+ matA[5]*matB[6]  + matA[6]*matB[10] + matA[7]*matB[14];
		matC[7]	 = matA[4]*matB[3] + matA[5]*matB[7] + matA[6]*matB[11] + matA[7]*matB[15];

		matC[8]	 = matA[8]*matB[0]	+ matA[9]*matB[4]  + matA[10]*matB[8]	+ matA[11]*matB[12];
		matC[9]	 = matA[8]*matB[1]	+ matA[9]*matB[5]  + matA[10]*matB[9]	+ matA[11]*matB[13];
		matC[10]  = matA[8]*matB[2]	 + matA[9]*matB[6]	+ matA[10]*matB[10] + matA[11]*matB[14];
		matC[11]  = matA[8]*matB[3] + matA[9]*matB[7] + matA[10]*matB[11] + matA[11]*matB[15];

		matC[12]  = matA[12]*matB[0]  + matA[13]*matB[4]  + matA[14]*matB[8]   + matA[15]*matB[12];
		matC[13]  = matA[12]*matB[1]  + matA[13]*matB[5]  + matA[14]*matB[9]   + matA[15]*matB[13];
		matC[14]  = matA[12]*matB[2]  + matA[13]*matB[6]  + matA[14]*matB[10] + matA[15]*matB[14];
		matC[15]  = matA[12]*matB[3] + matA[13]*matB[7] + matA[14]*matB[11] + matA[15]*matB[15];
	}

	void mat4fMulV(const float * __restrict__ matA, const float * __restrict__ vecA, float * __restrict__ vecB)
	{
		vecB[0]	 = matA[0]*vecA[0]	+ matA[1]*vecA[1]  + matA[2]*vecA[2]   + matA[3]*vecA[3];
		vecB[1]	 = matA[4]*vecA[0]	+ matA[5]*vecA[1]  + matA[6]*vecA[2]   + matA[7]*vecA[3];
		vecB[2]	 = matA[8]*vecA[0]	+ matA[9]*vecA[1]  + matA[10]*vecA[2]	+ matA[11]*vecA[3];
		vecB[3]	 = matA[12]*vecA[0]	 + matA[13]*vecA[1]	 + matA[14]*vecA[2]	  + matA[15]*vecA[3];
	}

	void mat4fMulV3(const float * __restrict__ matA, const float * __restrict__ vecA, float * __restrict__ vecB)
	{
		vecB[0]	 = matA[0]*vecA[0]	+ matA[1]*vecA[1]  + matA[2]*vecA[2]   + matA[3];
		vecB[1]	 = matA[4]*vecA[0]	+ matA[5]*vecA[1]  + matA[6]*vecA[2]   + matA[7];
		vecB[2]	 = matA[8]*vecA[0]	+ matA[9]*vecA[1]  + matA[10]*vecA[2]	+ matA[11];
		float w	 = matA[12]*vecA[0]	 + matA[13]*vecA[1]	 + matA[14]*vecA[2]	  + matA[15];
		vecB[0] /= w;
		vecB[1] /= w;
		vecB[2] /= w;
	}

	std::string mat4fToString(const float * matA)
	{
		std::ostringstream oss;
		for (unsigned int i = 0; i < 16; ++i)
		{
			oss << matA[i];
			if ((i+1)%4)
				oss << " , ";
			else
				oss << std::endl;
		}
		return oss.str();
	}
	std::string mat4dToString(const double * matA)
	{
		std::ostringstream oss;
		for (unsigned int i = 0; i < 16; ++i)
		{
			oss << matA[i];
			if ((i+1)%4)
				oss << " , ";
			else
				oss << std::endl;
		}
		return oss.str();
	}
	void vec4fCopy(float * __restrict__ vecD, const float * __restrict__ vecS)
	{
		vecD[0] = vecS[0];
		vecD[1] = vecS[1];
		vecD[2] = vecS[2];
		vecD[3] = vecS[3];
	}


	void vec4fAdd(const float * __restrict__ vecA, const float * __restrict__ vecB, float * __restrict__ vecC)
	{
		vecC[0] = vecA[0] + vecB[0];
		vecC[1] = vecA[1] + vecB[1];
		vecC[2] = vecA[2] + vecB[2];
		vecC[3] = vecA[3] + vecB[3];
	}

	void vec4fSub(const float * __restrict__ vecA, const float * __restrict__ vecB, float * __restrict__ vecC)
	{
		vecC[0] = vecA[0] - vecB[0];
		vecC[1] = vecA[1] - vecB[1];
		vecC[2] = vecA[2] - vecB[2];
		vecC[3] = vecA[3] - vecB[3];
	}

	void vec4fCCart(float * vecA)
	{
		vecA[0] = vecA[0] / vecA[3];
		vecA[1] = vecA[1] / vecA[3];
		vecA[2] = vecA[2] / vecA[3];
		vecA[3] = 1.f;
	}

	float vec3fNorm(const float * vecA)
	{
		float n1 = vecA[0] * vecA[0];
		float n2 = vecA[1] * vecA[1];
		float n3 = vecA[2] * vecA[2];
		return sqrt(n1 + n2 + n3);
	}

	void vec3fAdd(const float * __restrict__ vecA, const float * __restrict__ vecB, float * __restrict__ vecC)
	{
		vecC[0] = vecA[0] + vecB[0];
		vecC[1] = vecA[1] + vecB[1];
		vecC[2] = vecA[2] + vecB[2];
	}

	void vec3fNormalize(float * vecA, float norm)
	{
		vecA[0] /= norm;
		vecA[1] /= norm;
		vecA[2] /= norm;
	}

	void vec3fScale(float * vecA, float s)
	{
		vecA[0] *= s;
		vecA[1] *= s;
		vecA[2] *= s;
	}

	std::string vec4fToString(const float * vec)
	{
		std::ostringstream oss;
		oss << vec[0] << " , "
			<< vec[1] << " , "
			<< vec[2] << " , "
			<< vec[3];
		return oss.str();
	}

	std::string vec4dToString(const double * vec)
	{
		std::ostringstream oss;
		oss << vec[0] << " , "
			<< vec[1] << " , "
			<< vec[2] << " , "
			<< vec[3];
		return oss.str();
	}
	void vec3fCopy(float * __restrict__ vecD, const float * __restrict__ vecS)
	{
		vecD[0] = vecS[0];
		vecD[1] = vecS[1];
		vecD[2] = vecS[2];
	}

	void vec3fCross(const float * __restrict__ vecA, const float * __restrict__ vecB, float * __restrict__ vecC)
	{
		vecC[0] = vecA[1] * vecB[2] - vecA[2] * vecB[1];
		vecC[1] = vecA[2] * vecB[0] - vecA[0] * vecB[2];
		vecC[2] = vecA[0] * vecB[1] - vecA[1] * vecB[0];
	}

	float vec3fDot(const float * __restrict__ vecA, const float * __restrict__ vecB)
	{
		return vecA[0] * vecB[0] + vecA[1] * vecB[1] + vecA[2] * vecB[2];
	}

	void vecnfLerp(const float * __restrict__ vecA, const float * __restrict__ vecB, float * __restrict__ vecC,
				   unsigned int size, float ratio)
	{
		for (unsigned int i = 0; i < size; ++i)
		{
			vecC[i] = ratio * vecA[i] + (1.f-ratio) * vecB[i];
		}
	}

	void barycentricToCart(const float * __restrict__ a,
						   const float * __restrict__ b,
						   const float * __restrict__ c,
						   const float * __restrict__ bc,
						   float * __restrict__ coord)
	{
		float pA[4];
		vec4fCopy(pA, a);
		vec3fScale(pA, bc[0]);
		float pB[4];
		vec4fCopy(pB, b);
		vec3fScale(pB, bc[1]);
		float pC[4];
		vec4fCopy(pC, c);
		vec3fScale(pC, bc[2]);
		float pAB[4];
		vec3fAdd(pA, pB, pAB);
		vec3fAdd(pAB, pC, coord);
		coord[3] = 1.f;
	}

	void vectorizeMatrix(const float in[][4], float* out)
	{
		for(unsigned int u=0; u<4; ++u)
			for(unsigned int v=0; v<4; ++v)
				out[u*4 + v] = in[u][v];
	}

} // namespace viewer
