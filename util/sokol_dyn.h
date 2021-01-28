#if defined(SOKOL_IMPL) && !defined(SOKOL_DYN_IMPL)
#define SOKOL_DYN_IMPL
#endif
#ifndef SOKOL_DYN_INCLUDED
/*
    sokol_dyn.h -- dynamic library loader for sokol libraries

    Project URL: https://github.com/floooh/sokol

    Do this:
        #define SOKOL_IMPL or
        #define SOKOL_DYN_IMPL
    before you include this file in *one* C or C++ file to create the
    implementation.

    FEATURE OVERVIEW
    ================
    sokol_dyn.h is a "drop-in" solution to allow dynamic loading of runtime
    libraries which implement sokol_app, sokol_gfx and sokol_glue functions.

    STEP BY STEP
    ============
    --- Compile sokol_app, sokol_gfx (and optionally, sokol_glue) as a shared
        library, using the backend of your choice:

            #define SOKOL_IMPL
            #define SOKOL_DLL
            #define SOKOL_NO_ENTRY

            #define SOKOL_D3D11

            #include <sokol_app.h>
            #include <sokol_gfx.h>
            #include <sokol_glue.h>

    --- In your application, implement sokol_dyn AFTER including headers for
        those libraries compiled above:

            #define SOKOL_NO_ENTRY

            #include <sokol_app.h>
            #include <sokol_gfx.h>
            #include <sokol_glue.h>

            #define SOKOL_IMPL
            #include <util/sokol_dyn.h>

        DO NOT define SOKOL_DLL here. sokol_dyn implements statically linked
        wrappers which call their respective DLL counterparts through function
        pointers obtained at load time.

    --- As a last step, call sdyn_load() with the path/name of your shared
        library, before calling any sokol_app or sokol_gfx functions.

            sdyn_load("sokol-dll");

            sapp_desc desc = {
                // ...
            };

            sapp_run(&desc);


    LICENSE
    =======
    zlib/libpng license

    Copyright (c) 2018 Andre Weissflog

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.

        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.

        3. This notice may not be removed or altered from any source
        distribution.
*/
#define SOKOL_DYN_INCLUDED

#if !defined(SOKOL_APP_INCLUDED)
#error "Please include sokol_app.h before sokol_dyn.h"
#endif
#if !defined(SOKOL_GFX_INCLUDED)
#error "Please include sokol_gfx.h before sokol_dyn.h"
#endif
#if !defined(SOKOL_GLUE_INCLUDED) && !defined(SOKOL_DYN_NO_GLUE)
#error "Please include sokol_glue.h before sokol_dyn.h"
#endif

#if defined(SOKOL_APP_IMPL)
#error "Do *NOT* implement sokol_app.h in this translation unit"
#endif
#if defined(SOKOL_GFX_IMPL)
#error "Do *NOT* implement sokol_gfx.h in this translation unit"
#endif
#if defined(SOKOL_GLUE_IMPL) && !defined(SOKOL_DYN_NO_GLUE)
#error "Do *NOT* implement sokol_glue.h in this translation unit"
#endif

#if defined(SOKOL_DLL)
#error "Wait, what? I know, it's DLLs all the way down, but come on!"
#endif

#define SOKOL_DYN_API_DECL extern

#ifdef __cplusplus
extern "C" {
#endif

SOKOL_DYN_API_DECL void sdyn_load(const char* library_name);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // SOKOL_DYN_INCLUDED

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef SOKOL_DYN_IMPL
#define SOKOL_DYN_IMPL_INCLUDED (1)

#if defined(_WIN32)
    #define _SDYN_WIN32 (1)
#else
#error "sokol_dyn.h: Unknown platform"
#endif

#ifndef SOKOL_API_IMPL
    #define SOKOL_API_IMPL
#endif
#ifndef SOKOL_DEBUG
    #ifndef NDEBUG
        #define SOKOL_DEBUG (1)
    #endif
#endif
#ifndef SOKOL_ASSERT
    #include <assert.h>
    #define SOKOL_ASSERT(c) assert(c)
#endif
#ifndef SOKOL_LOG
    #ifdef SOKOL_DEBUG
        #include <stdio.h>
        #define SOKOL_LOG(s) { SOKOL_ASSERT(s); puts(s); }
    #else
        #define SOKOL_LOG(s)
    #endif
#endif
#ifndef SOKOL_ABORT
    #include <stdlib.h>
    #define SOKOL_ABORT() abort()
#endif

/*== PLATFORM SPECIFIC INCLUDES AND DEFINES ==================================*/
#if defined(_SDYN_WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifdef _MSC_VER
        #pragma warning(push)
        #pragma warning(disable: 4098)   /* void function returning a value */
    #endif
    #include <windows.h>
    #include <shellapi.h>
#endif

/*== COMMON DECLARATIONS =====================================================*/

#define _SDYN_APP_FUNCS \
    _SDYN_XMACRO(sapp_isvalid, bool, (void), ()); \
    _SDYN_XMACRO(sapp_width, int, (void), ()); \
    _SDYN_XMACRO(sapp_height, int, (void), ()); \
    _SDYN_XMACRO(sapp_color_format, int, (void), ()); \
    _SDYN_XMACRO(sapp_depth_format, int, (void), ()); \
    _SDYN_XMACRO(sapp_sample_count, int, (void), ()); \
    _SDYN_XMACRO(sapp_high_dpi, bool, (void), ()); \
    _SDYN_XMACRO(sapp_dpi_scale, float, (void), ()); \
    _SDYN_XMACRO(sapp_show_keyboard, void, (bool show), (show)); \
    _SDYN_XMACRO(sapp_keyboard_shown, bool, (void), ()); \
    _SDYN_XMACRO(sapp_is_fullscreen, bool, (void), ()); \
    _SDYN_XMACRO(sapp_toggle_fullscreen, void, (void), ()); \
    _SDYN_XMACRO(sapp_show_mouse, void, (bool show), (show)); \
    _SDYN_XMACRO(sapp_mouse_shown, bool, (void), ()); \
    _SDYN_XMACRO(sapp_lock_mouse, void, (bool lock), (lock)); \
    _SDYN_XMACRO(sapp_mouse_locked, bool, (void), ()); \
    _SDYN_XMACRO(sapp_userdata, void*, (void), ()); \
    _SDYN_XMACRO(sapp_query_desc, sapp_desc, (void), ()); \
    _SDYN_XMACRO(sapp_request_quit, void, (void), ()); \
    _SDYN_XMACRO(sapp_cancel_quit, void, (void), ()); \
    _SDYN_XMACRO(sapp_quit, void, (void), ()); \
    _SDYN_XMACRO(sapp_consume_event, void, (void), ()); \
    _SDYN_XMACRO(sapp_frame_count, uint64_t, (void), ()); \
    _SDYN_XMACRO(sapp_set_clipboard_string, void, (const char* str), (str)); \
    _SDYN_XMACRO(sapp_get_clipboard_string, const char*, (void), ()); \
    _SDYN_XMACRO(sapp_set_window_title, void, (const char* str), (str)); \
    _SDYN_XMACRO(sapp_get_num_dropped_files, int, (void), ()); \
    _SDYN_XMACRO(sapp_get_dropped_file_path, const char*, (int index), (index)); \
    _SDYN_XMACRO(sapp_run, void, (const sapp_desc* desc), (desc)); \
    _SDYN_XMACRO(sapp_gles2, bool, (void), ()); \
    _SDYN_XMACRO(sapp_html5_ask_leave_site, void, (bool ask), (ask)); \
    _SDYN_XMACRO(sapp_html5_get_dropped_file_size, uint32_t, (int index), (index)); \
    _SDYN_XMACRO(sapp_html5_fetch_dropped_file, void, (const sapp_html5_fetch_request* request), (request)); \
    _SDYN_XMACRO(sapp_metal_get_device, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_metal_get_renderpass_descriptor, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_metal_get_drawable, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_macos_get_window, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_ios_get_window, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_d3d11_get_device, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_d3d11_get_device_context, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_d3d11_get_render_target_view, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_d3d11_get_depth_stencil_view, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_win32_get_hwnd, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_wgpu_get_device, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_wgpu_get_render_view, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_wgpu_get_resolve_view, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_wgpu_get_depth_stencil_view, const void*, (void), ()); \
    _SDYN_XMACRO(sapp_android_get_native_activity, const void*, (void), ());

#define _SDYN_GFX_FUNCS \
    _SDYN_XMACRO(sg_setup, void, (const sg_desc* desc), (desc)); \
    _SDYN_XMACRO(sg_shutdown, void, (void), ()); \
    _SDYN_XMACRO(sg_isvalid, bool, (void), ()); \
    _SDYN_XMACRO(sg_reset_state_cache, void, (void), ()); \
    _SDYN_XMACRO(sg_install_trace_hooks, sg_trace_hooks, (const sg_trace_hooks* trace_hooks), (trace_hooks)); \
    _SDYN_XMACRO(sg_push_debug_group, void, (const char* name), (name)); \
    _SDYN_XMACRO(sg_pop_debug_group, void, (void), ()); \
    _SDYN_XMACRO(sg_make_buffer, sg_buffer, (const sg_buffer_desc* desc), (desc)); \
    _SDYN_XMACRO(sg_make_image, sg_image, (const sg_image_desc* desc), (desc)); \
    _SDYN_XMACRO(sg_make_shader, sg_shader, (const sg_shader_desc* desc), (desc)); \
    _SDYN_XMACRO(sg_make_pipeline, sg_pipeline, (const sg_pipeline_desc* desc), (desc)); \
    _SDYN_XMACRO(sg_make_pass, sg_pass, (const sg_pass_desc* desc), (desc)); \
    _SDYN_XMACRO(sg_destroy_buffer, void, (sg_buffer buf), (buf)); \
    _SDYN_XMACRO(sg_destroy_image, void, (sg_image img), (img)); \
    _SDYN_XMACRO(sg_destroy_shader, void, (sg_shader shd), (shd)); \
    _SDYN_XMACRO(sg_destroy_pipeline, void, (sg_pipeline pip), (pip)); \
    _SDYN_XMACRO(sg_destroy_pass, void, (sg_pass pass), (pass)); \
    _SDYN_XMACRO(sg_update_buffer, void, (sg_buffer buf, const void* data_ptr, int data_size), (buf, data_ptr, data_size)); \
    _SDYN_XMACRO(sg_update_image, void, (sg_image img, const sg_image_content* data), (img, data)); \
    _SDYN_XMACRO(sg_append_buffer, int, (sg_buffer buf, const void* data_ptr, int data_size), (buf, data_ptr, data_size)); \
    _SDYN_XMACRO(sg_query_buffer_overflow, bool, (sg_buffer buf), (buf)); \
    _SDYN_XMACRO(sg_begin_default_pass, void, (const sg_pass_action* pass_action, int width, int height), (pass_action, width, height)); \
    _SDYN_XMACRO(sg_begin_pass, void, (sg_pass pass, const sg_pass_action* pass_action), (pass, pass_action)); \
    _SDYN_XMACRO(sg_apply_viewport, void, (int x, int y, int width, int height, bool origin_top_left), (x, y, width, height, origin_top_left)); \
    _SDYN_XMACRO(sg_apply_scissor_rect, void, (int x, int y, int width, int height, bool origin_top_left), (x, y, width, height, origin_top_left)); \
    _SDYN_XMACRO(sg_apply_pipeline, void, (sg_pipeline pip), (pip)); \
    _SDYN_XMACRO(sg_apply_bindings, void, (const sg_bindings* bindings), (bindings)); \
    _SDYN_XMACRO(sg_apply_uniforms, void, (sg_shader_stage stage, int ub_index, const void* data, int num_bytes), (stage, ub_index, data, num_bytes)); \
    _SDYN_XMACRO(sg_draw, void, (int base_element, int num_elements, int num_instances), (base_element, num_elements, num_instances)); \
    _SDYN_XMACRO(sg_end_pass, void, (void), ()); \
    _SDYN_XMACRO(sg_commit, void, (void), ()); \
    _SDYN_XMACRO(sg_query_desc, sg_desc, (void), ()); \
    _SDYN_XMACRO(sg_query_backend, sg_backend, (void), ()); \
    _SDYN_XMACRO(sg_query_features, sg_features, (void), ()); \
    _SDYN_XMACRO(sg_query_limits, sg_limits, (void), ()); \
    _SDYN_XMACRO(sg_query_pixelformat, sg_pixelformat_info, (sg_pixel_format fmt), (fmt)); \
    _SDYN_XMACRO(sg_query_buffer_state, sg_resource_state, (sg_buffer buf), (buf)); \
    _SDYN_XMACRO(sg_query_image_state, sg_resource_state, (sg_image img), (img)); \
    _SDYN_XMACRO(sg_query_shader_state, sg_resource_state, (sg_shader shd), (shd)); \
    _SDYN_XMACRO(sg_query_pipeline_state, sg_resource_state, (sg_pipeline pip), (pip)); \
    _SDYN_XMACRO(sg_query_pass_state, sg_resource_state, (sg_pass pass), (pass)); \
    _SDYN_XMACRO(sg_query_buffer_info, sg_buffer_info, (sg_buffer buf), (buf)); \
    _SDYN_XMACRO(sg_query_image_info, sg_image_info, (sg_image img), (img)); \
    _SDYN_XMACRO(sg_query_shader_info, sg_shader_info, (sg_shader shd), (shd)); \
    _SDYN_XMACRO(sg_query_pipeline_info, sg_pipeline_info, (sg_pipeline pip), (pip)); \
    _SDYN_XMACRO(sg_query_pass_info, sg_pass_info, (sg_pass pass), (pass)); \
    _SDYN_XMACRO(sg_query_buffer_defaults, sg_buffer_desc, (const sg_buffer_desc* desc), (desc)); \
    _SDYN_XMACRO(sg_query_image_defaults, sg_image_desc, (const sg_image_desc* desc), (desc)); \
    _SDYN_XMACRO(sg_query_shader_defaults, sg_shader_desc, (const sg_shader_desc* desc), (desc)); \
    _SDYN_XMACRO(sg_query_pipeline_defaults, sg_pipeline_desc, (const sg_pipeline_desc* desc), (desc)); \
    _SDYN_XMACRO(sg_query_pass_defaults, sg_pass_desc, (const sg_pass_desc* desc), (desc)); \
    _SDYN_XMACRO(sg_alloc_buffer, sg_buffer, (void), ()); \
    _SDYN_XMACRO(sg_alloc_image, sg_image, (void), ()); \
    _SDYN_XMACRO(sg_alloc_shader, sg_shader, (void), ()); \
    _SDYN_XMACRO(sg_alloc_pipeline, sg_pipeline, (void), ()); \
    _SDYN_XMACRO(sg_alloc_pass, sg_pass, (void), ()); \
    _SDYN_XMACRO(sg_dealloc_buffer, void, (sg_buffer buf_id), (buf_id)); \
    _SDYN_XMACRO(sg_dealloc_image, void, (sg_image img_id), (img_id)); \
    _SDYN_XMACRO(sg_dealloc_shader, void, (sg_shader shd_id), (shd_id)); \
    _SDYN_XMACRO(sg_dealloc_pipeline, void, (sg_pipeline pip_id), (pip_id)); \
    _SDYN_XMACRO(sg_dealloc_pass, void, (sg_pass pass_id), (pass_id)); \
    _SDYN_XMACRO(sg_init_buffer, void, (sg_buffer buf_id, const sg_buffer_desc* desc), (buf_id, desc)); \
    _SDYN_XMACRO(sg_init_image, void, (sg_image img_id, const sg_image_desc* desc), (img_id, desc)); \
    _SDYN_XMACRO(sg_init_shader, void, (sg_shader shd_id, const sg_shader_desc* desc), (shd_id, desc)); \
    _SDYN_XMACRO(sg_init_pipeline, void, (sg_pipeline pip_id, const sg_pipeline_desc* desc), (pip_id, desc)); \
    _SDYN_XMACRO(sg_init_pass, void, (sg_pass pass_id, const sg_pass_desc* desc), (pass_id, desc)); \
    _SDYN_XMACRO(sg_uninit_buffer, bool, (sg_buffer buf_id), (buf_id)); \
    _SDYN_XMACRO(sg_uninit_image, bool, (sg_image img_id), (img_id)); \
    _SDYN_XMACRO(sg_uninit_shader, bool, (sg_shader shd_id), (shd_id)); \
    _SDYN_XMACRO(sg_uninit_pipeline, bool, (sg_pipeline pip_id), (pip_id)); \
    _SDYN_XMACRO(sg_uninit_pass, bool, (sg_pass pass_id), (pass_id)); \
    _SDYN_XMACRO(sg_fail_buffer, void, (sg_buffer buf_id), (buf_id)); \
    _SDYN_XMACRO(sg_fail_image, void, (sg_image img_id), (img_id)); \
    _SDYN_XMACRO(sg_fail_shader, void, (sg_shader shd_id), (shd_id)); \
    _SDYN_XMACRO(sg_fail_pipeline, void, (sg_pipeline pip_id), (pip_id)); \
    _SDYN_XMACRO(sg_fail_pass, void, (sg_pass pass_id), (pass_id)); \
    _SDYN_XMACRO(sg_setup_context, sg_context, (void), ()); \
    _SDYN_XMACRO(sg_activate_context, void, (sg_context ctx_id), (ctx_id)); \
    _SDYN_XMACRO(sg_discard_context, void, (sg_context ctx_id), (ctx_id)); \
    _SDYN_XMACRO(sg_d3d11_device, const void*, (void), ()); \
    _SDYN_XMACRO(sg_mtl_device, const void*, (void), ()); \
    _SDYN_XMACRO(sg_mtl_render_command_encoder, const void*, (void), ());

#if !defined(SOKOL_DYN_NO_GLUE)
#define _SDYN_GLUE_FUNCS \
    _SDYN_XMACRO(sapp_sgcontext, sg_context_desc, (void), ());
#else
#define _SDYN_GLUE_FUNCS
#endif

#define _SDYN_XMACRO(name, ret, params, args) \
    typedef ret (__stdcall * PFN_ ## name) params; \
    static PFN_ ## name pfn_ ## name; \
    extern ret name params { return pfn_ ## name args; }

_SDYN_APP_FUNCS
_SDYN_GFX_FUNCS
_SDYN_GLUE_FUNCS

#undef _SDYN_XMACRO

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

/*=== PRIVATE HELPER FUNCTIONS ===============================================*/

static void _sdyn_fail(const char* msg) {
    SOKOL_ASSERT(msg);
    SOKOL_ABORT();
}

/*== WINDOWS DESKTOP =========================================================*/
#if defined(_SDYN_WIN32)

SOKOL_API_IMPL void sdyn_load(const char* library_name) {
    HMODULE dll = LoadLibraryA(library_name);
    if (!dll) {
        _sdyn_fail("Failed to load library\n");
    }

#define _SDYN_XMACRO(name, ret, params, args) \
    pfn_ ## name = (PFN_ ## name) (void*) GetProcAddress(dll, #name); \
    SOKOL_ASSERT(pfn_ ## name != NULL);

    _SDYN_APP_FUNCS
    _SDYN_GFX_FUNCS
    _SDYN_GLUE_FUNCS

#undef _SDYN_XMACRO
}

#endif // _SDYN_WIN32

#endif // SOKOL_DYN_IMPL
