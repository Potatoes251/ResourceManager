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
uniform vec3 lightPosition;
uniform vec3 viewPosition;

//spotlight specific
uniform vec3 spotDirection;
uniform float cutOff;
uniform float outerCutOff;

// attenuation
uniform float constant;
uniform float linear;
uniform float quadratic;

//output
out vec4 FragColor;

void main()
{
	//normalization
	vec3 normal = normalize(Normal);
	vec3 lightDirection = normalize(lightPosition - FragPos);
	vec3 viewDirection = normalize(viewPosition - FragPos);
	vec3 halfwayDirection = normalize(lightDirection + viewDirection);

	//attenuation
	float distance = length(lightPosition - FragPos);
	float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
	
	//intensity
    float theta = dot(lightDirection, normalize(-spotDirection));
    float epsilon = cutOff - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
	
	//ambientColor
	vec4 ambient = ambientColor * attenuation;
	
	//diffuse
	float diff = max(dot(normal, lightDirection), 0.0f);
	vec4 diffuse = diff * diffuseColor * attenuation * intensity;
	
	//specular - blinn
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), shininess);
    vec4 specular = spec * specularColor * attenuation * intensity;
	
	//attenuation
	//float distance = length(lightPosition - FragPos);
	//float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

	//combination
	vec4 result = ambient + diffuse + specular;
	
	//apply
	FragColor = result;
}