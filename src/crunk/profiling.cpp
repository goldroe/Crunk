#define MAX_FRAME_DELTA_BUFFER 512
struct Frame_Delta_Ring_Buffer {
    int count;
    int cap;
    int index;
    f32 data[MAX_FRAME_DELTA_BUFFER];
};

global Frame_Delta_Ring_Buffer g_frame_delta_ring_buffer;

internal void push_frame_delta(f32 delta) {
    Frame_Delta_Ring_Buffer *buffer = &g_frame_delta_ring_buffer;
    buffer->data[buffer->index++] = delta;
    buffer->index %= MAX_FRAME_DELTA_BUFFER;
    buffer->count++;
    buffer->count = ClampTop(buffer->count, MAX_FRAME_DELTA_BUFFER);
}
