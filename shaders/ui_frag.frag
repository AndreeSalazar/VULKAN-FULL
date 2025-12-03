#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D fontSampler;

void main() {
    // Sample font texture with proper filtering for crisp text
    vec4 texColor = texture(fontSampler, fragTexCoord);
    
    // For text rendering: multiply vertex color by texture alpha
    // This allows colored text while using the font texture as a mask
    // The texture contains the glyph shape (alpha channel)
    // The vertex color contains the text color (RGB) and overall alpha
    outColor = vec4(fragColor.rgb, fragColor.a * texColor.a);
    
    // Discard fully transparent pixels for better performance
    if (outColor.a < 0.01) {
        discard;
    }
    
    // Ensure text is fully opaque when visible (no semi-transparent text)
    if (outColor.a > 0.01) {
        outColor.a = 1.0;
    }
}
