#version 330 core

in vec3 fragPos;
in vec3 fragNormal;

uniform vec3 uViewPos;
uniform vec3 uLightDir;

out vec4 FragColor;

uniform vec3 COLOR_FRONT   = vec3(0.82, 0.76, 0.62);
uniform vec3 COLOR_BACK    = vec3(0.45, 0.40, 0.35);

uniform float AMBIENT = 0.4;
uniform float DIFFUSE = 0.8;
uniform float SHEEN_AMOUNT = 0.2;
uniform float SHEEN_WIDTH = 0.4;
uniform float ANISOTROPY = 0.8; 
uniform float ANISOTROPY_WIDTH = 0.2; 

// Generates a pseudo-random tangent direction from position
vec3 generateTangent(vec3 pos, vec3 normal) {
    vec3 tangent = normalize(cross(normal, vec3(sin(pos.x * 0.5), cos(pos.y * 0.5), sin(pos.z * 0.3))));
    return tangent;
}

void main() {
    vec3 normal = normalize(fragNormal);
    if (dot(normal, normalize(uViewPos - fragPos)) < 0.0)
        normal = -normal;

    vec3 baseColor = COLOR_FRONT;

    // Ambient term
    vec3 ambient = AMBIENT * baseColor;

    // Diffuse term
    float diff   = max(dot(normal, uLightDir), 0.0);
    vec3 diffuse = DIFFUSE * diff * baseColor;

    // Sheen term with anisotropy 
    vec3 viewDir  = normalize(uViewPos - fragPos);
    vec3 halfVec  = normalize(uLightDir + viewDir);
    
    // Standard sheen (isotropic): strongest at grazing angles
    float nh = max(dot(normal, halfVec), 0.0);
    float sheenFactor = sqrt(1.0 - nh * nh); // Perpendicular component
    float baseSheen = pow(sheenFactor, SHEEN_WIDTH);
    
    // Anisotropic component
    vec3 tangent = generateTangent(fragPos, normal);
    float tangentDot = dot(halfVec, tangent);
    
    float anisoSheen = pow(abs(tangentDot), ANISOTROPY_WIDTH);
    
    // Blend isotropic and anisotropic terms
    float finalSheen = mix(baseSheen, anisoSheen, ANISOTROPY);
    vec3 sheenColor = SHEEN_AMOUNT * finalSheen * vec3(1.0);

    vec3 result = ambient + diffuse + sheenColor;

    FragColor = vec4(result, 1.0);
}