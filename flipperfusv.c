#include <furi.h>
#include <gui/gui.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BUFFER_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 8)

static void render_callback(Canvas* canvas, void* ctx) {
    uint8_t* screen_buffer = ctx;
    if(screen_buffer) {
        // Direct bitmap drawing for speed
        canvas_draw_xbm(canvas, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, screen_buffer);
    }
}

int32_t pc_stream_app(void* p) {
    UNUSED(p);
    uint8_t* video_buffer = malloc(BUFFER_SIZE);
    memset(video_buffer, 0, BUFFER_SIZE);

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, video_buffer);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Acquire USB Serial (VCP)
    FuriHalSerialHandle* serial = furi_hal_serial_control_acquire(FuriHalSerialIdUsb);
    furi_hal_serial_init(serial, 250000); // High baud rate

    while(1) {
        // Wait for exactly one frame of data (1024 bytes)
        size_t read = furi_hal_serial_read(serial, video_buffer, BUFFER_SIZE, 50);
        if(read == BUFFER_SIZE) {
            view_port_update(view_port);
        }
        furi_delay_ms(1); // Yield to system
    }

    // Cleanup (in case of exit)
    furi_hal_serial_control_release(serial);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    free(video_buffer);
    return 0;
}