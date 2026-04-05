#version 330 core

in vec3 fragPos;
in vec3 fragNormal;

uniform vec3 uViewPos;
uniform vec3 uLightDir;

out vec4 FragColor;

uniform vec3 COLOR_FRONT   = vec3(0.82, 0.76, 0.62);
uniform vec3 ClothColor    = vec3(0.45, 0.40, 0.35);

const float AMBIENT      = 0.22;
const float DIFFUSE      = 0.75;
const float SPECULAR     = 0.35;
const float SHININESS    = 96.0;

void main() {
    vec3 normal = normalize(fragNormal);
    if (dot(normal, normalize(uViewPos - fragPos)) < 0.0)
        normal = -normal;

    vec3 baseColor = gl_FrontFacing ? COLOR_FRONT : ClothColor;

    vec3 ambient = AMBIENT * baseColor;

    float diff   = max(dot(normal, uLightDir), 0.0);
    vec3 diffuse = DIFFUSE * diff * baseColor;

    vec3 viewDir  = normalize(uViewPos - fragPos);
    vec3 halfVec  = normalize(uLightDir + viewDir);
    float spec    = pow(max(dot(normal, halfVec), 0.0), SHININESS);
    vec3 specular = SPECULAR * spec * vec3(1.0); 

    vec3 result = ambient + diffuse + specular;

    FragColor = vec4(result, 1.0);
}