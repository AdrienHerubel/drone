#include "Transform.hpp"
#include "LinearAlgebra.hpp"

#include <cmath>
#include <iostream>

namespace viewer
{
	void translate(const float * __restrict__ matA, const float * __restrict__ vecP, float * __restrict__ matB)
	{
		float matTmp[16];
		mat4fToIdentity(matTmp);
		mat4fRMSetCol(matTmp, vecP, 3);
		mat4fMul(matA, matTmp, matB);
	}

	void rotate(const float * __restrict__ matA, const float * __restrict__ vecAxis, float angle, float * __restrict__ matB)
	{
		if (vecAxis[0])
			return rotate(matA, AxisX(), angle, matB);
		else if (vecAxis[1])
			return rotate(matA, AxisY(), angle, matB);
		else
			return rotate(matA, AxisZ(), angle, matB);
	}

	void rotate(const float * __restrict__ matA, const AxisX & axis, float angle, float * __restrict__ matB)
	{
		float matTmp[16] =
			{
				1, 0, 0, 0,
				0, cos(angle), sin(angle), 0,
				0, -sin(angle), cos(angle), 0,
				0, 0, 0, 1
			};
		mat4fMul(matA, matTmp, matB);
	}

	void rotate(const float * __restrict__ matA, const AxisY & axis, float angle, float * __restrict__ matB)
	{
		float matTmp[16] =
			{
				cos(angle), 0, -sin(angle), 0,
				0, 1, 0, 0,
				sin(angle), 0, cos(angle), 0,
				0, 0, 0, 1
			};
		mat4fMul(matA, matTmp, matB);
	}

	void rotate(const float * __restrict__ matA, const AxisZ & axis, float angle, float * __restrict__ matB)
	{
		float matTmp[16] =
			{
				cos(angle), sin(angle), 0, 0,
				-sin(angle), cos(angle), 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1
			};
		mat4fMul(matA, matTmp, matB);
	}

	void frustum(float left, float right, float bottom, float top, float nearVal, float farVal, float * matA)
	{
		matA[0] = 2.0f*nearVal/(right-left); matA[1] = 0.0f; matA[2] = 0.0f; matA[3] = 0.0f;
		matA[4] = 0.0f;	matA[5] = 2.0f*nearVal/(top-bottom); matA[6] = 0.0f; matA[7] = 0.0f;
		matA[8] = (right+left)/(right-left); matA[9] = (top+bottom)/(top-bottom); matA[10] = -(farVal+nearVal)/(farVal-nearVal);matA[11] = -1.0f;
		matA[12] = 0.0f; matA[13] = 0.0f; matA[14] = -(2.0f*farVal*nearVal)/(farVal-nearVal); matA[15] = 0.0f;
	}

	void ortho(float left, float right, float bottom, float top, float nearVal, float farVal, float * matA)
	{
		float tx = -(right + left) / ( right - left);
		float ty = -(top + bottom) / ( top - bottom);
		float tz = -(farVal + nearVal) / ( farVal - nearVal);
		matA[0] = 2.0f/(right-left); matA[1] = 0.0f; matA[2] = 0.0f; matA[3] = 0.0f;
		matA[4] = 0.0f;	matA[5] = 2.0f/(top-bottom); matA[6] = 0.0f; matA[7] = 0.0f;
		matA[8] = 0.0f; matA[9] = 0.0f; matA[10] = -2.0f/(farVal-nearVal); matA[11] = 0.0f;
		matA[12] = tx; matA[13] = ty; matA[14] = tz; matA[15] = 1.0f;
	}

	void perspective(float fovy, float aspect, float zNear, float zFar, float * matA)
	{
		float xmin, xmax, ymin, ymax;
		ymax = zNear * tanf(fovy * M_PI / 360.0f);
		ymin = -ymax;
		xmin = ymin * aspect;
		xmax = ymax * aspect;
		frustum(xmin, xmax, ymin, ymax, zNear, zFar, matA);
	}

	void lookAt(const float * __restrict__	vecEye,
				const float * __restrict__	vecCenter,
				const float * __restrict__	vecUp,
				float * __restrict__  matA)
	{
		float forward[4];
		float side[4];
		float up[4];
		float eyeInv[4] = {-vecEye[0], -vecEye[1], -vecEye[2], 1.f};
		float mat[16];

		vec4fSub(vecCenter, vecEye, forward);
		vec3fNormalize(forward, vec3fNorm(forward));
		vec3fCross(forward, vecUp, side);
		vec3fNormalize(side, vec3fNorm(side));
		vec3fCross(side, forward, up);



		mat[0] = side[0];	  mat[1] = side[1];		mat[2] = side[2];	   mat[3] = 0.0f;
		mat[4] = up[0];		  mat[5] = up[1];		mat[6] = up[2];		   mat[7] = 0.0f;
		mat[8] = -forward[0]; mat[9] = -forward[1]; mat[10] = -forward[2]; mat[11] = 0.0f;
		mat[12] = 0.0f;		  mat[13] = 0.0f;		mat[14] = 0.0f;		   mat[15] = 1.0f;

		translate(mat, eyeInv, matA);
	}

	void unProject(const float * __restrict__ vecScreen,
				   const float * __restrict__ matView,
				   const float * __restrict__ matProj,
				   const float * __restrict__ viewport,
				   float * __restrict__ vecWorld)
	{
		float viewProj[16];
		float viewi[16];
		float proji[16];
		float viewProji[16];
		float viewProjiT[16];
		float v[4] =
			{
				2.f*((vecScreen[0] - viewport[0])/viewport[2])-1.f,
				2.f*((vecScreen[1] - viewport[1])/viewport[3])-1.f,
				2.f * vecScreen[2] - 1.f,
				1.f
			};
		float vWorld[4];
		float vWorldH[4];
		mat4fMul(matView, matProj, viewProj);
		mat4fInverse(viewProj, viewProji);
		mat4fTranspose(viewProji, viewProjiT);
		mat4fMulV(viewProjiT, v, vecWorld);
		vec4fCCart(vecWorld);
	}

	void removeScaleFromRotate(const float * __restrict__ matA, float * __restrict__ matB)
	{
		mat4fCopy(matB, matA);
		vec3fNormalize(& matB[0], vec3fNorm(&matB[0]));
		vec3fNormalize(& matB[4], vec3fNorm(&matB[4]));
		vec3fNormalize(& matB[8], vec3fNorm(&matB[8]));
	}

} // namespace viewer
