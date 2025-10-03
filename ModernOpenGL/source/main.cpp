#include "Application.h"
#include "model.h"
#include "mesh.h"
#include "texture.h"
#include "light.h"
#include "graph.h"

#include <string>

#include "memoryLeak.h"

void initShader(Shader& shader)
{
	shader.setVertexShader("vertexShader.glsl");
	shader.setFragmentShader("Fragment.glsl");
	shader.link();
	shader.use();
}

Mesh loadMesh1(ResourceManager& rm)
{
	std::string file = "diablo3_pose/diablo3_pose.obj";
	rm.Create<Model>(file);

	return Mesh (rm.Get<Model>(file),
		LibMath::Matrix4::createTransform(LibMath::Vector3::zero(),
			LibMath::Vector3::up(),
			LibMath::Radian(0),
			LibMath::Vector3(1.f, 1.f, 1.f)));
}

Mesh loadMesh2(ResourceManager& rm)
{
	std::string file = "bunny.obj";
	rm.Create<Model>(file);

	return 	Mesh(rm.Get<Model>(file),
		LibMath::Matrix4::createTransform(LibMath::Vector3(0.f, 3.f, 0.f),
			LibMath::Vector3::up(),
			LibMath::Radian(0),
			LibMath::Vector3(.5f, .5f, .5f)));
}

void initGraph(Graph& graph, Mesh& mesh1, Mesh& mesh2, Texture& texture1, Texture& texture2)
{
	std::string filet1 = "container.jpg";
	texture1.setTexture(filet1);

	std::string filet2 = "awesomeface.png";
	texture2.setTexture(filet2);

	//NODEs

	Graph::SceneNode* n1 = graph.addChild(nullptr, mesh1);
	Graph::SceneNode* n2 = graph.addChild(n1, mesh2);

	n1->setTexture(texture1);
	n2->setTexture(texture2);
}

void initLights(Shader& shader, Camera& camera)
{
	//Light
	Lights lights;

	DirectionalLight light1(
		LibMath::Vector4(1.0f, 0.0f, 0.08f, 1.0f),   // ambient: dark blue-ish
		LibMath::Vector4(1.0f, 0.0f, 0.0f, 1.0f),      // diffuse: vibrant blue
		LibMath::Vector4(0.6f, 0.8f, 1.0f, 1.0f),      // specular: ice blue
		64.0f,                                         // shininess: high / metalic
		LibMath::Vector3(0, .0f, 1.f)              // light direction: from above and behind
	);

	lights.addDirectionalLight(light1);

	lights.setUniform(shader, camera);
}

void initResources(Shader& shader, Camera& camera, Application& app)
{
	initShader(shader);
	glfwSetWindowUserPointer(app.getWindow(), &camera);
	//glfwSetCursorPosCallback(app.getWindow(), Camera::mouseCallback);

	initLights(shader, camera);
}

void mainLoop(Application& app, Camera& camera, Graph& graph, Shader& shader)
{
	while (!glfwWindowShouldClose(app.getWindow()))
	{
		app.update();
		camera.update(app.getWindow());


		graph.startDrawing(camera, shader);


		glfwSwapBuffers(app.getWindow());
		glfwPollEvents();
	}
}

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	//_CrtSetBreakAlloc(1217827);
	{
		Application app;
		app.open();

		//RM
		ResourceManager rm;

		//MESH
		Mesh mesh1 = loadMesh1(rm);
		Mesh mesh2 = loadMesh2(rm);

		//SHADER
		Shader shader;

		//CAMERA
		Camera camera(LibMath::Vector3(0.0f, 0.0f, 5.0f), LibMath::Vector3(0.0f, 0.0f, -1.0f));

		initResources(shader, camera, app);

		//TEXTURES
		Graph graph;
		Texture texture1;
		Texture texture2;
		initGraph(graph, mesh1, mesh2, texture1, texture2);

		mainLoop(app, camera, graph, shader);

		rm.clearAll();

		app.close();
	}
	_CrtDumpMemoryLeaks();
	return 0;
}