#pragma once

#include "LibMath/LibMath.h"
#include <vector>
#include "shader.h"
#include "camera.h"


struct PointLight
{
	LibMath::Vector4	m_DiffuseColor;
	LibMath::Vector4	m_AmbientColor;
	LibMath::Vector4	m_SpecularColor;

	float				m_Shininess;

	LibMath::Vector3	m_Position;

	explicit PointLight(LibMath::Vector4, LibMath::Vector4, LibMath::Vector4,
						float, 
						LibMath::Vector3);

	~PointLight() = default;
};

struct DirectionalLight
{
	LibMath::Vector4	m_DiffuseColor;
	LibMath::Vector4	m_AmbientColor;
	LibMath::Vector4	m_SpecularColor;

	float				m_Shininess;

	LibMath::Vector3	m_Direction;

	explicit DirectionalLight(LibMath::Vector4, LibMath::Vector4, LibMath::Vector4,
							  float, 
							  LibMath::Vector3);

	~DirectionalLight() = default;
};

struct SpotLight
{
	LibMath::Vector4	m_DiffuseColor;
	LibMath::Vector4	m_AmbientColor;
	LibMath::Vector4	m_SpecularColor;

	float				m_Shininess;

	LibMath::Vector3	m_Position;

	LibMath::Vector3	m_spotDirection;
	float				m_cutOff;
	float				m_outerCutOff;

	explicit SpotLight(LibMath::Vector4, LibMath::Vector4, LibMath::Vector4,
					   float, 
					   LibMath::Vector3,
					   LibMath::Vector3, float, float);

	~SpotLight() = default;
};

class Lights
{
public:
	std::vector<PointLight>			m_pointLights;
	std::vector<DirectionalLight>	m_directionalLights;
	std::vector<SpotLight>			m_spotLights;

	Lights();

	void	addPointLight(PointLight);
	void	addDirectionalLight(DirectionalLight);
	void	addSpotLight(SpotLight);

	void	setPointLight(Shader& shader, Camera camera) const;
	void	setDirectionalLight(Shader& shader, Camera camera) const;
	void	setSpotLight(Shader& shader, Camera camera) const;

	void	setUniform(Shader& shader, Camera camera) const;
};