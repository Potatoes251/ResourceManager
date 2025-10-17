#include "Application.h"
#include "model.h"
#include "mesh.h"
#include "texture.h"
#include "light.h"
#include "graph.h"
#include "SoundManager.h"

#include <string>

#include "memoryLeak.h"

void	initShader(Shader& shader)
{
	shader.setVertexShader("vertexShader.glsl");
	shader.setFragmentShader("Fragment.glsl");
	shader.link();
	shader.use();
}

void	onMesh1Loaded(Model* model, Graph* graph, ResourceManager& rm, Application* app)
{
	app->postToMain([=, &rm]() {
		model->uploadToGPU();

		Mesh mesh(model,
			LibMath::Matrix4::createTransform(
				LibMath::Vector3::zero(),
				LibMath::Vector3::up(),
				LibMath::Radian(3.1415),
				LibMath::Vector3(1.f, 1.f, 1.f)));

		Graph::SceneNode* node = graph->addChild(nullptr, mesh);
		node->setTexture(rm.Get<Texture>("container.jpg"));
		});
}

void	loadMesh1(ResourceManager& rm, Graph* graph, Application* app)
{
	std::string file = "diablo3_pose/diablo3_pose.obj";
	rm.createAsync<Model>(file, onMesh1Loaded, graph, std::ref(rm), app);
}

void	copyMesh1(ResourceManager& rm, Graph* graph)
{
	Model* model = rm.Get<Model>("diablo3_pose/diablo3_pose.obj");

	if (model == nullptr) return;

	Mesh mesh(model,
		LibMath::Matrix4::createTransform(
			LibMath::Vector3(0.f, 0.f, 2.f),
			LibMath::Vector3::up(),
			LibMath::Radian(3.1415),
			LibMath::Vector3(1.f, 1.f, 1.f)));

	Graph::SceneNode* node = graph->addChild(nullptr, mesh);

	node->setTexture(rm.Get<Texture>("container.png"));
}

void	onMesh2Loaded(Model* model, Graph* graph, ResourceManager& rm, Application* app)
{
	app->postToMain([=, &rm]() {
		model->uploadToGPU();

		Mesh mesh(model,
			LibMath::Matrix4::createTransform(
				LibMath::Vector3(0.f, 3.f, 0.f),
				LibMath::Vector3::up(),
				LibMath::Radian(0),
				LibMath::Vector3(.5f, .5f, .5f)));

		Graph::SceneNode* node = graph->addChild(nullptr, mesh);
		node->setTexture(rm.Get<Texture>("Potatoes.png"));
		});
}

void	loadMesh2(ResourceManager& rm, Graph* graph, Application* app)
{
	std::string file = "locomotive.obj";
	rm.createAsync<Model>(file, onMesh2Loaded, graph, std::ref(rm), app);
}

void	copyMesh2(ResourceManager& rm, Graph* graph)
{
	Model* model = rm.Get<Model>("locomotive.obj");

	if (model == nullptr) return;

	Mesh mesh(model,
		LibMath::Matrix4::createTransform(
			LibMath::Vector3(0.f, 3.f, 2.f),
			LibMath::Vector3::up(),
			LibMath::Radian(0),
			LibMath::Vector3(1.f, 1.f, 1.f)));

	Graph::SceneNode* node = graph->addChild(nullptr, mesh);

	node->setTexture(rm.Get<Texture>("Potatoes.png"));
}

void	onMesh3Loaded(Model* model, Graph* graph, ResourceManager& rm, Application* app)
{
	app->postToMain([=, &rm]() {
		model->uploadToGPU();

		Mesh mesh(model,
			LibMath::Matrix4::createTransform(
				LibMath::Vector3(0.f, -3.f, 0.f),
				LibMath::Vector3::up(),
				LibMath::Radian(-3.1415 / 2),
				LibMath::Vector3(2.f, 2.f, 2.f)));

		Graph::SceneNode* node = graph->addChild(nullptr, mesh);
		node->setTexture(rm.Get<Texture>("Potatoes.jpg"));
		});
}

void	loadMesh3(ResourceManager& rm, Graph* graph, Application* app)
{
	std::string file = "dragon.obj";
	rm.createAsync<Model>(file, onMesh3Loaded, graph, std::ref(rm), app);
}

void	copyMesh3(ResourceManager& rm, Graph* graph)
{
	Model* model = rm.Get<Model>("dragon.obj");

	if (model == nullptr) return;

	Mesh mesh(model,
		LibMath::Matrix4::createTransform(
			LibMath::Vector3(0.f, -3.f, 2.f),
			LibMath::Vector3::up(),
			LibMath::Radian(-3.1415 / 2),
			LibMath::Vector3(1.f, 1.f, 1.f)));

	Graph::SceneNode* node = graph->addChild(nullptr, mesh);

	node->setTexture(rm.Get<Texture>("Potatoes.png"));
}

void	onMesh4Loaded(Model* model, Graph* graph, ResourceManager& rm, Application* app)
{
	app->postToMain([=, &rm]() {
		model->uploadToGPU();

		Mesh mesh(model,
			LibMath::Matrix4::createTransform(
				LibMath::Vector3(3.f, 0.f, 0.f),
				LibMath::Vector3::up(),
				LibMath::Radian(0),
				LibMath::Vector3(2.f, 2.f, 2.f)));

		Graph::SceneNode* node = graph->addChild(nullptr, mesh);
		node->setTexture(rm.Get<Texture>("container.jpg"));
		});
}

void	loadMesh4(ResourceManager& rm, Graph* graph, Application* app)
{
	std::string file = "armadillo.obj";
	rm.createAsync<Model>(file, onMesh4Loaded, graph, std::ref(rm), app);
}

void	copyMesh4(ResourceManager& rm, Graph* graph)
{
	Model* model = rm.Get<Model>("armadillo.obj");

	if (model == nullptr) return;

	Mesh mesh(model,
		LibMath::Matrix4::createTransform(
			LibMath::Vector3(3.f, 0.f, 2.f),
			LibMath::Vector3::up(),
			LibMath::Radian(0),
			LibMath::Vector3(1.f, 1.f, 1.f)));

	Graph::SceneNode* node = graph->addChild(nullptr, mesh);

	node->setTexture(rm.Get<Texture>("container.png"));
}

void	initLights(Shader& shader, Camera& camera)
{
	//Light
	Lights lights;

	DirectionalLight light1(
		LibMath::Vector4(0.1f, 0.1f, 0.15f, 1.0f), // ambient: subtle blue-ish
		LibMath::Vector4(0.7f, 0.7f, 0.8f, 1.0f),  // diffuse: moderate
		LibMath::Vector4(0.5f, 0.5f, 0.6f, 1.0f),  // specular: not too strong
		32.0f,                                     // shininess: typical Blinn-Phong
		LibMath::Vector3(0, .0f, 1.f)				// light direction: from above
	);

	lights.addDirectionalLight(light1);

	lights.setUniform(shader, camera);
}

void	initResources(Shader& shader, Camera& camera, ResourceManager& rm)
{
	rm.Create<Texture>("awesomeface.png");
	rm.Create<Texture>("container.jpg");
	rm.Create<Texture>("Potatoes.jpg");

	initShader(shader);
	initLights(shader, camera);
}

void	processInput(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
	if (key == GLFW_KEY_1)
	{
		if (ctx->mesh1Loaded)
		{
			copyMesh1(*(ctx->m_rm), ctx->m_graph);
		}
		else
		{
			ctx->mesh1Loaded = true;
			loadMesh1(*(ctx->m_rm), ctx->m_graph, ctx->m_app);
		}
	}
	else if (key == GLFW_KEY_2)
	{
		if (ctx->mesh2Loaded)
		{
			copyMesh2(*(ctx->m_rm), ctx->m_graph);
		}
		else
		{
			ctx->mesh2Loaded = true;
			loadMesh2(*(ctx->m_rm), ctx->m_graph, ctx->m_app);
		}
	}
	else if (key == GLFW_KEY_3)
	{
		if (ctx->mesh3Loaded)
		{
			copyMesh3(*(ctx->m_rm), ctx->m_graph);
		}
		else
		{
			ctx->mesh3Loaded = true;
			loadMesh3(*(ctx->m_rm), ctx->m_graph, ctx->m_app);
		}
	}
	else if (key == GLFW_KEY_4)
	{
		if (ctx->mesh4Loaded)
		{
			copyMesh4(*(ctx->m_rm), ctx->m_graph);
		}
		else
		{
			ctx->mesh4Loaded = true;
			loadMesh4(*(ctx->m_rm), ctx->m_graph, ctx->m_app);
		}
	}
}

void	mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
	ctx->m_camera->processMouseMovement(xpos, ypos);
}

bool	mainLoop(Application& app, Camera& camera, Graph& graph, Shader& shader)
{
	while (!glfwWindowShouldClose(app.getWindow()))
	{
		app.update();
		app.runMainThreadTasks();
		camera.update(app.getWindow());

		if (glfwGetKey(app.getWindow(), GLFW_KEY_R) == GLFW_PRESS)
		{
			return true;
		}

		graph.startDrawing(camera, shader);

		glfwSwapBuffers(app.getWindow());
		glfwPollEvents();
	}
	return false;
}

int		main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	//_CrtSetBreakAlloc(1217827);
	bool reset = true;
	bool multithread = true;

	while (reset)
	{
		ResourceManager rm(multithread);
		Graph			graph;
		Camera			camera(LibMath::Vector3(0.0f, 0.0f, -5.0f), LibMath::Vector3(0.0f, 0.0f, -1.0f));
		Application		app(&rm, &graph, &camera);
		Shader			shader;

		initResources(shader, camera, rm);

		glfwSetKeyCallback(app.getWindow(), processInput);
		glfwSetCursorPosCallback(app.getWindow(), mouseCallback);

		reset = mainLoop(app, camera, graph, shader);

		multithread = !multithread;
	}
	_CrtDumpMemoryLeaks();
	return 0;
}