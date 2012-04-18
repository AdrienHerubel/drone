#ifndef __VIEWER_CAMERA_HPP__
#define __VIEWER_CAMERA_HPP__

namespace viewer
{
	//! General camera data + transformations
	class Camera
	{
	public :
		Camera();
		~Camera();
		//! Computes and returns view matrix
		const float * worldToView();
		//! Returns view matrix
		const float * perspectiveProjection();
		//! Turn around the point of interest
		void turn(float phi, float theta);
		//! Shift the point of interest along the view plane
		void pan(float x, float y);
		//! Modify the radius around point of interest
		void zoom(float factor);
		//! Set the 4 corners of the viewport
		void setViewport(float x0, float y0, float xn, float yn);
		//! Set the perspective view frustum
		void setPerspective(float near, float far, float fov, float ratio);
		//! Center the view on a particular point
		void centerOn(float center[4], float halfDists[4]);
		//! Returns camera position
		const float * position() const { return m_eye; }
		//! Returns near distance
		float near() const { return m_near; }
		//! Returns far distance
		float far() const { return m_far; }
	private :
		Camera(const Camera &);
		Camera & operator=(const Camera & );

		//! Angle with x axis
		float m_phi;
		//! Angle with y axis
		float m_theta;
		//! Radius of rotation around point of interest
		float m_radius;
		//! Point of interest
		float m_o[4];
		//! Eye position
		float m_eye[4];
		//! View transform matrix
		float m_view[16];
		//! Near clipping plane
		float m_near;
		//! Far clipping plane
		float m_far;
		//! Field of view angle
		float m_fov;
		//! height/width ratio
		float m_ratio;
		//! 4-corners viewport
		float m_viewport[4];
		//! Perspetive matrix
		float m_perspective[16];
	};

} // namespace dcmr

#endif // __VIEWER_CAMERA_HPP__
