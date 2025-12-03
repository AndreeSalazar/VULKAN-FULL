//! FFI Header - Generado para C++
//! Este header define las estructuras y funciones FFI desde Rust

#ifndef ENGINE_UI_FFI_H
#define ENGINE_UI_FFI_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Estado del motor (compartido con C++)
struct EngineState {
    float fps;
    float frame_time;
    uint64_t frame_count;
    float total_time;
};

// Estructura de vértice UI (compatible con Rust)
struct UIVertex {
    float pos[2];
    float tex_coord[2];
    uint32_t color; // RGBA packed
};

// Estructura para datos de renderizado
struct RenderData {
    const struct UIVertex* vertices_ptr;
    size_t vertices_count;
    const uint32_t* indices_ptr;
    size_t indices_count;
    size_t textures_count;
};

// Estructura para datos de textura de fuente
struct FontTextureData {
    const uint8_t* pixels_ptr;
    uint32_t width;
    uint32_t height;
    bool has_data;
};

// Funciones FFI desde Rust
bool egui_init(void* window_handle, void* vulkan_instance);
bool egui_render(const struct EngineState* state, void* command_buffer);
bool egui_get_render_data(struct RenderData* render_data);
bool egui_get_font_texture_data(struct FontTextureData* texture_data);
void egui_handle_event(unsigned int event_type, float x, float y);
void egui_handle_mouse_event(float x, float y, unsigned int buttons, bool pressed);
void egui_handle_key_event(unsigned int key, bool pressed, unsigned int modifiers);
void egui_new_frame();
void egui_set_screen_size(float width, float height);
void egui_show_demo();
void egui_cleanup();
bool egui_is_initialized();

#ifdef __cplusplus
}
#endif

#endif // ENGINE_UI_FFI_H

