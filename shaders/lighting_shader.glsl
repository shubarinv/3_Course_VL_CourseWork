#shader vertex
#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 3) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
    #shader fragment
    #version 410 core

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;

    vec3 mat_diffuse;
    vec3 mat_specular;
    vec3 mat_ambient;

    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 30
#define NR_DIR_LIGHTS 30
#define NR_SPOT_LIGHTS 30

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform vec3 viewPos;
uniform int NUM_POINT_LIGHTS;
uniform int NUM_SPOT_LIGHTS;
uniform int NUM_DIR_LIGHTS;
uniform DirLight dirLights[NR_DIR_LIGHTS];
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight[NR_SPOT_LIGHTS];
uniform Material material;
uniform int useTexture;
uniform sampler2D shadowMap;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main()
{
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================
    // phase 1: directional lighting
    vec3 result;
    for (int i = 0; i < NUM_DIR_LIGHTS; i++)
    result += CalcDirLight(dirLights[i], norm, viewDir);
    // phase 2: point lights
    for (int i = 0; i < NUM_POINT_LIGHTS; i++)
    result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    // phase 3: spot light
    for (int i = 0; i < NUM_SPOT_LIGHTS; i++)
    result += CalcSpotLight(spotLight[i], norm, FragPos, viewDir);

    FragColor = vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    if (useTexture==1){
        ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
        float shadow = ShadowCalculation(FragPosLightSpace);
        return (ambient + (1.0 - shadow) * (diffuse + specular)) * vec3(texture(material.diffuse, TexCoords));
    }
    else {
        ambient = light.ambient * material.mat_diffuse;
        diffuse = light.diffuse * diff *material. mat_diffuse;
        specular = light.specular * spec * material.mat_specular;
        float shadow = ShadowCalculation(FragPosLightSpace);
        return (ambient + (1.0 - shadow) * (diffuse + specular)) *material. mat_diffuse;
    }

}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    if (useTexture==1){
        ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    }
    else {
        ambient = light.ambient * material.mat_diffuse;
        diffuse = light.diffuse * diff *material. mat_diffuse;
        specular = light.specular * spec * material.mat_specular;

    }

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    if (useTexture==1){
        float shadow = ShadowCalculation(FragPosLightSpace);
        return (ambient + (1.0 - shadow) * (diffuse + specular)) * vec3(texture(material.diffuse, TexCoords));
    }
    else {
        float shadow = ShadowCalculation(FragPosLightSpace);
        return (ambient + (1.0 - shadow) * (diffuse + specular)) *material. mat_diffuse;
    }
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    if (useTexture==1){
        ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
        diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
        specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    }
    else {
        ambient = light.ambient * material.mat_diffuse;
        diffuse = light.diffuse * diff *material. mat_diffuse;
        specular = light.specular * spec * material.mat_specular;

    }
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    if (useTexture==1){
        float shadow = ShadowCalculation(FragPosLightSpace);
        return (ambient + (1.0 - shadow) * (diffuse + specular)) * vec3(texture(material.diffuse, TexCoords));
    }
    else {
        float shadow = ShadowCalculation(FragPosLightSpace);
        return (ambient + (1.0 - shadow) * (diffuse + specular)) *material. mat_diffuse;
    }
}