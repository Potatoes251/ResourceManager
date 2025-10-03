#include "camera.h"
#include "LibMath/Angle.h"
#include "LibMath/Arithmetic.h"

Camera::Camera(LibMath::Vector3 pos, LibMath::Vector3 forward)
{
	m_position = pos;
	m_forward = forward;
    m_forward.normalize();

    m_projection = LibMath::Matrix4::perspective(M_PI / 2.f, 1.33f, .1f, 250.f);

    m_up = LibMath::Vector3::up();
    m_right = LibMath::Vector3::right();

    m_pitch = std::asin(-m_forward.m_y);  // Negative Y due to pitch being positive upward
    m_yaw = std::atan2(m_forward.m_x, -m_forward.m_z);  // Look at +x is +yaw, look at -z is yaw = 0

    updateVector();
}

void Camera::updateVector()
{
    LibMath::Radian radYaw = LibMath::Radian(m_yaw);
    LibMath::Radian radPitch = LibMath::Radian(m_pitch);

    float cosPitch = LibMath::cos(radPitch);
    float sinPitch = LibMath::sin(radPitch);
    float cosYaw = LibMath::cos(radYaw);
    float sinYaw = LibMath::sin(radYaw);

    // Right vector (X-axis)
    m_right = LibMath::Vector3{ cosYaw, 0.0f, -sinYaw };
    m_right.normalize();

    // Forward vector (Z-axis, looking direction)
    m_forward = LibMath::Vector3{ sinYaw * cosPitch, sinPitch, cosPitch * cosYaw };
    m_forward.normalize();

    //m_forward = m_position - LibMath::Vector3::zero();
    //m_forward.normalize();

    //m_right = LibMath::Vector3::up().cross(m_forward);
    //m_right.normalize();

    // Up vector (Y-axis)
    m_up = m_forward.cross(m_right);
    m_up.normalize();

    updateView();
}

void Camera::updateView()   // lookat matrix
{
    m_view = LibMath::Matrix4(
        m_right.m_x,                m_up.m_x,               -m_forward.m_x,              .0f,
        m_right.m_y,                m_up.m_y,               -m_forward.m_y,              .0f,
        m_right.m_z,                m_up.m_z,               -m_forward.m_z,              .0f,
        -m_right.dot(m_position),   -m_up.dot(m_position),  -m_forward.dot(m_position), 1.0f);

    m_viewProjection = m_projection * m_view;
}

void Camera::update(GLFWwindow* window)
{
    const float moveSpeed = 0.01f;
    const float zoomSpeed = 0.01f;

    // Forward (O)
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
    {
        m_position += m_forward * zoomSpeed;
    }

    // Backward (U)
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
        m_position -= m_forward * zoomSpeed;
    }

    // Left (J)
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        m_position -= m_right * moveSpeed;
    }

    // Right (L)
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        m_position += m_right * moveSpeed;
    }

    // Up (K)
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        m_position -= m_up * moveSpeed;
    }

    // Down (I)
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        m_position += m_up * moveSpeed;
    }

    updateVector();
}

void Camera::processMouseMovement(double xpos, double ypos)
{
    if (m_firstMouse)
    {
        m_lastMouseX = xpos;
        m_lastMouseY = ypos;
        m_firstMouse = false;
    }

    float xoffset = xpos - m_lastMouseX;
    float yoffset = m_lastMouseY - ypos;
    m_lastMouseX = xpos;
    m_lastMouseY = ypos;

    float sensitivity = 0.001f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    m_yaw -= xoffset;
    m_pitch -= yoffset;

    const float pitchLimit = M_PI / 2.0f - 0.01f;
    if (m_pitch > pitchLimit) m_pitch = pitchLimit;
    if (m_pitch < -pitchLimit) m_pitch = -pitchLimit;

    updateVector();
}

LibMath::Matrix4 Camera::getVPMatrix() const
{
    return m_viewProjection;
}

LibMath::Vector3 Camera::getPosition() const
{
    return m_position;
}