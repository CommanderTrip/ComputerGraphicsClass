use std::any::Any;

use glu_sys::*;
use sdl2::video::{GLContext, Window};
use sdl2::VideoSubsystem;

fn my_3d_projection(width: i32, height: i32) {
    unsafe {
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(
            0.0,
            width.try_into().unwrap(),
            0.0,
            height.try_into().unwrap(),
            0.0,
            100.0,
        );
        glMatrixMode(GL_MODELVIEW);
    }
}

const START_X_POS: i32 = 25;
const START_Y_POS: i32 = 25;

pub fn my_setup(
    screen_width: i32,
    screen_height: i32,
    window_name: &str,
    video_subsystem: &VideoSubsystem,
) -> (Window, GLContext, Box<dyn Any>) {
    let gl_attributes = video_subsystem.gl_attr();

    // Allow for current OpenGL4.3 but backwards compatibility to legacy GL 4.3
    gl_attributes.set_context_version(4, 3);
    gl_attributes.set_context_profile(sdl2::video::GLProfile::Compatibility);
    gl_attributes.set_double_buffer(false); // Change to 'true' for double buffering

    // Build the Window
    let window = video_subsystem
        .window(
            window_name,
            screen_width.try_into().unwrap(),
            screen_height.try_into().unwrap(),
        )
        .position(START_X_POS, START_Y_POS)
        .resizable()
        .opengl()
        .build()
        .unwrap();

    // Build the OpenGL context and environment
    let _gl_context = window.gl_create_context().unwrap();
    let _gl =
        gl::load_with(|s| video_subsystem.gl_get_proc_address(s) as *const std::os::raw::c_void);
    my_3d_projection(screen_width, screen_height);
    return (window, _gl_context, Box::new(_gl));
}
