layout (binding = 0) uniform sampler2D colourSampler;

uniform float gamma = 2.2;

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 colour;

void main() {
    colour = vec4(pow(texture(colourSampler, uv).xyz, vec3(1.0 / gamma)), 1.0);
}
