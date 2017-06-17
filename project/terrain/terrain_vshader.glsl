#version 330

// inputs
in vec2 position;

// outputs
out vec4 vVertexOut;

void main() {
    vVertexOut = vec4(position.x, 0.0, -position.y, 1.0);
}
