#version 330 core
out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct PointLight {
    vec3 position;

	float constant;
	float linear;
	float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define MAX_POINT_LIGHTS 10
#define MAX_DIR_LIGHTS 3

uniform int numPointLights;
uniform int numDirLights;

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform DirLight dirLights[MAX_DIR_LIGHTS];

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, int i);

in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace[MAX_DIR_LIGHTS];
  
uniform vec3 viewPos;

uniform vec3 objectColor;
uniform Material material;

uniform sampler2D shadowMap;

float ShadowCalculation(vec3 normal, vec3 lightDir, vec4 fragPosLightSpace, int i)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;

    return shadow;
}

void main() {
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 result = vec3(0);

	for(int i = 0; i < min(numPointLights, MAX_POINT_LIGHTS); i++) {
		result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
	}

	for(int i = 0; i < min(numDirLights, MAX_DIR_LIGHTS); i++) {
		result += CalcDirLight(dirLights[i], norm, viewDir, i);
	}

	FragColor = vec4(result, 1.0);
} 

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	
	vec3 ambient = light.ambient * material.ambient;
	vec3 diffuse = light.diffuse * diff * material.diffuse;
	vec3 specular = light.specular * spec * material.specular;

	return (ambient + diffuse + specular) * attenuation;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, int i) {
	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
	vec3 ambient = light.ambient * material.ambient;
	vec3 diffuse = light.diffuse * diff * material.diffuse;
	vec3 specular = light.specular * spec * material.specular;

	float shadow = ShadowCalculation(normal, lightDir, FragPosLightSpace[i], i);

	return ambient + (1.0 - shadow) * (diffuse + specular);
}
