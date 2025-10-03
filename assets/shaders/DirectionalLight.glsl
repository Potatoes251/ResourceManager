#shader fragment
#version 330 core

//elements that will be given every frame
in vec3 Normal;
in vec3 FragPos;   // Fragment position in world space

//colors + shininess
uniform vec4 diffuseColor;
uniform vec4 ambientColor;
uniform vec4 specularColor;

uniform float shininess;

//light and view
uniform vec3 viewPosition;

uniform vec3 lightDir;

//output
out vec4 FragColor;

void main()
{
	//normalization
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(-lightDir);
	vec3 viewDirection = normalize(viewPosition - FragPos);
	vec3 halfwayDirection = normalize(lightDirection + viewDirection);
	
	//ambientColor
	vec4 ambient = ambientColor;
	
	//diffuse
	float diff = max(dot(normal, lightDirection), 0.0f);
	vec4 diffuse = diff * diffuseColor;
	
	//specular - blinn
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), shininess);
    vec4 specular = spec * specularColor;
	
	//combination
	vec4 result = ambient + diffuse + specular;
	
	//apply
	FragColor = result;
}