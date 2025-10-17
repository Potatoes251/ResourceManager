#ifndef APPLICATION_H
#define APPLICATION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <mutex>
#include <vector>
#include <functional>

#include "resourceManager.h"
#include "graph.h"
#include "camera.h"

class Application;

struct AppContext 
{
	ResourceManager*	m_rm;
	Graph*				m_graph;
	Application*		m_app;
	Camera*				m_camera;

	bool mesh1Loaded = false;
	bool mesh2Loaded = false;
	bool mesh3Loaded = false;
	bool mesh4Loaded = false;
};

class Application
{
public :
	Application(ResourceManager* rm, Graph* graph, Camera* camera)
		: m_ctx(rm, graph, this, camera)
	{
		open();
		glfwSetWindowUserPointer(m_window, &m_ctx);
	}
	~Application()
	{
		close();
	}

	void	open();				// open the window

	void	update() const;		// update the window each frame (need to be called)

	// post a function to the main thread
	template<class Func, typename... Args>
	void postToMain(Func&& f, Args&&... args)
	{
		{
			std::unique_lock<std::mutex> lock(m_mainMutex);
			// enqueue the function and its arguments as a callable without parameters
			m_mainThreadTasks.emplace_back(std::bind(std::forward<Func>(f), std::forward<Args>(args)...));
		}
		m_condition.notify_one();
	}

	// post a member function to the main thread
	template <typename Obj, typename MemFunc, typename... Args>
	void postToMain(Obj* obj, MemFunc func, Args&&... args)
	{
		{
			std::unique_lock<std::mutex> lock(m_mainMutex);
			// call member function
			m_mainThreadTasks.emplace_back([obj, func, args...]() mutable {
				(obj->*func)(args...);
				});
		}
		m_condition.notify_one();
	}

	void	runMainThreadTasks();

	void	close() const;			// close the window

	GLFWwindow* getWindow() const;	// getter for the window

private :
	GLFWwindow* m_window;
	AppContext m_ctx;

	std::condition_variable				m_condition;
	std::mutex							m_mainMutex;
	std::vector<std::function<void()>>	m_mainThreadTasks;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);	// function to be able to update the size of the window


#endif