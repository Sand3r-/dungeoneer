#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <xinput.h>
#include <objbase.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <timeapi.h>
#include <stdio.h>
#include "program_options.h"
#include "language_layer.h"
#include "platform.h"
#include "app.c"

static Platform global_platform = {0};
static char global_executable_path[256];
static char global_executable_directory[256];
static HDC global_device_context;

internal void
LinuxOutputError(char *title, char *format, ...)
{
    fprintf(stderr, "ERROR: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

internal void
LinuxToggleFullscreen(HWND hwnd)
{
    local_persist WINDOWPLACEMENT last_window_placement = {
        sizeof(last_window_placement)
    };
    
    DWORD window_style = GetWindowLong(hwnd, GWL_STYLE);
    if(window_style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        if(GetWindowPlacement(hwnd, &last_window_placement) &&
           GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY),
                          &monitor_info))
        {
            
            SetWindowLong(hwnd, GWL_STYLE,
                          window_style & ~WS_OVERLAPPEDWINDOW);
            
            SetWindowPos(hwnd, HWND_TOP,
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right -
                         monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom -
                         monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(hwnd, GWL_STYLE,
                      window_style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hwnd, &last_window_placement);
        SetWindowPos(hwnd, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

#include "linux_opengl.c"
#include "linux_alsa.c"

internal LRESULT
Win32WindowProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result;
    if(message == WM_CLOSE || message == WM_DESTROY || message == WM_QUIT)
    {
        global_platform.quit = 1;
        result = 0;
    }
    else if(message == WM_LBUTTONDOWN)
    {
        if(!global_platform.left_mouse_down)
        {
            global_platform.left_mouse_pressed = 1;
        }
        global_platform.left_mouse_down = 1;
    }
    else if(message == WM_LBUTTONUP)
    {
        global_platform.left_mouse_down = 0;
        global_platform.left_mouse_pressed = 0;
    }
    else if(message == WM_RBUTTONDOWN)
    {
        if(!global_platform.right_mouse_down)
        {
            global_platform.right_mouse_pressed = 1;
        }
        global_platform.right_mouse_down = 1;
    }
    else if(message == WM_RBUTTONUP)
    {
        global_platform.right_mouse_down = 0;
        global_platform.right_mouse_pressed = 0;
    }
    else if(message == WM_MOUSEWHEEL)
    {
        i16 wheel_delta = HIWORD(w_param);
        global_platform.mouse_scroll_y = (f32)wheel_delta;
    }
    else if(message == WM_MOUSEHWHEEL)
    {
        i16 wheel_delta = HIWORD(w_param);
        global_platform.mouse_scroll_x = (f32)wheel_delta;
    }
    else if(message == WM_SYSKEYDOWN || message == WM_SYSKEYUP ||
            message == WM_KEYDOWN || message == WM_KEYUP)
    {
        u64 vkey_code = w_param;
        b32 was_down = !!(l_param & (1 << 30));
        b32 is_down =   !(l_param & (1 << 31));
        
        u64 key_input = 0;
        
        if((vkey_code >= 'A' && vkey_code <= 'Z') ||
           (vkey_code >= '0' && vkey_code <= '9'))
        {
            // NOTE(rjf): Letter/number buttons
            key_input = (vkey_code >= 'A' && vkey_code <= 'Z') ? KEY_a + (vkey_code-'A') : KEY_0 + (vkey_code-'0');
        }
        else
        {
            if(vkey_code == VK_ESCAPE)
            {
                key_input = KEY_esc;
            }
            else if(vkey_code >= VK_F1 && vkey_code <= VK_F12)
            {
                key_input = KEY_f1 + vkey_code - VK_F1;
            }
            else if(vkey_code == VK_OEM_3)
            {
                key_input = KEY_grave_accent;
            }
            else if(vkey_code == VK_OEM_MINUS)
            {
                key_input = KEY_minus;
            }
            else if(vkey_code == VK_OEM_PLUS)
            {
                key_input = KEY_equal;
            }
            else if(vkey_code == VK_BACK)
            {
                key_input = KEY_backspace;
            }
            else if(vkey_code == VK_TAB)
            {
                key_input = KEY_tab;
            }
            else if(vkey_code == VK_SPACE)
            {
                key_input = KEY_space;
            }
            else if(vkey_code == VK_RETURN)
            {
                key_input = KEY_enter;
            }
            else if(vkey_code == VK_CONTROL)
            {
                key_input = KEY_ctrl;
            }
            else if(vkey_code == VK_SHIFT)
            {
                key_input = KEY_shift;
            }
            else if(vkey_code == VK_MENU)
            {
                key_input = KEY_alt;
            }
            else if(vkey_code == VK_UP)
            {
                key_input = KEY_up;
            }
            else if(vkey_code == VK_LEFT)
            {
                key_input = KEY_left;
            }
            else if(vkey_code == VK_DOWN)
            {
                key_input = KEY_down;
            }
            else if(vkey_code == VK_RIGHT)
            {
                key_input = KEY_right;
            }
            else if(vkey_code == VK_DELETE)
            {
                key_input = KEY_delete;
            }
            else if(vkey_code == VK_PRIOR)
            {
                key_input = KEY_page_up;
            }
            else if(vkey_code == VK_NEXT)
            {
                key_input = KEY_page_down;
            }
        }
        
        if(is_down)
        {
            if(!global_platform.key_down[key_input])
            {
                global_platform.key_pressed[key_input] = 1;
            }
            ++global_platform.key_down[key_input];
            global_platform.last_key = (i32)key_input;
            
            if(key_input == KEY_backspace && global_platform.target_text)
            {
                if(global_platform.key_down[KEY_ctrl])
                {
                    for(u32 i = global_platform.target_text_edit_pos-2;
                        i >= 0 && i < global_platform.target_text_max_characters;
                        --i)
                    {
                        if(!i || global_platform.target_text[i-1] == ' ')
                        {
                            global_platform.target_text[i] = 0;
                            global_platform.target_text_edit_pos = i;
                            break;
                        }
                    }
                }
                else
                {
                    if(global_platform.target_text_edit_pos)
                    {
                        // NOTE(rjf): This assumes editing only takes place at
                        //            the end of the string!!!
                        global_platform.target_text[--global_platform.target_text_edit_pos] = 0;
                    }
                }
            }
            else if(key_input == KEY_f4 && global_platform.key_down[KEY_alt])
            {
                global_platform.quit = 1;
            }
        }
        else
        {
            global_platform.key_down[key_input] = 0;
            global_platform.key_pressed[key_input] = 0;
        }
    }
    else
    {
        result = DefWindowProc(window_handle, message, w_param, l_param);
    }
    return result;
}

internal void
LinuxSwapBuffers(void)
{
    LinuxOpenGLSwapBuffers();
}

internal void *
LinuxHeapAlloc(u32 size)
{
    return malloc(size);
}

internal void
LinuxHeapFree(void *data)
{
    free(data);
}

internal b32
LinuxLoadEntireFile(char *filename, void **data, u32 *data_size)
{
    b32 read_successful = 0;
    
    return read_successful;
}

internal void
LinuxFreeFileData(void *data)
{
    LinuxHeapFree(data);
}

int
main(int argument_count, char **arguments)
{
    // NOTE(rjf): Calculate executable name and path
    {
        DWORD size_of_executable_path =
            GetModuleFileNameA(0, global_executable_path, sizeof(global_executable_path));
        
        // NOTE(rjf): Calculate executable directory
        {
            MemoryCopy(global_executable_directory, global_executable_path, size_of_executable_path);
            char *one_past_last_slash = global_executable_directory;
            for(i32 i = 0; global_executable_directory[i]; ++i)
            {
                if(global_executable_directory[i] == '\\')
                {
                    one_past_last_slash = global_executable_directory + i + 1;
                }
            }
            *one_past_last_slash = 0;
        }
    }
    
    WNDCLASS window_class = {0};
    {
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc = Win32WindowProc;
        window_class.hInstance = instance;
        window_class.lpszClassName = WINDOW_TITLE "WindowClass";
        window_class.hCursor = LoadCursor(0, IDC_ARROW);
    }
    
    if(!RegisterClass(&window_class))
    {
        // NOTE(rjf): ERROR: Window class registration failure
        Win32OutputError("Fatal Error", "Window class registration failure.");
        goto quit;
    }
    
    HWND window_handle = CreateWindow(window_class.lpszClassName, WINDOW_TITLE,
                                      WS_OVERLAPPEDWINDOW,
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      DEFAULT_WINDOW_WIDTH,
                                      DEFAULT_WINDOW_HEIGHT,
                                      0, 0, instance, 0);
    
    if(!window_handle)
    {
        // NOTE(rjf): ERROR: Window creation failure
        Win32OutputError("Fatal Error", "Window creation failure.");
        goto quit;
    }
    
    // NOTE(rjf): Initialize sound output
    Win32SoundOutput sound_output = {0};
    {
        sound_output.channels = 2;
        sound_output.samples_per_second = 48000;
        sound_output.latency_frame_count = 48000;
        Win32LoadWASAPI();
        Win32InitWASAPI(&sound_output);
    }
    
    // NOTE(rjf): Initialize platform
    {
        global_platform.quit = 0;
        global_platform.initialized = 0;
        global_platform.window_width = DEFAULT_WINDOW_WIDTH;
        global_platform.window_height = DEFAULT_WINDOW_HEIGHT;
        global_platform.target_frames_per_second = 60.f;
        global_platform.current_time = 0.f;
        
        global_platform.permanent_storage_size = PERMANENT_STORAGE_SIZE;
        global_platform.scratch_storage_size = SCRATCH_STORAGE_SIZE;
        
        global_platform.permanent_storage = calloc(1, global_platform.permanent_storage_size);
        global_platform.scratch_storage = calloc(1, global_platform.scratch_storage_size);
        
        global_platform.sample_out = calloc(1, sound_output.samples_per_second * sizeof(i16) * 2);
        
        global_platform.OutputError = LinuxOutputError;
        global_platform.LoadOpenGLProcedure = LinuxLoadOpenGLProcedure;
        global_platform.SwapBuffers = LinuxSwapBuffers;
        global_platform.LoadEntireFile = LinuxLoadEntireFile;
        global_platform.FreeFileData = LinuxFreeFileData;
    }
    
    global_device_context = GetDC(window_handle);
    
    if(!LinuxInitOpenGL(&global_device_context, instance))
    {
        // NOTE(rjf): ERROR: OpenGL initialization error
        LinuxOutputError("Fatal Error", "OpenGL initialization failure.");
        goto quit;
    }
    
    b32 sleep_is_granular = (timeBeginPeriod(1) == TIMERR_NOERROR);
    LARGE_INTEGER performance_counter_frequency;
    LARGE_INTEGER begin_frame_time_data;
    LARGE_INTEGER end_frame_time_data;
    
    QueryPerformanceFrequency(&performance_counter_frequency);
    
    ShowWindow(window_handle, n_show_cmd);
    UpdateWindow(window_handle);
    
    while(!global_platform.quit)
    {
        QueryPerformanceCounter(&begin_frame_time_data);
        
        MSG message;
        while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        
        // NOTE(rjf): Update input data (post-event)
        {
            POINT mouse;
            GetCursorPos(&mouse);
            ScreenToClient(window_handle, &mouse);
            global_platform.mouse_x = (f32)(mouse.x);
            global_platform.mouse_y = (f32)(mouse.y);
        }
        
        // NOTE(rjf): Update window size
        {
            RECT client_rect;
            GetClientRect(window_handle, &client_rect);
            global_platform.window_width = client_rect.right - client_rect.left;
            global_platform.window_height = client_rect.bottom - client_rect.top;
        }
        
        // NOTE(rjf): Find how much sound to write and where
        if(sound_output.initialized)
        {
            global_platform.sample_count_to_output = 0;
            UINT32 sound_padding_size;
            if(SUCCEEDED(sound_output.audio_client->lpVtbl->GetCurrentPadding(sound_output.audio_client,
                                                                              &sound_padding_size)))
            {
                
                global_platform.samples_per_second = sound_output.samples_per_second;
                
                global_platform.sample_count_to_output = 
                    (u32)(sound_output.latency_frame_count - sound_padding_size);
                
                if(global_platform.sample_count_to_output > sound_output.latency_frame_count)
                {
                    global_platform.sample_count_to_output = sound_output.latency_frame_count;
                }
            }
            
            for(u32 i = 0; i < sound_output.buffer_frame_count; ++i)
            {
                global_platform.sample_out[i] = 0;
            }
        }
        
        b32 last_vsync = global_platform.vsync;
        b32 last_fullscreen = global_platform.fullscreen;
        
        Update(&global_platform);
        
        // NOTE(rjf): Fill sound buffer with game sound
        if(sound_output.initialized)
        {
            LinuxFillSoundBuffer(global_platform.sample_count_to_output,
                                 global_platform.sample_out,
                                 &sound_output);
        }
        
        // NOTE(rjf): Update vsync if necessary
        if(last_vsync != global_platform.vsync)
        {
            LinuxOpenGLSetVerticalSync(global_platform.vsync);
        }
        
        // NOTE(rjf): Update fullscreen if necessary
        if(last_fullscreen != global_platform.fullscreen)
        {
            LinuxToggleFullscreen(window_handle);
        }
        
        QueryPerformanceCounter(&end_frame_time_data);
        
        // NOTE(rjf): Wait if necessary.
        {
            i64 desired_frame_counts = (i64)(performance_counter_frequency.QuadPart / global_platform.target_frames_per_second);
            i64 frame_count = end_frame_time_data.QuadPart - begin_frame_time_data.QuadPart;
            i64 counts_to_wait = desired_frame_counts - frame_count;
            
            LARGE_INTEGER begin_wait_time_data;
            LARGE_INTEGER end_wait_time_data;
            
            QueryPerformanceCounter(&begin_wait_time_data);
            
            while(counts_to_wait > 0)
            {
                if(sleep_is_granular)
                {
                    DWORD milliseconds_to_sleep = (DWORD)(1000.0 * ((f64)(counts_to_wait) / performance_counter_frequency.QuadPart));
                    if(milliseconds_to_sleep > 1)
                    {
                        --milliseconds_to_sleep;
                        Sleep(milliseconds_to_sleep);
                    }
                }
                
                QueryPerformanceCounter(&end_wait_time_data);
                counts_to_wait -= end_wait_time_data.QuadPart - begin_wait_time_data.QuadPart;
                begin_wait_time_data = end_wait_time_data;
            }
        }
        
        global_platform.current_time += 1.f / global_platform.target_frames_per_second;
    }
    
    LinuxCleanUpOpenGL(&global_device_context);
    
    quit:;
    
    return 0;
}
