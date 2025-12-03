#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D fontSampler;

void main() {
    // Usar colores reales de eGUI ahora que sabemos que funciona
    // Obtener color de la textura
    vec4 texColor = texture(fontSampler, fragTexCoord);
    
    // Multiplicar color por textura (modo típico de UI)
    outColor = fragColor * texColor;
    
    // Asegurar alpha mínimo para visibilidad
    if (outColor.a > 0.01) {
        outColor.a = max(outColor.a, 0.9);
    } else {
        discard;
    }
}
