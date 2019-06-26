#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <malloc.h>
#include <time.h>

#include "glad_wgl.h"
#include "../scene.h"

#define LOG(...) {char cad[1024]; sprintf(cad, __VA_ARGS__);  OutputDebugString(cad);}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        // Acording to documentation low-order word of lParam gives us screen width pixel size,
        // and high-order word of lParam gives height pixel size.
        // https://docs.microsoft.com/en-us/windows/desktop/winmsg/wm-size
        int32_t width = LOWORD(lParam);
        int32_t height = HIWORD(lParam);
        glViewport(0, 0, width, height);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Debug callback for OpenGL 4.3 feature glDebugOutput
// https://www.khronos.org/opengl/wiki/OpenGL_Error#Catching_errors_.28the_easy_way.29
void GLAPIENTRY MessageCallback(GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar* message,
                                const void* userParam) {
    LOG("GL CALLBACK: %s type = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        type, message);
}

static uint32_t CompileGLShaderFile(const char* filename, GLenum shaderType) {
    HANDLE shaderFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    assert(shaderFile != INVALID_HANDLE_VALUE);

    LARGE_INTEGER size;
    assert(GetFileSizeEx(shaderFile, &size));

    // Allocating shader source buffer in stack with alloca function.
    // Deallocate automatically when function complete.
    // TODO: If the shader source size bigger than stack size, we'll get a crash.
    // TODO: We should check for stack size or use _malloca function which does auto check stack size
    // and allocate on heap if size bigger than stack size.
    char* shaderSource = (char*) alloca(size.QuadPart + 1);
    DWORD readSize;
    assert(ReadFile(shaderFile, (LPVOID) shaderSource, size.QuadPart, &readSize, NULL));
    assert(readSize == size.QuadPart);
    // Put 0 char on end of the source buffer for indicating this file ended.
    shaderSource[size.QuadPart] = '\0';
    CloseHandle(shaderFile);

    uint32_t shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, 0);
    glCompileShader(shader);

    int32_t success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    char infoLog[1024];
    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
    OutputDebugString(infoLog);
    assert(success);

    return shader;
}

int WINAPI wWinMain(HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    PWSTR pCmdLine,
                    int nCmdShow) {
    // Window creation
    char* windowClassName = "MyWindowClass";
    WNDCLASS windowClass = {};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = windowClassName;

    RegisterClass(&windowClass);

    const int WIDTH = 1280;
    const int HEIGHT = 720;
    HWND windowHandle = CreateWindowEx(0,
                                       windowClassName,
                                       "Window",
                                       WS_OVERLAPPEDWINDOW,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       WIDTH,
                                       HEIGHT,
                                       NULL,
                                       NULL,
                                       hInstance,
                                       NULL);


    HDC deviceContext = GetDC(windowHandle);
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    int pixelFormat = ChoosePixelFormat(deviceContext, &pfd);
    SetPixelFormat(deviceContext, pixelFormat, &pfd);

    // Create OpenGL context
    HGLRC wglContext = wglCreateContext(deviceContext);
    // To able to use wgl extensions, we have to make current a WGL context.
    // Otherwise we can't get function pointers to WGL extensions.
    wglMakeCurrent(deviceContext, wglContext);
    
    // Load WGL extensions.
    gladLoadWGL(deviceContext);

    // We use OpenGL version 4.5 core profile and debug context for reasonable error messages.
    int glAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    // Create new WGL context with attributes, using WGL extensions.
    // And make current the new one and delete the old one.
    HGLRC newContext = wglCreateContextAttribsARB(deviceContext, 0, glAttribs);
    wglMakeCurrent(deviceContext, newContext);
    wglDeleteContext(wglContext);
    
    gladLoadGL();

    ShowWindow(windowHandle, nCmdShow);

    World* world = createScene();
    Vector3 cameraPosition = world->camera->position;

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);
    
    // Compute program
    uint32_t computeShader = CompileGLShaderFile("../win32gpu/compute.glsl", GL_COMPUTE_SHADER);

    uint32_t computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);
    int32_t computeProgramSuccess;
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &computeProgramSuccess);
    char infoLog[1024];
    glGetProgramInfoLog(computeProgram, 1024, NULL, infoLog);
    OutputDebugString(infoLog);
    assert(computeProgramSuccess);

    // Draw program
    uint32_t vertexShader = CompileGLShaderFile("../win32gpu/vertex.glsl", GL_VERTEX_SHADER);
    uint32_t fragmentShader = CompileGLShaderFile("../win32gpu/fragment.glsl", GL_FRAGMENT_SHADER);

    uint32_t shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    int32_t programSuccess;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &programSuccess);
    assert(programSuccess);

    const float buffer[] = { -1.0f, -1.0f,  0.0f, 0.0f,
                             -1.0f,  1.0f,  0.0f, 1.0f,
                              1.0f,  1.0f,  1.0f, 1.0f,
                              1.0f, -1.0f,  1.0f, 0.0f};

    const uint32_t elementBuffer[] = { 0, 1, 2,
                                       3, 0, 2 };
    
    uint32_t vbo;
    glGenBuffers(1, &vbo);

    uint32_t ebo;
    glGenBuffers(1, &ebo);
    
    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementBuffer), elementBuffer, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH,
        HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Compute shader initialization
    uint32_t ssbObjects[4];
    glGenBuffers(ARRAYSIZE(ssbObjects), ssbObjects);

    uint32_t materialsSSBO = ssbObjects[0];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materialsSSBO);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(Material) * world->materialCount, world->materials, 0);

    uint32_t spheresSSBO = ssbObjects[1];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spheresSSBO);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(Sphere) * world->sphereCount, world->spheres, 0);

    uint32_t planesSSBO = ssbObjects[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, planesSSBO);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(Plane) * world->planeCount, world->planes, 0);

    uint32_t bounceCountSSBO = ssbObjects[3];   

    glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    int frameIndex = 0;
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            uint32_t timeQuery;
            glGenQueries(1, &timeQuery);
            glBeginQuery(GL_TIME_ELAPSED, timeQuery);

            // Reset bounce count buffer to 0 on beginnig of the frame.
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bounceCountSSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW);
            uint32_t bounceCount = 0;
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t), &bounceCount);

            for (uint32_t ssboIndex = 0; ssboIndex < ARRAYSIZE(ssbObjects); ++ssboIndex) {
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssboIndex, ssbObjects[ssboIndex]);
            }

            glUseProgram(computeProgram);
            // We treat our main texture as a double buffer.
            // It is maybe not safe that using the same object for double buffer. 
            // But in the shader, we first read pixel and calculate color and write new color.
            // So there is no conflict on pixels. So we are okay for now!
            glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glUniform3f(glGetUniformLocation(computeProgram, "cameraPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z);
            glUniform1ui(glGetUniformLocation(computeProgram, "time"), clock());
            glUniform1ui(glGetUniformLocation(computeProgram, "frameIndex"), frameIndex);
            glDispatchCompute(WIDTH / 8, HEIGHT / 8, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, bounceCountSSBO);
            uint32_t* counter = (uint32_t*) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
            bounceCount = counter[0];
            glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    
            // Unbind storage buffers
            for (uint32_t ssboIndex = 0; ssboIndex < ARRAYSIZE(ssbObjects); ++ssboIndex) {
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssboIndex, 0);
            }

            glUseProgram(shaderProgram);
            glActiveTexture(GL_TEXTURE0 + 2);
            glBindTexture(GL_TEXTURE_2D, texture);
            glUniform1i(glGetUniformLocation(shaderProgram, "image"), 2);
            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            ++frameIndex;

            glEndQuery(GL_TIME_ELAPSED);
            uint32_t available = 0;
            while (!available) {
                glGetQueryObjectuiv(timeQuery, GL_QUERY_RESULT_AVAILABLE, &available);
            }

            uint64_t elapsedTimeNanos = 0;
            glGetQueryObjectui64v(timeQuery, GL_QUERY_RESULT, &elapsedTimeNanos);
            char cad[1024];
            sprintf(cad, "Elapsed Time: %lldms, Bounce Count: %ld, Performance: %.9fms/ray", elapsedTimeNanos / 1000000,
                bounceCount, ((double)elapsedTimeNanos / (double)bounceCount) / 1000000.0);
            SetWindowText(windowHandle, cad);

            SwapBuffers(deviceContext);
        }
    }

    // Yeah, I should destroy all the opengl objects that created, free scene memory,
    // destroy window and gl constext, etc
    // But after application terminated, all memory we allocated will be freed, and all objects will be destroyed.
    // Everything will be cleaned up by the OS.
    // Should I do that cleanup in code?

    return 0;
}
