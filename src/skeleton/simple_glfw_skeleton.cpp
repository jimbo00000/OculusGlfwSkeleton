// simple_glfw_skeleton.cpp
// GLFW Skeleton for the basic Oculus Rift/OVR OpenGL app.

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef USE_CUDA
#else
#  include "vector_make_helpers.h"
#endif
#include "vectortypes.h"

#include "Logger.h"
#include "FBO.h"

#include "AntOculusAppSkeleton.h"

AntOculusAppSkeleton g_app;

int running = 0;

GLFWwindow* g_pControlWindow = NULL;
GLFWwindow* g_pOculusWindow = NULL;

GLFWmonitor* g_pPrimaryMonitor = NULL;
GLFWmonitor* g_pOculusMonitor = NULL;

Timer g_timer;

void display()
{
    /// display control window
    if (g_pControlWindow != NULL)
    {
        glfwMakeContextCurrent(g_pControlWindow);

        glViewport(0,0, g_app.w(), g_app.h());
        g_app.display(false);
        glfwSwapBuffers(g_pControlWindow);
    }

    /// Display Oculus window - for best results have no windows on your extended Win7 desktop.
    if (g_pOculusWindow != NULL)
    {
        glfwMakeContextCurrent(g_pOculusWindow);

        glViewport(0,0,(GLsizei)g_app.GetOculusWidth(), (GLsizei)g_app.GetOculusHeight());
        g_app.display(true);
        glfwSwapBuffers(g_pOculusWindow);
    }
}

void mouseDown(GLFWwindow* pWindow, int button, int action, int mods)
{
    double x,y;
    glfwGetCursorPos(pWindow, &x, &y);
    g_app.mouseDown(button, action, (int)x, (int)y);
}

void mouseMove(GLFWwindow* pWindow, double x, double y)
{
    g_app.mouseMove((int)x, (int)y);
}

void mouseWheel(GLFWwindow* pWindow, double x, double y)
{
    g_app.mouseWheel((int)x, (int)y);
}

void keyboard(GLFWwindow* pWindow, int key, int codes, int action, int mods)
{
    g_app.keyboard(key, action, 0,0);
}


void timestep()
{
    float dt = (float)g_timer.seconds();
    g_timer.reset();
    g_app.timestep(dt);
}


///@brief Window resized event callback
void resize(GLFWwindow* pWindow, int w, int h)
{
    g_app.resize(w,h);
    glViewport(0,0,(GLsizei)w, (GLsizei)h);

    ///@note We can mitigate the effect of resizing the control window on the Oculus user
    /// by continuing to track the head in timestep as we resize.
    /// However, when the mouse is held still while resizing, we do not refresh.
    timestep();
    display();
}


///@brief Attempt to determine which of the connected monitors is the Oculus Rift and which
/// are not. The only heuristic available for this purpose is resolution.
void IdentifyMonitors()
{
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    for (int i=0; i<count; ++i)
    {
        GLFWmonitor* pMonitor = monitors[i];
        if (pMonitor == NULL)
            continue;
        const GLFWvidmode* mode = glfwGetVideoMode(pMonitor);

        /// Take a guess at which is the Oculus by resolution
        if (
            (mode->width  == g_app.GetOculusWidth()) &&
            (mode->height == g_app.GetOculusHeight())
            )
        {
            g_pOculusMonitor = pMonitor;
        }
        else if (g_pPrimaryMonitor == NULL)
        {
            /// Guess that the first (probably)non-Oculus monitor is primary.
            ///@note The multi-monitor setup requires the two screens to be aligned along their top edge
            /// with the Oculus monitor to the right of the primary.
            g_pPrimaryMonitor = pMonitor;
        }
    }
}


///@brief Dump a list of monitor info to Log and stdout.
/// http://www.glfw.org/docs/3.0/monitor.html
void PrintMonitorInfo()
{
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    printf("Found %d monitors:\n", count);
    LOG_INFO("Found %d monitors:", count);
    for (int i=0; i<count; ++i)
    {
        GLFWmonitor* pMonitor = monitors[i];
        if (pMonitor == NULL)
            continue;
        printf("  Monitor %d:\n", i);
        LOG_INFO("  Monitor %d:", i);

        /// Monitor name
        const char* pName = glfwGetMonitorName(pMonitor);
        printf("    Name: %s\n", pName);
        LOG_INFO("    Name: %s", pName);

        /// Monitor Physical Size
        int widthMM, heightMM;
        glfwGetMonitorPhysicalSize(pMonitor, &widthMM, &heightMM);
        //const double dpi = mode->width / (widthMM / 25.4);
        printf("    physical size: %d x %d mm\n");
        LOG_INFO("    physical size: %d x %d mm");

        /// Modes
        const GLFWvidmode* mode = glfwGetVideoMode(pMonitor);
        printf("    Current mode:  %dx%d @ %dHz (RGB %d %d %d bits)\n",
                mode->width,
                mode->height,
                mode->refreshRate,
                mode->redBits,
                mode->greenBits, 
                mode->blueBits);
        LOG_INFO("    Current mode:  %dx%d @ %dHz (RGB %d %d %d bits)",
                mode->width,
                mode->height,
                mode->refreshRate,
                mode->redBits,
                mode->greenBits, 
                mode->blueBits);

#if 0
        int modeCount;
        const GLFWvidmode* modes = glfwGetVideoModes(pMonitor, &modeCount);
        printf("    %d Modes:\n", modeCount);
        LOG_INFO("    %d Modes:", modeCount);
        for (int j=0; j<modeCount; ++j)
        {
            const GLFWvidmode& m = modes[j];
            printf("      %dx%d @ %dHz (RGB %d %d %d bits)\n",
                m.width, m.height, m.refreshRate, m.redBits, m.greenBits, m.blueBits);
            LOG_INFO("      %dx%d @ %dHz (RGB %d %d %d bits)",
                m.width, m.height, m.refreshRate, m.redBits, m.greenBits, m.blueBits);
        }
#endif
    }
}


static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

bool initGlfw(int argc, char **argv, bool fullScreen)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return false;

    PrintMonitorInfo();
    IdentifyMonitors();

    //LOG_INFO("Initializing Glfw and window @ %d x %d", m_windowWidth, m_windowHeight);

    /// Init Control window containing AntTweakBar
    {
        g_pControlWindow = glfwCreateWindow(g_app.w(), g_app.h(), "Control Window", NULL, NULL);
        if (!g_pControlWindow)
        {
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(g_pControlWindow);

        glfwSetWindowSizeCallback (g_pControlWindow, resize);
        glfwSetMouseButtonCallback(g_pControlWindow, mouseDown);
        glfwSetCursorPosCallback  (g_pControlWindow, mouseMove);
        glfwSetScrollCallback     (g_pControlWindow, mouseWheel);
        glfwSetKeyCallback        (g_pControlWindow, keyboard);
        //glfwSetCharCallback       (g_pControlWindow, charkey);
    }

    /// Init secondary window
    if (g_pOculusMonitor != NULL)
    {
        g_pOculusWindow = glfwCreateWindow(
            g_app.GetOculusWidth(), g_app.GetOculusHeight(),
            "Oculus Window",
            ///@note Fullscreen windows cannot be positioned. The fullscreen window over the Oculus
            /// monitor would be the preferred solution, but on Windows that fullscreen window will disappear
            /// on the first mouse input occuring outside of it, defeating the purpose of the first window.
            NULL, //g_pOculusMonitor,
            g_pControlWindow);

        if (!g_pOculusWindow)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }
        glfwMakeContextCurrent(g_pOculusWindow);

        /// Position Oculus secondary monitor pseudo-fullscreen window
        if (g_pPrimaryMonitor != NULL)
        {
            const GLFWvidmode* pMode = glfwGetVideoMode(g_pPrimaryMonitor);
            if (pMode != NULL)
            {
                glfwSetWindowPos(g_pOculusWindow, pMode->width, 0);
            }
        }

        glfwSetKeyCallback(g_pOculusWindow, keyboard);
        glfwShowWindow(g_pOculusWindow);
    }

    /// If we are not sharing contexts between windows, make the appropriate one current here.
    //glfwMakeContextCurrent(g_pControlWindow);

    return true;
}



/// Initialize then enter the main loop
int main(int argc, char *argv[])
{
    bool fullScreen = false;

    g_app.initVR(fullScreen);

    initGlfw(argc, argv, fullScreen);

    g_app.initGL(argc, argv);
    g_app.initJoysticks();

    /// Main loop
    running = GL_TRUE;
    while (running)
    {
        timestep();
        g_app.frameStart();
        display();
        glfwPollEvents();

        if (glfwWindowShouldClose(g_pControlWindow))
            running = GL_FALSE;
        if (g_pOculusWindow && glfwWindowShouldClose(g_pOculusWindow))
            running = GL_FALSE;
    }

    return 0;
}
