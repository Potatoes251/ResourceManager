#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Application
{
public :
	Application() = default;
	~Application() = default;

	void open();				// open the window

	void update() const;		// update the window each frame (need to be called)

	void close() const;			// close the window

	GLFWwindow* getWindow() const;	// getter for the window

private :
	GLFWwindow* m_window;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);	// function to be able to update the size of the window