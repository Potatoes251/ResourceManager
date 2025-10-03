#include "light.h"
#include "shader.h"

PointLight::PointLight(LibMath::Vector4 diffuse, LibMath::Vector4 ambient, LibMath::Vector4 specular, 
	float shininess, 
	LibMath::Vector3 position)
	: m_DiffuseColor(diffuse),
	m_AmbientColor(ambient),
	m_SpecularColor(specular),
	m_Shininess(shininess),
	m_Position(position) {}

DirectionalLight::DirectionalLight(LibMath::Vector4 diffuse, LibMath::Vector4 ambient, LibMath::Vector4 specular, 
	float shininess, 
	LibMath::Vector3 direction)
	: m_DiffuseColor(diffuse),
	m_AmbientColor(ambient),
	m_SpecularColor(specular),
	m_Shininess(shininess),
	m_Direction(direction) {}

SpotLight::SpotLight(LibMath::Vector4 diffuse, LibMath::Vector4 ambient, LibMath::Vector4 specular, 
	float shininess, 
	LibMath::Vector3 position, 
	LibMath::Vector3 spotDirection , float cutOff, float outerCutOff)
	: m_DiffuseColor(diffuse),
	m_AmbientColor(ambient),
	m_SpecularColor(specular),
	m_Shininess(shininess),
	m_Position(position),
	m_spotDirection(spotDirection),
	m_cutOff(cutOff),
	m_outerCutOff(outerCutOff) {}


Lights::Lights(): m_pointLights(), m_directionalLights(), m_spotLights(){}

void	Lights::addPointLight(PointLight pl)
{
	m_pointLights.push_back(pl);
}

void	Lights::addDirectionalLight(DirectionalLight dl)
{
	m_directionalLights.push_back(dl);
}

void	Lights::addSpotLight(SpotLight sl)
{
	m_spotLights.push_back(sl);
}

void Lights::setPointLight(Shader& shader, Camera camera) const
{
	for (size_t i = 0; i < m_pointLights.size(); ++i)
	{
		PointLight light = m_pointLights[i];
		std::string prefix = "pointLights[" + std::to_string(i) + "].";

		shader.setUniform((prefix + "diffuseColor").c_str(), &light.m_DiffuseColor);
		shader.setUniform((prefix + "ambientColor").c_str(), &light.m_AmbientColor);
		shader.setUniform((prefix + "specularColor").c_str(), &light.m_SpecularColor);

		shader.setUniform((prefix + "shininess").c_str(), &light.m_Shininess);

		shader.setUniform((prefix + "lightPosition").c_str(), light.m_Position);
	}
}

void Lights::setDirectionalLight(Shader& shader, Camera camera) const
{
	for (size_t i = 0; i < m_directionalLights.size(); ++i)
	{
		DirectionalLight light = m_directionalLights[i];
		std::string prefix = "dirLights[" + std::to_string(i) + "].";

		shader.setUniform((prefix + "diffuseColor").c_str(), &light.m_DiffuseColor);
		shader.setUniform((prefix + "ambientColor").c_str(), &light.m_AmbientColor);
		shader.setUniform((prefix + "specularColor").c_str(), &light.m_SpecularColor);

		shader.setUniform((prefix + "shininess").c_str(), &light.m_Shininess);

		shader.setUniform((prefix + "lightDir").c_str(), light.m_Direction);
	}
}

void Lights::setSpotLight(Shader& shader, Camera camera) const
{
	for (size_t i = 0; i < m_spotLights.size(); ++i)
	{
		SpotLight light = m_spotLights[i];
		std::string prefix = "spotLights[" + std::to_string(i) + "].";

		shader.setUniform((prefix + "diffuseColor").c_str(), &light.m_DiffuseColor);
		shader.setUniform((prefix + "ambientColor").c_str(), &light.m_AmbientColor);
		shader.setUniform((prefix + "specularColor").c_str(), &light.m_SpecularColor);

		shader.setUniform((prefix + "shininess").c_str(), &light.m_Shininess);

		shader.setUniform((prefix + "lightPosition").c_str(), light.m_Position);

		shader.setUniform((prefix + "spotDirection").c_str(), light.m_spotDirection);
		shader.setUniform((prefix + "cutOff").c_str(), &light.m_cutOff);
		shader.setUniform((prefix + "outerCutOff").c_str(), &light.m_outerCutOff);
	}
}

void	Lights::setUniform(Shader& shader, Camera camera) const
{
	shader.setUniform(("viewPosition"), camera.getPosition());

	setPointLight(shader, camera);

	setDirectionalLight(shader, camera);

	setSpotLight(shader, camera);

	int nbDL = m_directionalLights.size();
	int nbPL = m_pointLights.size();
	int nbSL = m_spotLights.size();

	shader.setUniform("NUM_DIR_LIGHTS", &nbDL);
	shader.setUniform("NUM_POINT_LIGHTS", &nbPL);
	shader.setUniform("NUM_SPOT_LIGHTS", &nbSL);
}


