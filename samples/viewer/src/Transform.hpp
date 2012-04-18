#ifndef __VIEWER_GEOMETRY_TRANSFORM_HPP__
#define __VIEWER_GEOMETRY_TRANSFORM_HPP__

namespace viewer
{
	//! Translate matA with position vecP and store result in matB
	void translate(const float * __restrict__ matA, const float * __restrict__ vecP, float * __restrict__ matB);
	//! Dummy structs for compile time rotation selection
	struct AxisX {};
	struct AxisY {};
	struct AxisZ {};
	//! Rotate matA	 with axis selection (GL like) and angle. Store results in matB
	void rotate(const float * __restrict__ matA, const float * __restrict__ vecAxis, float angle, float * __restrict__ matB);
	//! Rotate matA	 around axis X	and angle. Store results in matB
	void rotate(const float * __restrict__ matA, const AxisX & axis, float angle, float * __restrict__ matB);
	//! Rotate matA	 around axis Y	and angle. Store results in matB
	void rotate(const float * __restrict__ matA, const AxisY & axis, float angle, float * __restrict__ matB);
	//! Rotate matA	 around axis Z	and angle. Store results in matB
	void rotate(const float * __restrict__ matA, const AxisZ & axis, float angle, float * __restrict__ matB);
	//! Setup a view frustrum in matA
	void frustum(float left, float right, float bottom, float top, float nearVal, float farVal, float * matA);
	//! Setup a orthographic projection in matA
	void ortho(float left, float right, float bottom, float top, float nearVal, float farVal, float * matA);
	//! Setup a perspective view frustrum in matA
	void perspective(float fovy, float aspect, float zNear, float zFar, float * matA);
	//! Setup a view transform gluLookAt like in matA
	void lookAt(const float * __restrict__	vecEye,
				const float * __restrict__	vecCenter,
				const float * __restrict__	vecUp,
				float * __restrict__  matA);
	//! Compute world space coordinates from screen space
	void unProject(const float * __restrict__ vecScreen,
				   const float * __restrict__ matView,
				   const float * __restrict__ matProj,
				   const float * __restrict__ viewport,
				   float * __restrict__ vecWorld);
	//! Remove scale factor from matA Store results in matB
	void removeScaleFromRotate(const float * __restrict__ matA, float * __restrict__ matB);

} // namespace viewer

#endif // __VIEWER_GEOMETRY_TRANSFORM_HPP__
