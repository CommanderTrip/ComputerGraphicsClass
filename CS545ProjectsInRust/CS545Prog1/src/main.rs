extern crate gl;

use crate::setup::my_setup;
use glu_sys::*;
use sdl2::{
    event::Event,
    sys::{SDL_Event, SDL_PushEvent},
    EventSubsystem, TimerSubsystem,
};

mod setup;

// Pen and Canvas Colors
const PEN_YELLOWISH_ORANGE: (f32, f32, f32) = (1.0, 0.60, 0.0);
const PEN_GREEN: (f32, f32, f32) = (0.0, 1.0, 0.0);
const PEN_RED: (f32, f32, f32) = (1.0, 0.0, 0.0);
const CANVAS_BLACK: (f32, f32, f32, f32) = (0.0, 0.0, 0.0, 1.0);

// Canvas Dimensions
const SCREEN_WIDTH: i32 = 480;
const SCREEN_HEIGHT: i32 = 480;

// Misc. Details
const CANVAS_NAME: &str = "CS 445/545 Prog 1 for Jason Carnahan";
const FRAMERATE: u32 = 30;
const TIMER_DELAY: u32 = 1000 / FRAMERATE;

// Necessary globals for scene tracking
static mut FOWL_X_OFFSET: i32 = 0;
static mut FOWL_HORIZONTAL_SEP: i32 = 0;
static FOWL_STARTING_POS_X: i32 = 50;
static HALF_FOWL_LENGTH: i32 = 14; // L in the fowl math description
static NUM_OF_FOWLS: i32 = 3;
static ANIMATE_RIGHT: bool = true;

/*
    Draws a green tower on the right side of the screen. The tower is defined
    to be 5px wide by 300px tall and connected by line segments. It is 10 units
    from the right side of the screen.
*/
fn draw_tower() {
    let height = 300;
    let width = 5;
    let margin = 10;

    unsafe {
        glColor3f(PEN_GREEN.0, PEN_GREEN.1, PEN_GREEN.2);
        glBegin(GL_LINE_LOOP);
        glVertex2i(SCREEN_WIDTH - margin - width, height); // Top left
        glVertex2i(SCREEN_WIDTH - margin - width, 1); // Bottom left
        glVertex2i(SCREEN_WIDTH - margin, 1); // Bottom right
        glVertex2i(SCREEN_WIDTH - margin, height); // Top right
        glEnd();
    }
}

/*
    Draws a red "Y" slingshot on the left side of the screen. The fowls will be
    "launched" from this slingshot represented with line segments. There are no
    size spcifications other than the base must be on the ground.
*/
fn draw_slingshot() {
    unsafe {
        glColor3f(PEN_RED.0, PEN_RED.1, PEN_RED.2);

        // Top left rectanlge of Y
        glBegin(GL_LINE_LOOP);
        glVertex2i(FOWL_STARTING_POS_X - 40, SCREEN_WIDTH / 2 + 45); // Top left
        glVertex2i(FOWL_STARTING_POS_X + 15, SCREEN_WIDTH / 2 - 55); // Bottom left
        glVertex2i(FOWL_STARTING_POS_X + 25, SCREEN_WIDTH / 2 - 50); // Bottom right
        glVertex2i(FOWL_STARTING_POS_X - 30, SCREEN_WIDTH / 2 + 50); // Top right
        glEnd();

        // Top Right rectanlge of Y
        glBegin(GL_LINE_LOOP);
        glVertex2i(FOWL_STARTING_POS_X + 30, SCREEN_WIDTH / 2 + 50); // Top left
        glVertex2i(FOWL_STARTING_POS_X + 15, SCREEN_WIDTH / 2 - 50); // Bottom left
        glVertex2i(FOWL_STARTING_POS_X + 25, SCREEN_WIDTH / 2 - 55); // Bottom right
        glVertex2i(FOWL_STARTING_POS_X + 40, SCREEN_WIDTH / 2 + 45); // Top right
        glEnd();

        // Stem of Y
        glBegin(GL_LINE_LOOP);
        glVertex2i(FOWL_STARTING_POS_X + 15, SCREEN_WIDTH / 2 - 50); // Top left
        glVertex2i(FOWL_STARTING_POS_X + 15, 1); // Bottom left
        glVertex2i(FOWL_STARTING_POS_X + 25, 1); // Bottom right
        glVertex2i(FOWL_STARTING_POS_X + 25, SCREEN_WIDTH / 2 - 50); // Top right
        glEnd();
    }
}

/**
    Draws a fowl to the canvas. Fowls are defined to have "Yellowish Orange"
    feathers and shaped to have 5 sides; 3 major sides will be 20px long
    and in the shape of a triangle. The base of the triangle will connect to
    the 2 minor sides, each 5px long. The base must also be parallel to the
    canvas width. Fowls will spawn in the left, center of the canvas.

    Fowl Point Math:
    Starting at the top point of the fowl and moving CCW, the points are A, B, C,
    D, E (top, left, bottom left, bottom right, right). The origin (x, y) of the
    fowl is at the intersection of the line segment BE and the line segment where
    A perpendicularly meets the CD line segment.

    - Points ABE will define the upper triangle and points BCDE will define the
    lower trapezoid.
    - ABE is a right triangle where A is 90 degrees. The line segment AB must be
    20px; therefore, the height of the triangle (L) is 20sin(45) ~= 14px.
    - The line segment BC must be 5px; therefore, a 3-4-5 triangle will be used
    to define a vertical height (H) of 4px and a horizontal base (B) of 3px.

    With this information, we can define every point for a singular fowl about
    the origin by the OpenGL Right Hand coordinate system:
    a = (x, y + L)
    b = (x - L, y)
    c = (x - L + B, y - H)
    d = (x + L - B, y - H)
    e = (x + L, y)

    flockSize - How many fowls to spawn; typically 3
    distanceBetween - The distance between fowls, typically 10 - 15px
*/
fn draw_fowl(flockSize: i32, distanceBetween: i32) {
    let flockOriginY = SCREEN_HEIGHT / 2;

    // Draw the flock
    for i in 0..flockSize {
        unsafe {
            // Flock origin is defined as the origin of the first fowl
            let flockOriginX = FOWL_STARTING_POS_X + FOWL_X_OFFSET;
            FOWL_HORIZONTAL_SEP = distanceBetween + HALF_FOWL_LENGTH * 2;

            // Included to reduce the size of the vertex inputs
            let calculatedSeparation = i * FOWL_HORIZONTAL_SEP;
            glColor3f(
                PEN_YELLOWISH_ORANGE.0,
                PEN_YELLOWISH_ORANGE.1,
                PEN_YELLOWISH_ORANGE.2,
            );

            // Draw Fowl Body in CCW order as defined by math
            glBegin(GL_LINE_LOOP);
            glVertex2i(
                flockOriginX + calculatedSeparation,
                flockOriginY + HALF_FOWL_LENGTH,
            ); // Top
            glVertex2i(
                flockOriginX - HALF_FOWL_LENGTH + calculatedSeparation,
                flockOriginY,
            ); // Left
            glVertex2i(
                flockOriginX - HALF_FOWL_LENGTH + 3 + calculatedSeparation,
                flockOriginY - 4,
            ); // Bottom Left
            glVertex2i(
                flockOriginX + HALF_FOWL_LENGTH - 3 + calculatedSeparation,
                flockOriginY - 4,
            ); // Bottom Right
            glVertex2i(
                flockOriginX + HALF_FOWL_LENGTH + calculatedSeparation,
                flockOriginY,
            ); // Right
            glEnd();

            // EXTRA CREDIT - Draw Fowl Hair Tufts
            glBegin(GL_LINES);
            glVertex2i(
                flockOriginX + calculatedSeparation,
                flockOriginY + HALF_FOWL_LENGTH,
            );
            glVertex2i(
                flockOriginX + 4 + calculatedSeparation,
                flockOriginY + HALF_FOWL_LENGTH + 3,
            ); // Right tuff
            glVertex2i(
                flockOriginX + calculatedSeparation,
                flockOriginY + HALF_FOWL_LENGTH,
            );
            glVertex2i(
                flockOriginX + calculatedSeparation,
                flockOriginY + HALF_FOWL_LENGTH + 5,
            ); // Vertical tuff
            glVertex2i(flockOriginX + calculatedSeparation, flockOriginY + 14);
            glVertex2i(
                flockOriginX - 4 + calculatedSeparation,
                flockOriginY + HALF_FOWL_LENGTH + 3,
            ); // Left tuff
            glEnd();

            // EXTRA CREDIT - Draw Fowl Face facing to our right
            glBegin(GL_LINES);
            glVertex2i(flockOriginX - 3 + calculatedSeparation, flockOriginY - 3);
            glVertex2i(flockOriginX + 3 + calculatedSeparation, flockOriginY); // Left mouth
            glVertex2i(flockOriginX + 3 + calculatedSeparation, flockOriginY);
            glVertex2i(flockOriginX + 5 + calculatedSeparation, flockOriginY - 3); // Right mouth
            glVertex2i(flockOriginX - 5 + calculatedSeparation, flockOriginY + 6);
            glVertex2i(flockOriginX + calculatedSeparation, flockOriginY + 3); // Fowl's right Eye
            glVertex2i(flockOriginX + 5 + calculatedSeparation, flockOriginY + 3);
            glVertex2i(flockOriginX + 7 + calculatedSeparation, flockOriginY + 5); // Fowl's left Eye
            glEnd();
        }
    }
}

fn window_event_handler() {
    unsafe {
        glClearColor(
            CANVAS_BLACK.0,
            CANVAS_BLACK.1,
            CANVAS_BLACK.2,
            CANVAS_BLACK.3,
        ); // Define background color
        glClear(GL_COLOR_BUFFER_BIT); // Clear the background
    }

    // Draw the scene
    draw_tower();
    draw_slingshot();
    draw_fowl(NUM_OF_FOWLS, 15);

    unsafe {
        // Execute Draw
        glFlush(); // Not including this caused issues for me
    }
}

fn timer_event_handler(
    timer_id: i32,
    event_subsystem: &EventSubsystem,
    timer_subsystem: &TimerSubsystem,
) {
    println!("hi?");
    match timer_id {
        1 => unsafe {
            FOWL_X_OFFSET += 4;
        },
        2 => unsafe {
            FOWL_X_OFFSET -= 4;
        },
        _ => {}
    }

    // Synthetic Window Event to Redraw
    let _res = event_subsystem.push_event(Event::Window {
        timestamp: 1,
        window_id: 1,
        win_event: sdl2::event::WindowEvent::FocusGained,
    });
}

fn main() {
    // Setup Environment
    let sdl_context = sdl2::init().unwrap();
    let video_subsystem = sdl_context.video().unwrap();
    let timer_subsystem = sdl_context.timer().unwrap();
    let event_subsystem = sdl_context.event().unwrap();

    // establishes OpenGL environment and Window
    let (_window, _gl_context, _gl) =
        my_setup(SCREEN_WIDTH, SCREEN_HEIGHT, CANVAS_NAME, &video_subsystem);

    let mut id = timer_subsystem.add_timer(
        1000,
        Box::new(|| -> u32 {
            timer_event_handler(1, &event_subsystem, &timer_subsystem);
            0
        }),
    );

    let mut event_queue = sdl_context.event_pump().unwrap(); // Get the event queue
    'main: loop {
        // Handle Events
        //`pushevent` can fire synthetic events
        for event in event_queue.poll_iter() {
            match event {
                Event::Quit { .. } => break 'main,
                Event::Window { timestamp, .. } => {
                    println!("{} Window Event", timestamp);
                    window_event_handler();
                }
                Event::KeyDown { keycode, .. } => match keycode {
                    _ => {
                        println!("Event::KeyDown");
                        id = timer_subsystem.add_timer(
                            1000,
                            Box::new(|| -> u32 {
                                timer_event_handler(1, &event_subsystem, &timer_subsystem);
                                0
                            }),
                        );
                    }
                },
                _ => {}
            }
        }
    }
}
