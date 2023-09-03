#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct DirectionalLight {
    vec3 direction;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;

};

struct Material {
    sampler2D texture_diffuse1;
    vec3 specular;

    float shininess;
};
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform DirectionalLight directionalLight;
uniform Material material;
uniform vec3 ambientLight;
uniform vec3 viewPosition;
// calculates the color when using a point light.
vec3 CalcPointLight(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular);
}

void main()
{
     vec3 ambient = ambientLight * vec3(texture(material.texture_diffuse1, TexCoords));
     FragColor = vec4(ambient, 1.0);
     float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
     if (brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
     else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}