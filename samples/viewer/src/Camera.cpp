#include "Camera.hpp"

#include "LinearAlgebra.hpp"
#include "Transform.hpp"
#include <cmath>

namespace viewer
{
	Camera::Camera() :
		m_phi(3.14/2.f),
		m_theta(3.14/2.f),
		m_radius(10.f),
		m_near(0.1f),
		m_far(1000.f),
		m_fov(60)		
	{
		vec4fCopy(m_o, NULLVEC4f);
		worldToView();
	}

	Camera::~Camera()
	{}

	const float * Camera::worldToView()
	{
		m_eye[0] = cos(m_theta) * sin(m_phi) * m_radius + m_o[0];	
		m_eye[1] = cos(m_phi) * m_radius + m_o[1] ;
		m_eye[2] = sin(m_theta) * sin(m_phi) * m_radius + m_o[2];	
		m_eye[3] = 1.f;
		float modelviewTransform[16];
		float up[4] = {0.f, (m_phi < M_PI)?1.f:-1.f, 0.f, 0.f};
		lookAt(m_eye, m_o, up, modelviewTransform);
		mat4fTranspose(modelviewTransform, m_view);
		return m_view;
	}

	void Camera::turn(float phi, float theta)
	{
		m_theta += 1.f * theta;
		m_phi   -= 1.f * phi;
		if (m_phi >= (2 * M_PI) - 0.1 )
			m_phi = 0.00001;
		else if (m_phi <= 0 )
			m_phi = 2 * M_PI - 0.1;
	}

	void Camera::pan(float x, float y)
	{
		float up[4] = {0.f, (m_phi < M_PI)?1.f:-1.f, 0.f, 0.f};
		float fwd[4] = {m_o[0] - m_eye[0], m_o[1] - m_eye[1], m_o[2] - m_eye[2], 0.f};
		vec3fNormalize(fwd, vec3fNorm(fwd));
		float side[4];
		vec3fCross(fwd, up, side);
		vec3fNormalize(side, vec3fNorm(side));
		vec3fCross(side, fwd, up);
		m_o[0] += up[0] * y * m_radius * 2;
		m_o[1] += up[1] * y * m_radius * 2;
		m_o[2] += up[2] * y * m_radius * 2;
		m_o[0] -= side[0] * x * m_radius * 2;
		m_o[1] -= side[1] * x * m_radius * 2;
		m_o[2] -= side[2] * x * m_radius * 2;		
	}

	void Camera::zoom(float factor)
	{
		m_radius += factor * m_radius ;
		if (m_radius < 0.1)
		{
			m_radius = 10.f;
			float fwd[4] = {m_o[0] - m_eye[0], m_o[1] - m_eye[1], m_o[2] - m_eye[2], 0.f};
			vec3fNormalize(fwd, vec3fNorm(fwd)); 
			vec3fScale(fwd, m_radius);
			vec3fAdd(m_eye, fwd, m_o);		
		}
	}
	
	const float * Camera::perspectiveProjection()
	{		
		return m_perspective;
	}
	
	void Camera::setViewport(float x0, float y0, float xn, float yn)
	{
		m_viewport[0] = x0;
		m_viewport[1] = y0;
		m_viewport[2] = xn;
		m_viewport[3] = yn;
		setPerspective(m_near, m_far, m_fov, xn/yn);
	}
	
	void Camera::setPerspective(float near, float far, float fov, float ratio)
	{
		m_near = near;
		m_far = far;
		m_fov = fov;
		m_ratio = ratio;
		perspective(fov, ratio, near, far, m_perspective );
	}

	void Camera::centerOn(float center[4], float halfDists[4])
	{
		vec4fCopy(m_o, center); 
		m_radius = vec3fNorm(halfDists) * 2.f;
		if (m_far < m_radius)
		{
			m_far = 1.5 *  m_radius;
			setPerspective(m_near, m_far, m_fov, m_ratio);
		}
	}

} // namespace dcmr
