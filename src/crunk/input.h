#ifndef INPUT_H
#define INPUT_H

enum Button_State {
    ButtonState_Down      = (1<<0),
    ButtonState_Pressed   = (1<<1),
    ButtonState_Released  = (1<<3),
};
EnumDefineFlagOperators(Button_State);

struct Input {
    Button_State buttons[OS_KEY_COUNT];

    v2_s32 mouse_position;
    v2_s32 last_mouse_position;

    v2_s32 mouse_drag_start;
    b32 mouse_dragging;

    v2_s32 scroll_delta;

    v2_s32 client_dim;
};

internal void input_begin(OS_Handle window_handle, OS_Event *events);
internal void input_end(OS_Handle window_handle);

#endif // INPUT_H
