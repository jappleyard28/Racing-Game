layout (location = 0) out vec2 uv;

uniform mat4 mvp;

const vec2 vertices[] = {
    vec2(-0.5, -0.5), vec2( 0.5, -0.5), vec2( 0.5,  0.5),
    vec2( 0.5,  0.5), vec2(-0.5,  0.5), vec2(-0.5, -0.5)
};

const vec2 texCoords[] = {
    vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(1.0, 0.0),
    vec2(1.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 1.0)
};

void main() {
    gl_Position = mvp * vec4(vertices[gl_VertexID], 0.0, 1.0);
    uv = texCoords[gl_VertexID];
}
