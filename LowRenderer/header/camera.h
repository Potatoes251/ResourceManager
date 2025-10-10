#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "LibMath/Vector.h"
#include "LibMath/Matrix.h"
#include "LibMath/Trigonometry.h"

class Camera
{
public :
	Camera(LibMath::Vector3 pos, LibMath::Vector3 forward);
	~Camera() = default;

	void updateVector();
	void updateView();

	void update(GLFWwindow* window);
	void processMouseMovement(double xpos, double ypos);

	//static void	mouseCallback(GLFWwindow* window, double xpos, double ypos)
	//{
	//	Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
	//	cam->processMouseMovement(xpos, ypos);
	//}

	LibMath::Matrix4 getVPMatrix() const;
	LibMath::Vector3 getPosition() const;

private :
	// mouse input variable
	bool m_firstMouse;
	float m_lastMouseX;
	float m_lastMouseY;

	LibMath::Vector3 m_position;
	LibMath::Vector3 m_forward;
	LibMath::Vector3 m_up;
	LibMath::Vector3 m_right;

	float m_pitch;	// x axis rotation
	float m_yaw;	// y axis rotation

	LibMath::Matrix4 m_view;
	LibMath::Matrix4 m_projection;
	LibMath::Matrix4 m_viewProjection;
};