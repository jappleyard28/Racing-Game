layout (binding = 0) uniform sampler2D colourSampler;

uniform vec3 tint = vec3(1.0, 1.0, 1.0);

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 colour;

void main() {
    colour = texture(colourSampler, uv);
    colour.xyz *= tint;
}
