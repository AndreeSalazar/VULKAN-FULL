//! Engine UI - Rust FFI Library for eGUI
//! This library provides Rust bindings for eGUI to be used from C++

use std::ffi::{c_float, c_int, c_uint, c_void};
use std::sync::Mutex;

// Estructuras compatibles con C++
#[repr(C)]
pub struct EngineState {
    pub fps: c_float,
    pub frame_time: c_float,
    pub frame_count: u64,
    pub total_time: c_float,
}

#[repr(C)]
pub struct UIVertex {
    pub pos: [f32; 2],
    pub tex_coord: [f32; 2],
    pub color: u32, // RGBA packed
}

#[repr(C)]
pub struct RenderData {
    pub vertices_ptr: *const UIVertex,
    pub vertices_count: usize,
    pub indices_ptr: *const u32,
    pub indices_count: usize,
    pub textures_count: usize,
}

#[repr(C)]
pub struct FontTextureData {
    pub pixels_ptr: *const u8,
    pub width: u32,
    pub height: u32,
    pub has_data: bool,
}

// Estado interno de eGUI
struct EguiState {
    context: Option<egui::Context>,
    width: f32,
    height: f32,
    pointer_pos: Option<egui::Pos2>,
    initialized: bool,
    last_output: Option<egui::PlatformOutput>,
}

// Estado global (thread-safe)
static EGUI_STATE: Mutex<Option<EguiState>> = Mutex::new(None);

// Buffers globales para datos de renderizado
static mut RENDER_VERTICES: Vec<UIVertex> = Vec::new();
static mut RENDER_INDICES: Vec<u32> = Vec::new();

// Textura de fuente REAL de eGUI (se obtiene después del primer render)
static mut FONT_TEXTURE_DATA: Option<Vec<u8>> = None;
static mut FONT_TEXTURE_SIZE: (u32, u32) = (0, 0);

/// Inicializar eGUI
/// 
/// # Safety
/// window_handle y vulkan_instance deben ser punteros válidos
#[no_mangle]
pub unsafe extern "C" fn egui_init(_window_handle: *mut c_void, _vulkan_instance: *mut c_void) -> bool {
    let mut state = EGUI_STATE.lock().unwrap();
    
    if state.is_some() {
        return true; // Ya inicializado
    }
    
    let mut context = egui::Context::default();
    
    // ===== CONFIGURAR FUENTE PROFESIONAL Y LEGIBLE =====
    // eGUI usa fuentes profesionales por defecto (ProggyClean, etc.)
    // Estas fuentes están optimizadas para legibilidad en interfaces
    let fonts = egui::FontDefinitions::default();
    context.set_fonts(fonts);
    
    // Configurar DPI scaling para texto nítido
    // 1.0 = mapeo 1:1 pixel para máxima nitidez
    context.set_pixels_per_point(1.0);
    
    // ===== NO CREAR TEXTURA DUMMY =====
    // La textura de fuente se obtendrá automáticamente después del primer render
    // No creamos ninguna textura dummy - esperamos a la real
    FONT_TEXTURE_DATA = None;
    FONT_TEXTURE_SIZE = (0, 0);
    
    *state = Some(EguiState {
        context: Some(context),
        width: 1920.0,
        height: 1080.0,
        pointer_pos: None,
        initialized: true,
        last_output: None,
    });
    
    eprintln!("[egui_init] ✅ Contexto creado con fuentes profesionales, textura se obtendrá después del primer render");
    
    true
}

/// Actualizar tamaño de pantalla
#[no_mangle]
pub extern "C" fn egui_set_screen_size(width: c_float, height: c_float) {
    let mut state = EGUI_STATE.lock().unwrap();
    if let Some(ref mut s) = *state {
        s.width = width;
        s.height = height;
    }
}

/// Iniciar nuevo frame (llamar antes de construir UI)
#[no_mangle]
pub unsafe extern "C" fn egui_new_frame() {
    let mut state = EGUI_STATE.lock().unwrap();
    if let Some(ref s) = *state {
        if let Some(ref ctx) = s.context {
            let mut raw_input = egui::RawInput::default();
            
            let viewport_id = ctx.viewport_id();
            let viewport_info = egui::ViewportInfo {
                native_pixels_per_point: Some(1.0),
                monitor_size: Some(egui::vec2(s.width, s.height)),
                ..Default::default()
            };
            raw_input.viewports.insert(viewport_id, viewport_info);
            
            if let Some(pos) = s.pointer_pos {
                raw_input.events.push(egui::Event::PointerMoved(pos));
            }
            
            raw_input.time = Some(std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap()
                .as_secs_f64());
            
            ctx.begin_frame(raw_input);
        }
    }
}

/// Renderizar UI (genera comandos de renderizado y guarda output)
#[no_mangle]
pub unsafe extern "C" fn egui_render(_state: *const EngineState, _command_buffer: *mut c_void) -> bool {
    let mut state = EGUI_STATE.lock().unwrap();
    if let Some(ref mut s) = *state {
        if let Some(ref ctx) = s.context {
            let output = ctx.end_frame();
            
            // ===== ACTUALIZAR TEXTURA DE FUENTE REAL DE eGUI =====
            // eGUI genera la textura de fuente automáticamente cuando se renderiza
            // La obtenemos del contexto después de renderizar
            static mut FONT_TEXTURE_UPDATED: bool = false;
            
            // Intentar obtener la textura desde el output del frame (método preferido)
            if !unsafe { FONT_TEXTURE_UPDATED } {
                let mut texture_found = false;
                
                // Método 1: Buscar en textures_delta.set (texturas nuevas/actualizadas)
                if !output.textures_delta.set.is_empty() {
                    for (_id, image_delta) in &output.textures_delta.set {
                        match &image_delta.image {
                            egui::ImageData::Font(font_img) => {
                                // FontImage tiene píxeles f32 (alpha values)
                                let alpha_pixels = font_img.pixels.as_slice();
                                let font_width = font_img.width();
                                let font_height = font_img.height();
                                
                                if font_width > 0 && font_height > 0 && !alpha_pixels.is_empty() {
                                    // Convertir de f32 alpha a RGBA bytes (blanco con alpha)
                                    let mut rgba_pixels = Vec::with_capacity(alpha_pixels.len() * 4);
                                    for alpha in alpha_pixels {
                                        let a = (*alpha * 255.0).clamp(0.0, 255.0) as u8;
                                        rgba_pixels.push(255); // R
                                        rgba_pixels.push(255); // G
                                        rgba_pixels.push(255); // B
                                        rgba_pixels.push(a);   // A
                                    }
                                    
                                    unsafe {
                                        FONT_TEXTURE_DATA = Some(rgba_pixels);
                                        FONT_TEXTURE_SIZE = (font_width as u32, font_height as u32);
                                        FONT_TEXTURE_UPDATED = true;
                                    }
                                    
                                    eprintln!("[egui_render] ✅ Textura de fuente PROFESIONAL obtenida: {}x{} ({} píxeles alpha)", 
                                             font_width, font_height, alpha_pixels.len());
                                    texture_found = true;
                                    break;
                                }
                            },
                            egui::ImageData::Color(color_image) => {
                                // ColorImage también puede ser la textura de fuente en algunos casos
                                let font_pixels = color_image.pixels.as_slice();
                                let font_width = color_image.width();
                                let font_height = color_image.height();
                                
                                if font_width > 0 && font_height > 0 && !font_pixels.is_empty() {
                                    // Convertir de Color32 a RGBA bytes
                                    let mut rgba_pixels = Vec::with_capacity(font_pixels.len() * 4);
                                    for color in font_pixels {
                                        rgba_pixels.push(color.r());
                                        rgba_pixels.push(color.g());
                                        rgba_pixels.push(color.b());
                                        rgba_pixels.push(color.a());
                                    }
                                    
                                    unsafe {
                                        FONT_TEXTURE_DATA = Some(rgba_pixels);
                                        FONT_TEXTURE_SIZE = (font_width as u32, font_height as u32);
                                        FONT_TEXTURE_UPDATED = true;
                                    }
                                    
                                    eprintln!("[egui_render] ✅ Textura de fuente PROFESIONAL (Color) obtenida: {}x{} ({} píxeles)", 
                                             font_width, font_height, font_pixels.len());
                                    texture_found = true;
                                    break;
                                }
                            },
                        }
                    }
                }
                
                // Método 2: Si no se encontró en el delta, intentar obtener desde el atlas
                // Nota: En eGUI, la textura generalmente aparece en textures_delta después del primer render
                // Si no está disponible aún, esperamos al siguiente frame
                if !texture_found {
                    eprintln!("[egui_render] ⚠️ Textura de fuente no encontrada en output, esperando al siguiente frame...");
                }
            }
            
            // Limpiar buffers anteriores
            RENDER_VERTICES.clear();
            RENDER_INDICES.clear();
            
            // Tessellate las formas
            if !output.shapes.is_empty() {
                let pixels_per_point = 1.0;
                let clipped_primitives = ctx.tessellate(output.shapes.clone(), pixels_per_point);
                
                for clip_primitive in clipped_primitives {
                    match &clip_primitive.primitive {
                        egui::epaint::Primitive::Mesh(mesh) => {
                            let start_vertex_idx = RENDER_VERTICES.len() as u32;
                            let mut old_to_new_idx: Vec<Option<u32>> = vec![None; mesh.vertices.len()];
                            let mut new_idx_counter = 0u32;
                            
                            for (old_idx, v) in mesh.vertices.iter().enumerate() {
                                let color = v.color;
                                let r = color.r() as u32;
                                let g = color.g() as u32;
                                let b = color.b() as u32;
                                let a = color.a() as u32;
                                let rgba = (a << 24) | (b << 16) | (g << 8) | r;
                                
                                let screen_x = v.pos.x;
                                let screen_y = v.pos.y;
                                
                                let is_valid = screen_x.is_finite() && screen_y.is_finite() &&
                                    screen_x >= -100.0 && screen_x <= s.width + 100.0 &&
                                    screen_y >= -100.0 && screen_y <= s.height + 100.0 &&
                                    !(screen_x.abs() < 1.0 && screen_y.abs() < 1.0 && a == 0);
                                
                                if is_valid {
                                    RENDER_VERTICES.push(UIVertex {
                                        pos: [screen_x, screen_y],
                                        tex_coord: [v.uv.x, v.uv.y],
                                        color: rgba,
                                    });
                                    old_to_new_idx[old_idx] = Some(start_vertex_idx + new_idx_counter);
                                    new_idx_counter += 1;
                                }
                            }
                            
                            for tri_idx in (0..mesh.indices.len()).step_by(3) {
                                if tri_idx + 2 < mesh.indices.len() {
                                    let i0 = mesh.indices[tri_idx] as usize;
                                    let i1 = mesh.indices[tri_idx + 1] as usize;
                                    let i2 = mesh.indices[tri_idx + 2] as usize;
                                    
                                    if i0 < old_to_new_idx.len() && i1 < old_to_new_idx.len() && i2 < old_to_new_idx.len() {
                                        if let (Some(new_i0), Some(new_i1), Some(new_i2)) = 
                                            (old_to_new_idx[i0], old_to_new_idx[i1], old_to_new_idx[i2]) {
                                            RENDER_INDICES.push(new_i0);
                                            RENDER_INDICES.push(new_i1);
                                            RENDER_INDICES.push(new_i2);
                                        }
                                    }
                                }
                            }
                        }
                        _ => {}
                    }
                }
            }
            
            let has_data = !RENDER_VERTICES.is_empty();
            
            use std::sync::atomic::{AtomicU32, Ordering};
            static CALL_COUNT: AtomicU32 = AtomicU32::new(0);
            let count = CALL_COUNT.fetch_add(1, Ordering::Relaxed);
            if count < 5 {
                eprintln!("[egui_render] call {}: vertices: {}, indices: {}, has_data: {}", 
                         count,
                         RENDER_VERTICES.len(), 
                         RENDER_INDICES.len(),
                         has_data);
            }
            
            has_data
        } else {
            false
        }
    } else {
        false
    }
}

/// Obtener datos de renderizado para Vulkan
#[no_mangle]
pub unsafe extern "C" fn egui_get_render_data(render_data: *mut RenderData) -> bool {
    if RENDER_VERTICES.is_empty() {
        return false;
    }
    
    let data = &mut *render_data;
    data.vertices_ptr = RENDER_VERTICES.as_ptr();
    data.vertices_count = RENDER_VERTICES.len();
    data.indices_ptr = RENDER_INDICES.as_ptr();
    data.indices_count = RENDER_INDICES.len();
    data.textures_count = 1;
    
    true
}

/// Obtener datos de textura de fuente
#[no_mangle]
pub unsafe extern "C" fn egui_get_font_texture_data(texture_data: *mut FontTextureData) -> bool {
    if let Some(ref pixels) = FONT_TEXTURE_DATA {
        let data = &mut *texture_data;
        data.pixels_ptr = pixels.as_ptr();
        data.width = FONT_TEXTURE_SIZE.0;
        data.height = FONT_TEXTURE_SIZE.1;
        data.has_data = true;
        true
    } else {
        false
    }
}

/// Procesar eventos de mouse
#[no_mangle]
pub extern "C" fn egui_handle_mouse_event(x: c_float, y: c_float, _buttons: c_uint, _pressed: bool) {
    let mut state = EGUI_STATE.lock().unwrap();
    if let Some(ref mut s) = *state {
        let pos = egui::pos2(x, y);
        s.pointer_pos = Some(pos);
    }
}

/// Procesar eventos de teclado
#[no_mangle]
pub extern "C" fn egui_handle_key_event(_key: c_uint, _pressed: bool, _modifiers: c_uint) {
    // TODO: Implementar
}

/// Procesar eventos de input (legacy)
#[no_mangle]
pub extern "C" fn egui_handle_event(_event_type: c_uint, _x: c_float, _y: c_float) {
    // Deprecated
}

/// Limpiar recursos de eGUI
#[no_mangle]
pub extern "C" fn egui_cleanup() {
    let mut state = EGUI_STATE.lock().unwrap();
    *state = None;
}

/// Verificar si eGUI está inicializado
#[no_mangle]
pub extern "C" fn egui_is_initialized() -> bool {
    let state = EGUI_STATE.lock().unwrap();
    state.is_some() && state.as_ref().unwrap().initialized
}

/// Renderizar interfaz estilo Unreal Engine 5 - PROFESIONAL
#[no_mangle]
pub extern "C" fn egui_show_demo() {
    let state = EGUI_STATE.lock().unwrap();
    if let Some(ref s) = *state {
        if let Some(ref ctx) = s.context {
            // DPI scaling optimizado para texto nítido y legible
            // 1.0 = 1:1 pixel mapping para máxima nitidez
            ctx.set_pixels_per_point(1.0);
            
            // ========================================
            // ESTILO UE5 PROFESIONAL - Mejorado
            // ========================================
            let mut style = (*ctx.style()).clone();

            // ========================================
            // ESTILO UE5 ULTRA PROFESIONAL
            // ========================================
            // Fondos ultra oscuros con máximo contraste
            style.visuals.dark_mode = true;
            style.visuals.panel_fill = egui::Color32::from_rgb(25, 25, 25);        // Muy oscuro
            style.visuals.window_fill = egui::Color32::from_rgb(20, 20, 20);      // Casi negro
            style.visuals.extreme_bg_color = egui::Color32::from_rgb(15, 15, 15); // Muy oscuro
            style.visuals.faint_bg_color = egui::Color32::from_rgb(35, 35, 35);    // Gris medio
            style.visuals.widgets.noninteractive.bg_fill = egui::Color32::from_rgb(25, 25, 25);

            // Texto ultra brillante para máximo contraste
            style.visuals.override_text_color = Some(egui::Color32::from_rgb(255, 255, 255));

            // Azul UE5 oficial vibrante
            style.visuals.selection.bg_fill = egui::Color32::from_rgb(0, 122, 204);  // Azul UE5 oficial
            style.visuals.hyperlink_color = egui::Color32::from_rgb(0, 122, 204);
            style.visuals.widgets.active.bg_fill = egui::Color32::from_rgb(0, 122, 204);
            style.visuals.widgets.hovered.bg_fill = egui::Color32::from_rgb(45, 45, 45);
            style.visuals.widgets.inactive.bg_fill = egui::Color32::from_rgb(25, 25, 25);

            // Bordes más definidos y profesionales
            style.visuals.window_stroke = egui::Stroke::new(1.5, egui::Color32::from_rgb(55, 55, 55));
            style.visuals.widgets.noninteractive.bg_stroke = egui::Stroke::new(1.0, egui::Color32::from_rgb(45, 45, 45));
            style.visuals.widgets.inactive.bg_stroke = egui::Stroke::new(1.0, egui::Color32::from_rgb(50, 50, 50));
            style.visuals.widgets.hovered.bg_stroke = egui::Stroke::new(1.5, egui::Color32::from_rgb(0, 122, 204));
            style.visuals.widgets.active.bg_stroke = egui::Stroke::new(1.5, egui::Color32::from_rgb(0, 122, 204));

            // Espaciado compacto y profesional
            style.spacing.item_spacing = egui::vec2(4.0, 3.0);
            style.spacing.window_margin = egui::Margin::same(4.0);
            style.spacing.button_padding = egui::vec2(8.0, 4.0);
            style.spacing.menu_margin = egui::Margin::same(4.0);
            style.spacing.indent = 18.0;
            
            // Esquinas completamente rectas (estilo UE5)
            style.visuals.window_rounding = egui::Rounding::same(0.0);
            style.visuals.widgets.noninteractive.rounding = egui::Rounding::same(0.0);
            style.visuals.widgets.inactive.rounding = egui::Rounding::same(0.0);
            style.visuals.widgets.hovered.rounding = egui::Rounding::same(0.0);
            style.visuals.widgets.active.rounding = egui::Rounding::same(0.0);

            ctx.set_style(style);

            // ========================================
            // MENU BAR - Barra superior principal (PROFESIONAL)
            // ========================================
            egui::TopBottomPanel::top("menu_bar")
                .exact_height(30.0)
                .frame(egui::Frame::default()
                    .fill(egui::Color32::from_rgb(30, 30, 30))  // Fondo oscuro consistente
                    .stroke(egui::Stroke::new(1.0, egui::Color32::from_rgb(50, 50, 50)))  // Borde inferior sutil
                    .inner_margin(egui::Margin::symmetric(10.0, 0.0)))
                .show(ctx, |ui| {
                    ui.horizontal(|ui| {
                        ui.set_height(30.0);
                        ui.vertical_centered(|ui| {
                            // Logo/Icon placeholder
                            ui.label(egui::RichText::new("🎮").size(18.0));
                            ui.add_space(10.0);
                            
                            // Menús principales con mejor diseño
                            if ui.selectable_label(false, egui::RichText::new("File").size(13.0).color(egui::Color32::from_rgb(220, 220, 220))).clicked() {}
                            if ui.selectable_label(false, egui::RichText::new("Edit").size(13.0).color(egui::Color32::from_rgb(220, 220, 220))).clicked() {}
                            if ui.selectable_label(false, egui::RichText::new("View").size(13.0).color(egui::Color32::from_rgb(220, 220, 220))).clicked() {}
                            if ui.selectable_label(false, egui::RichText::new("Tools").size(13.0).color(egui::Color32::from_rgb(220, 220, 220))).clicked() {}
                            if ui.selectable_label(false, egui::RichText::new("Build").size(13.0).color(egui::Color32::from_rgb(220, 220, 220))).clicked() {}
                            if ui.selectable_label(false, egui::RichText::new("Window").size(13.0).color(egui::Color32::from_rgb(220, 220, 220))).clicked() {}
                            if ui.selectable_label(false, egui::RichText::new("Help").size(13.0).color(egui::Color32::from_rgb(220, 220, 220))).clicked() {}
                        });
                        
                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            ui.label(egui::RichText::new("Vulkan Engine v1.0").size(12.0).color(egui::Color32::from_rgb(150, 150, 150)));
                            ui.add_space(10.0);
                        });
                    });
                });

            // ========================================
            // TOOLBAR - Barra de herramientas (PROFESIONAL)
            // ========================================
            egui::TopBottomPanel::top("toolbar")
                .exact_height(38.0)
                .frame(egui::Frame::default()
                    .fill(egui::Color32::from_rgb(35, 35, 38))  // Fondo ligeramente más claro que menu bar
                    .stroke(egui::Stroke::new(1.0, egui::Color32::from_rgb(55, 55, 55)))  // Borde sutil
                    .inner_margin(egui::Margin::symmetric(10.0, 4.0)))
                .show(ctx, |ui| {
                    ui.horizontal(|ui| {
                        ui.set_height(38.0);
                        ui.vertical_centered(|ui| {
                            // Herramientas de transformación
                            ui.label(egui::RichText::new("Transform:").size(12.0).color(egui::Color32::from_rgb(180, 180, 180)));
                            ui.add_space(6.0);
                            
                            let move_btn = ui.add_sized([36.0, 32.0], 
                                egui::Button::new(egui::RichText::new("⬌").size(16.0))
                                    .fill(egui::Color32::from_rgb(0, 120, 215)));
                            if move_btn.clicked() {}
                            move_btn.on_hover_text("Move Tool (W)");
                            
                            let rotate_btn = ui.add_sized([36.0, 32.0], 
                                egui::Button::new(egui::RichText::new("↻").size(16.0)));
                            if rotate_btn.clicked() {}
                            rotate_btn.on_hover_text("Rotate Tool (E)");
                            
                            let scale_btn = ui.add_sized([36.0, 32.0], 
                                egui::Button::new(egui::RichText::new("⊞").size(16.0)));
                            if scale_btn.clicked() {}
                            scale_btn.on_hover_text("Scale Tool (R)");
                            
                            ui.add_space(8.0);
                            ui.separator();
                            ui.add_space(8.0);
                            
                            // Herramientas de snapping
                            ui.label(egui::RichText::new("Snap:").size(12.0).color(egui::Color32::from_rgb(180, 180, 180)));
                            ui.add_space(6.0);
                            
                            let grid_btn = ui.add_sized([36.0, 32.0], 
                                egui::Button::new(egui::RichText::new("⊞").size(14.0)));
                            if grid_btn.clicked() {}
                            grid_btn.on_hover_text("Grid Snap");
                            
                            let angle_btn = ui.add_sized([36.0, 32.0], 
                                egui::Button::new(egui::RichText::new("∠").size(14.0)));
                            if angle_btn.clicked() {}
                            angle_btn.on_hover_text("Angle Snap");
                            
                            ui.add_space(8.0);
                            ui.separator();
                            ui.add_space(8.0);
                            
                            // Controles de reproducción
                            ui.label(egui::RichText::new("Playback:").size(12.0).color(egui::Color32::from_rgb(180, 180, 180)));
                            ui.add_space(6.0);
                            
                            let play_btn = ui.add_sized([40.0, 32.0], 
                                egui::Button::new(egui::RichText::new("▶").size(14.0).color(egui::Color32::from_rgb(100, 220, 100))));
                            if play_btn.clicked() {}
                            play_btn.on_hover_text("Play (Alt+P)");
                            
                            let pause_btn = ui.add_sized([40.0, 32.0], 
                                egui::Button::new(egui::RichText::new("⏸").size(14.0)));
                            if pause_btn.clicked() {}
                            pause_btn.on_hover_text("Pause");
                            
                            let stop_btn = ui.add_sized([40.0, 32.0], 
                                egui::Button::new(egui::RichText::new("⏹").size(14.0).color(egui::Color32::from_rgb(220, 100, 100))));
                            if stop_btn.clicked() {}
                            stop_btn.on_hover_text("Stop");
                        });
                        
                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            // Quick settings
                            if ui.small_button("⚙").clicked() {}
                        });
                    });
                });

            // ========================================
            // STATUS BAR - Barra inferior (PROFESIONAL)
            // ========================================
            egui::TopBottomPanel::bottom("status_bar")
                .exact_height(26.0)
                .frame(egui::Frame::default()
                    .fill(egui::Color32::from_rgb(30, 30, 30))  // Fondo oscuro profesional
                    .stroke(egui::Stroke::new(1.0, egui::Color32::from_rgb(50, 50, 50)))  // Borde superior sutil
                    .inner_margin(egui::Margin::symmetric(10.0, 4.0)))
                .show(ctx, |ui| {
                    ui.horizontal(|ui| {
                        ui.set_height(26.0);
                        ui.vertical_centered(|ui| {
                            // Status indicator con mejor diseño
                            ui.horizontal(|ui| {
                                ui.add_space(4.0);
                                ui.painter().circle_filled(
                                    egui::pos2(ui.min_rect().min.x + 8.0, ui.min_rect().center().y),
                                    4.0,
                                    egui::Color32::from_rgb(100, 220, 100)
                                );
                                ui.add_space(6.0);
                                ui.label(egui::RichText::new("Ready").size(13.0).color(egui::Color32::from_rgb(220, 220, 220)));
                            });
                        });
                        
                        ui.separator();
                        ui.add_space(8.0);
                        
                        // Stats con mejor formato
                        ui.label(egui::RichText::new(format!("FPS: {:.1}", 60.0)).size(12.0).color(egui::Color32::from_rgb(200, 200, 200)));
                        ui.add_space(12.0);
                        ui.label(egui::RichText::new(format!("Vertices: {}", "1,234")).size(12.0).color(egui::Color32::from_rgb(200, 200, 200)));
                        ui.add_space(12.0);
                        ui.label(egui::RichText::new(format!("{:.0}×{:.0}", s.width, s.height)).size(12.0).color(egui::Color32::from_rgb(180, 180, 180)));
                        
                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            ui.add_space(8.0);
                            ui.label(egui::RichText::new("Vulkan Renderer").size(12.0).color(egui::Color32::from_rgb(160, 160, 160)));
                        });
                    });
                });

            // ========================================
            // SCENE OUTLINER - Panel izquierdo (PROFESIONAL)
            // ========================================
            egui::SidePanel::left("scene_outliner")
                .default_width(280.0)
                .min_width(220.0)
                .max_width(500.0)
                .resizable(true)
                .frame(egui::Frame::default()
                    .fill(egui::Color32::from_rgb(42, 42, 45))  // Fondo más oscuro y profesional
                    .stroke(egui::Stroke::new(1.0, egui::Color32::from_rgb(60, 60, 60)))  // Borde derecho sutil
                    .inner_margin(egui::Margin::same(10.0)))
                .show(ctx, |ui| {
                    // Header mejorado
                    ui.horizontal(|ui| {
                        ui.heading(egui::RichText::new("Outliner").size(16.0).strong().color(egui::Color32::from_rgb(240, 240, 240)));
                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            if ui.small_button("⚙").clicked() {}
                            if ui.small_button("+").clicked() {}
                        });
                    });
                    ui.add_space(4.0);
                    ui.separator();
                    ui.add_space(6.0);
                    
                    // Search bar mejorado
                    ui.horizontal(|ui| {
                        ui.label(egui::RichText::new("🔍").size(14.0).color(egui::Color32::from_rgb(160, 160, 160)));
                        ui.add_space(4.0);
                        let mut search = "".to_string();
                        ui.add(egui::TextEdit::singleline(&mut search)
                            .hint_text("Search objects...")
                            .desired_width(f32::INFINITY)
                            .text_color(egui::Color32::from_rgb(220, 220, 220)));
                    });
                    ui.add_space(8.0);
                    
                    // Scene hierarchy tree con mejor espaciado
                    egui::ScrollArea::vertical()
                        .auto_shrink([false; 2])
                        .show(ui, |ui| {
                            ui.push_id("scene_tree", |ui| {
                                // Root objects
                                scene_tree_node(ui, "🌍 World", true, &[
                                    ("💡 DirectionalLight", false),
                                    ("📦 Cube_001", true),
                                    ("📦 Cube_002", false),
                                    ("🎥 MainCamera", false),
                                ]);
                                
                                ui.add_space(4.0);
                                
                                scene_tree_node(ui, "🎨 Materials", false, &[
                                    ("🎨 DefaultMaterial", false),
                                    ("🎨 MetalMaterial", false),
                                ]);
                                
                                ui.add_space(4.0);
                                
                                scene_tree_node(ui, "💡 Lighting", false, &[
                                    ("💡 SkyLight", false),
                                    ("💡 PointLight_001", false),
                                ]);
                            });
                        });
                });

            // ========================================
            // DETAILS PANEL - Panel derecho (PROFESIONAL)
            // ========================================
            egui::SidePanel::right("details_panel")
                .default_width(340.0)
                .min_width(300.0)
                .max_width(600.0)
                .resizable(true)
                .frame(egui::Frame::default()
                    .fill(egui::Color32::from_rgb(42, 42, 45))  // Fondo consistente
                    .stroke(egui::Stroke::new(1.0, egui::Color32::from_rgb(60, 60, 60)))  // Borde izquierdo sutil
                    .inner_margin(egui::Margin::same(10.0)))
                .show(ctx, |ui| {
                    // Header mejorado
                    ui.horizontal(|ui| {
                        ui.heading(egui::RichText::new("Details").size(16.0).strong().color(egui::Color32::from_rgb(240, 240, 240)));
                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            if ui.small_button("⚙").clicked() {}
                        });
                    });
                    ui.add_space(4.0);
                    ui.separator();
                    ui.add_space(8.0);
                    
                    // Selected object info mejorado
                    ui.horizontal(|ui| {
                        ui.label(egui::RichText::new("📦").size(20.0));
                        ui.add_space(6.0);
                        ui.label(egui::RichText::new("Cube_001").size(15.0).strong().color(egui::Color32::from_rgb(240, 240, 240)));
                    });
                    ui.add_space(2.0);
                    ui.label(egui::RichText::new("Static Mesh").size(12.0).color(egui::Color32::from_rgb(150, 150, 150)));
                    ui.add_space(6.0);
                    ui.separator();
                    ui.add_space(4.0);
                    
                    egui::ScrollArea::vertical().show(ui, |ui| {
                        // Transform Section
                        ui.push_id("transform", |ui| {
                            collapsing_section(ui, "Transform", true, |ui| {
                                property_vector3(ui, "Location", 0.0, 0.0, 0.0);
                                ui.add_space(4.0);
                                property_vector3(ui, "Rotation", 0.0, 45.0, 0.0);
                                ui.add_space(4.0);
                                property_vector3(ui, "Scale", 1.0, 1.0, 1.0);
                            });
                        });
                        
                        ui.separator();
                        
                        // Static Mesh Section
                        ui.push_id("mesh", |ui| {
                            collapsing_section(ui, "Static Mesh", true, |ui| {
                                property_text(ui, "Mesh Asset", "SM_Cube");
                                property_text(ui, "Material", "M_DefaultMaterial");
                                ui.add_space(4.0);
                                ui.horizontal(|ui| {
                                    ui.label(egui::RichText::new("Vertices:").size(11.0).color(egui::Color32::from_rgb(160, 160, 160)));
                                    ui.label("24");
                                });
                                ui.horizontal(|ui| {
                                    ui.label(egui::RichText::new("Triangles:").size(11.0).color(egui::Color32::from_rgb(160, 160, 160)));
                                    ui.label("12");
                                });
                            });
                        });
                        
                        ui.separator();
                        
                        // Rendering Section
                        ui.push_id("rendering", |ui| {
                            collapsing_section(ui, "Rendering", false, |ui| {
                                property_checkbox(ui, "Cast Shadows", true);
                                property_checkbox(ui, "Receive Shadows", true);
                                property_checkbox(ui, "Visible", true);
                            });
                        });
                        
                        ui.separator();
                        
                        // Physics Section
                        ui.push_id("physics", |ui| {
                            collapsing_section(ui, "Physics", false, |ui| {
                                property_checkbox(ui, "Simulate Physics", false);
                                property_checkbox(ui, "Generate Overlap Events", false);
                            });
                        });
                        
                        ui.add_space(12.0);
                        
                        ui.add_space(8.0);
                        
                        // Add Component button mejorado
                        if ui.add_sized([ui.available_width(), 28.0],
                            egui::Button::new(egui::RichText::new("+ Add Component").size(12.0).color(egui::Color32::from_rgb(0, 120, 215))))
                            .clicked() {}
                    });
                });

            // ========================================
            // CONTENT BROWSER - Panel inferior (PROFESIONAL)
            // ========================================
            egui::TopBottomPanel::bottom("content_browser")
                .default_height(240.0)
                .min_height(180.0)
                .max_height(500.0)
                .resizable(true)
                .frame(egui::Frame::default()
                    .fill(egui::Color32::from_rgb(42, 42, 45))  // Fondo consistente
                    .stroke(egui::Stroke::new(1.0, egui::Color32::from_rgb(60, 60, 60)))  // Borde superior sutil
                    .inner_margin(egui::Margin::same(10.0)))
                .show(ctx, |ui| {
                    // Header mejorado
                    ui.horizontal(|ui| {
                        ui.heading(egui::RichText::new("Content Browser").size(16.0).strong().color(egui::Color32::from_rgb(240, 240, 240)));
                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            if ui.button(egui::RichText::new("+ Import").size(12.0)).clicked() {}
                            ui.add_space(4.0);
                            if ui.small_button("⊞").clicked() {} // Grid/List toggle
                            if ui.small_button("≡").clicked() {} // View options
                        });
                    });
                    ui.add_space(4.0);
                    ui.separator();
                    ui.add_space(6.0);
                    
                    // Navigation toolbar mejorado
                    ui.horizontal(|ui| {
                        if ui.small_button("◄").clicked() {}
                        if ui.small_button("►").clicked() {}
                        if ui.small_button("▲").clicked() {}
                        ui.add_space(8.0);
                        ui.separator();
                        ui.add_space(8.0);
                        
                        // Breadcrumb path mejorado
                        ui.label(egui::RichText::new("📁").size(14.0));
                        ui.add_space(4.0);
                        ui.selectable_label(false, egui::RichText::new("Content").size(12.0));
                        ui.label(egui::RichText::new("›").size(12.0).color(egui::Color32::from_rgb(140, 140, 140)));
                        ui.add_space(2.0);
                        ui.selectable_label(false, egui::RichText::new("Assets").size(12.0));
                        
                        ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                            ui.add_space(8.0);
                            let mut filter = "".to_string();
                            ui.add(egui::TextEdit::singleline(&mut filter)
                                .hint_text("🔍 Filter...")
                                .desired_width(150.0)
                                .text_color(egui::Color32::from_rgb(220, 220, 220)));
                        });
                    });
                    ui.add_space(6.0);
                    ui.separator();
                    ui.add_space(6.0);
                    
                    // Folder tree (left) + Asset grid (right)
                    ui.horizontal(|ui| {
                        // Folder tree mejorado
                        ui.vertical(|ui| {
                            ui.set_width(180.0);
                            egui::ScrollArea::vertical()
                                .auto_shrink([false; 2])
                                .show(ui, |ui| {
                                    folder_tree_node(ui, "📁 Content", true, &[
                                        ("📁 Meshes", false),
                                        ("📁 Materials", false),
                                        ("📁 Textures", false),
                                        ("📁 Blueprints", false),
                                        ("📁 Audio", false),
                                    ]);
                                });
                        });
                        
                        ui.add_space(8.0);
                        ui.separator();
                        ui.add_space(8.0);
                        
                        // Asset grid
                        egui::ScrollArea::vertical().show(ui, |ui| {
                            ui.set_min_width(ui.available_width());
                            
                            // Grid layout for assets
                            let item_size = 90.0;
                            let spacing = 12.0;
                            let available_width = ui.available_width();
                            let columns = (available_width / (item_size + spacing)).floor().max(1.0) as usize;
                            
                            // Sample assets
                            let assets = [
                                ("📦", "SM_Cube", "Static Mesh"),
                                ("📦", "SM_Sphere", "Static Mesh"),
                                ("🎨", "M_Default", "Material"),
                                ("🎨", "M_Metal", "Material"),
                                ("🖼", "T_Grid", "Texture"),
                                ("🖼", "T_Normal", "Texture"),
                            ];
                            
                            ui.horizontal_wrapped(|ui| {
                                for (icon, name, asset_type) in &assets {
                                    asset_thumbnail(ui, icon, name, asset_type, item_size);
                                }
                            });
                            
                            // Empty state if no assets
                            if assets.is_empty() {
                                ui.vertical_centered(|ui| {
                                    ui.add_space(30.0);
                                    ui.label(egui::RichText::new("📂 No assets found").size(13.0).color(egui::Color32::from_rgb(140, 140, 140)));
                                    ui.add_space(8.0);
                                    ui.label(egui::RichText::new("Import assets to get started").size(11.0).color(egui::Color32::from_rgb(100, 100, 100)));
                                });
                            }
                        });
                    });
                });

            // ========================================
            // VIEWPORT CENTRAL - Área 3D principal
            // ========================================
            egui::CentralPanel::default()
                .frame(egui::Frame::none().fill(egui::Color32::from_rgb(28, 28, 30)))
                .show(ctx, |ui| {
                    let rect = ui.max_rect();
                    
                    // === Viewport Controls (esquina superior derecha) ===
                    let controls_w = 180.0;
                    let controls_h = 120.0;
                    let controls_rect = egui::Rect::from_min_size(
                        egui::pos2(rect.max.x - controls_w - 16.0, rect.min.y + 16.0),
                        egui::vec2(controls_w, controls_h)
                    );
                    
                    ui.allocate_ui_at_rect(controls_rect, |ui| {
                        egui::Frame::default()
                            .fill(egui::Color32::from_rgb(30, 30, 32).linear_multiply(0.98))  // Fondo más oscuro y sutil
                            .rounding(egui::Rounding::same(4.0))  // Bordes menos redondeados
                            .stroke(egui::Stroke::new(1.0, egui::Color32::from_rgb(60, 60, 60)))  // Borde sutil
                            .inner_margin(egui::Margin::same(10.0))
                            .show(ui, |ui| {
                                ui.vertical(|ui| {
                                    ui.label(egui::RichText::new("View Mode").size(12.0).strong().color(egui::Color32::from_rgb(240, 240, 240)));
                                    ui.add_space(6.0);
                                    
                                    if ui.selectable_label(true, egui::RichText::new("● Perspective").size(11.0).color(egui::Color32::from_rgb(220, 220, 220))).clicked() {}
                                    if ui.selectable_label(false, egui::RichText::new("○ Top").size(11.0).color(egui::Color32::from_rgb(180, 180, 180))).clicked() {}
                                    if ui.selectable_label(false, egui::RichText::new("○ Front").size(11.0).color(egui::Color32::from_rgb(180, 180, 180))).clicked() {}
                                    if ui.selectable_label(false, egui::RichText::new("○ Side").size(11.0).color(egui::Color32::from_rgb(180, 180, 180))).clicked() {}
                                    
                                    ui.add_space(8.0);
                                    ui.separator();
                                    ui.add_space(4.0);
                                    
                                    ui.label(egui::RichText::new("Shading").size(12.0).strong().color(egui::Color32::from_rgb(240, 240, 240)));
                                    ui.add_space(4.0);
                                    ui.horizontal(|ui| {
                                        if ui.small_button("Lit").clicked() {}
                                        if ui.small_button("Unlit").clicked() {}
                                        if ui.small_button("Wire").clicked() {}
                                    });
                                });
                            });
                    });
                    
                    // === Info Overlay (esquina superior izquierda) ===
                    let info_w = 220.0;
                    let info_h = 100.0;
                    let info_rect = egui::Rect::from_min_size(
                        rect.min + egui::vec2(16.0, 16.0),
                        egui::vec2(info_w, info_h)
                    );
                    
                    ui.allocate_ui_at_rect(info_rect, |ui| {
                        egui::Frame::default()
                            .fill(egui::Color32::from_rgb(30, 30, 32).linear_multiply(0.98))  // Fondo más oscuro y sutil
                            .rounding(egui::Rounding::same(4.0))  // Bordes menos redondeados
                            .stroke(egui::Stroke::new(1.0, egui::Color32::from_rgb(0, 120, 215)))  // Borde azul UE5 más sutil
                            .inner_margin(egui::Margin::same(10.0))
                            .show(ui, |ui| {
                                ui.vertical(|ui| {
                                    ui.label(egui::RichText::new("3D Viewport").size(14.0).strong().color(egui::Color32::from_rgb(240, 240, 240)));
                                    ui.add_space(4.0);
                                    ui.label(egui::RichText::new("Perspective View").size(11.0).color(egui::Color32::from_rgb(200, 200, 200)));
                                    ui.add_space(2.0);
                                    ui.label(egui::RichText::new(format!("{:.0} × {:.0}", rect.width(), rect.height())).size(10.0).color(egui::Color32::from_rgb(160, 160, 160)));
                                    ui.add_space(4.0);
                                    ui.label(egui::RichText::new("1 object selected").size(10.0).color(egui::Color32::from_rgb(100, 220, 100)));
                                });
                            });
                    });
                    
                    // === Grid indicator (bottom left) - MEJORADO ===
                    let grid_text_pos = egui::pos2(rect.min.x + 12.0, rect.max.y - 12.0);
                    ui.painter().text(
                        grid_text_pos,
                        egui::Align2::LEFT_BOTTOM,
                        "Grid: 1.0 units",
                        egui::FontId::proportional(10.0),
                        egui::Color32::from_rgb(150, 150, 150)  // Color más visible
                    );
                });
        }
    }
}

// ========================================
// HELPER FUNCTIONS - UI Components
// ========================================

/// Scene tree node with icon and children (MEJORADO)
fn scene_tree_node(ui: &mut egui::Ui, label: &str, expanded: bool, children: &[(&str, bool)]) {
    let id = ui.make_persistent_id(label);
    egui::collapsing_header::CollapsingState::load_with_default_open(ui.ctx(), id, expanded)
        .show_header(ui, |ui| {
            ui.label(egui::RichText::new(label).size(13.0).color(egui::Color32::from_rgb(220, 220, 220)));
        })
        .body(|ui| {
            for (child_label, selected) in children {
                ui.horizontal(|ui| {
                    ui.add_space(20.0);
                    let text_color = if *selected {
                        egui::Color32::from_rgb(0, 120, 215)  // Azul UE5 para seleccionado
                    } else {
                        egui::Color32::from_rgb(200, 200, 200)  // Gris claro para no seleccionado
                    };
                    let response = ui.selectable_label(*selected, egui::RichText::new(*child_label).size(13.0).color(text_color));
                    if response.clicked() {
                        // Handle selection
                    }
                });
            }
        });
}

/// Folder tree node (MEJORADO)
fn folder_tree_node(ui: &mut egui::Ui, label: &str, expanded: bool, children: &[(&str, bool)]) {
    let id = ui.make_persistent_id(label);
    egui::collapsing_header::CollapsingState::load_with_default_open(ui.ctx(), id, expanded)
        .show_header(ui, |ui| {
            ui.label(egui::RichText::new(label).size(12.0).color(egui::Color32::from_rgb(220, 220, 220)));
        })
        .body(|ui| {
            for (child_label, _selected) in children {
                ui.horizontal(|ui| {
                    ui.add_space(16.0);
                    ui.selectable_label(false, egui::RichText::new(*child_label).size(12.0).color(egui::Color32::from_rgb(200, 200, 200)));
                });
            }
        });
}

/// Collapsing section for properties (MEJORADO)
fn collapsing_section<R>(
    ui: &mut egui::Ui,
    title: &str,
    default_open: bool,
    add_contents: impl FnOnce(&mut egui::Ui) -> R,
) {
    let id = ui.make_persistent_id(title);
    egui::collapsing_header::CollapsingState::load_with_default_open(ui.ctx(), id, default_open)
        .show_header(ui, |ui| {
            ui.label(egui::RichText::new(title).size(13.0).strong().color(egui::Color32::from_rgb(240, 240, 240)));
        })
        .body(|ui| {
            ui.add_space(8.0);
            add_contents(ui);
            ui.add_space(6.0);
        });
}

/// Vector3 property editor (MEJORADO)
fn property_vector3(ui: &mut egui::Ui, label: &str, x: f32, y: f32, z: f32) {
    ui.label(egui::RichText::new(label).size(12.0).color(egui::Color32::from_rgb(180, 180, 180)));
    ui.add_space(2.0);
    ui.horizontal(|ui| {
        ui.label(egui::RichText::new("X").size(11.0).color(egui::Color32::from_rgb(220, 100, 100)));
        let mut x_str = format!("{:.3}", x);
        ui.add(egui::TextEdit::singleline(&mut x_str)
            .desired_width(70.0)
            .text_color(egui::Color32::from_rgb(220, 220, 220)));
        
        ui.add_space(4.0);
        ui.label(egui::RichText::new("Y").size(11.0).color(egui::Color32::from_rgb(100, 220, 100)));
        let mut y_str = format!("{:.3}", y);
        ui.add(egui::TextEdit::singleline(&mut y_str)
            .desired_width(70.0)
            .text_color(egui::Color32::from_rgb(220, 220, 220)));
        
        ui.add_space(4.0);
        ui.label(egui::RichText::new("Z").size(11.0).color(egui::Color32::from_rgb(100, 150, 220)));
        let mut z_str = format!("{:.3}", z);
        ui.add(egui::TextEdit::singleline(&mut z_str)
            .desired_width(70.0)
            .text_color(egui::Color32::from_rgb(220, 220, 220)));
    });
}

/// Text property (MEJORADO)
fn property_text(ui: &mut egui::Ui, label: &str, value: &str) {
    ui.horizontal(|ui| {
        ui.label(egui::RichText::new(format!("{}:", label)).size(11.0).color(egui::Color32::from_rgb(180, 180, 180)));
        ui.add_space(8.0);
        ui.label(egui::RichText::new(value).size(11.0).color(egui::Color32::from_rgb(220, 220, 220)));
    });
}

/// Checkbox property (MEJORADO)
fn property_checkbox(ui: &mut egui::Ui, label: &str, mut value: bool) {
    ui.horizontal(|ui| {
        ui.checkbox(&mut value, "");
        ui.add_space(4.0);
        ui.label(egui::RichText::new(label).size(11.0).color(egui::Color32::from_rgb(220, 220, 220)));
    });
}

/// Asset thumbnail in content browser
fn asset_thumbnail(ui: &mut egui::Ui, icon: &str, name: &str, asset_type: &str, size: f32) {
    ui.vertical(|ui| {
        ui.set_width(size);
        
        let (rect, response) = ui.allocate_exact_size(
            egui::vec2(size, size),
            egui::Sense::click()
        );
        
        // Background
        let fill_color = if response.hovered() {
            egui::Color32::from_rgb(62, 62, 66)
        } else {
            egui::Color32::from_rgb(52, 52, 55)
        };
        
        ui.painter().rect_filled(
            rect,
            4.0,
            fill_color
        );
        
        ui.painter().rect_stroke(
            rect,
            4.0,
            egui::Stroke::new(1.0, egui::Color32::from_rgb(70, 70, 70))
        );
        
        // Icon
        ui.painter().text(
            rect.center(),
            egui::Align2::CENTER_CENTER,
            icon,
            egui::FontId::proportional(32.0),
            egui::Color32::from_rgb(200, 200, 200)
        );
        
        // Label mejorado
        ui.add_space(6.0);
        ui.label(egui::RichText::new(name).size(11.0).color(egui::Color32::from_rgb(230, 230, 230)));
        ui.label(egui::RichText::new(asset_type).size(10.0).color(egui::Color32::from_rgb(150, 150, 150)));
    });
}
