#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in uint inColor; // RGBA packed as uint

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec4 fragColor;

layout(push_constant) uniform PushConstants {
    vec2 scale;
    vec2 translate;
} pc;

void main() {
    fragTexCoord = inTexCoord;
    
    // Unpack RGBA from uint32 (formato: ARGB)
    // Bits: A=24-31, B=16-23, G=8-15, R=0-7
    float r = float((inColor >> 0) & 0xFF) / 255.0;
    float g = float((inColor >> 8) & 0xFF) / 255.0;
    float b = float((inColor >> 16) & 0xFF) / 255.0;
    float a = float((inColor >> 24) & 0xFF) / 255.0;
    fragColor = vec4(r, g, b, a);
    
    // Convert screen coordinates to clip space
    // eGUI coordinate system: (0,0) = top-left, Y increases downward
    // Vulkan clip space: (-1,-1) = bottom-left, (1,1) = top-right, Y increases upward
    //
    // Standard transformation:
    // X: [0, width]  -> [-1, 1]  => clipX = (x * scale.x) + translate.x
    // Y: [0, height] -> [1, -1]  => clipY = (y * scale.y) + translate.y (scale.y is negative to flip Y)
    float clipX = (inPosition.x * pc.scale.x) + pc.translate.x;
    float clipY = (inPosition.y * pc.scale.y) + pc.translate.y;
    
    // Clamp a rango válido para evitar problemas de precisión
    clipX = clamp(clipX, -2.0, 2.0);
    clipY = clamp(clipY, -2.0, 2.0);
    
    // Asegurar que Z y W sean correctos
    gl_Position = vec4(clipX, clipY, 0.0, 1.0);
}


