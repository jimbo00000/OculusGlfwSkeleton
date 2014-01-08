// AntOculusAppSkeleton.cpp

#include "AntOculusAppSkeleton.h"

AntOculusAppSkeleton::AntOculusAppSkeleton()
: m_timer()
, m_fps(0.0f)
#ifdef USE_ANTTWEAKBAR
, m_bar(NULL)
#endif
{
}

AntOculusAppSkeleton::~AntOculusAppSkeleton()
{
    ///@todo Delete this before glfw
    //delete m_bar;
}

#ifdef USE_ANTTWEAKBAR

static void TW_CALL ResetDistortion(void *clientData)
{
    if (clientData)
    {
        RiftDistortionParams* pP = static_cast<RiftDistortionParams *>(clientData);
        RiftDistortionParams reset;
        memcpy(pP, &reset, sizeof(RiftDistortionParams));
    }
}


void AntOculusAppSkeleton::_InitializeBar()
{
    ///@note Bad size errors will be thrown if this is not called at init
    TwWindowSize(m_windowWidth, m_windowHeight);

    // Create a tweak bar
    m_bar = TwNewBar("TweakBar");
    TwDefine(" TweakBar refresh=0.1 ");

    TwAddVarRO(m_bar, "fps", TW_TYPE_FLOAT, &m_fps, 
               " label='fps' help='Frames per second' precision=0 ");

    TwAddVarRW(m_bar, "cube scale", TW_TYPE_FLOAT, &m_scene.m_cubeScale, 
               " label='cube scale' min=1 max=20 step=1.0 keyIncr=a keyDecr=A help='cube scale' ");
    TwAddVarRW(m_bar, "amplitude", TW_TYPE_FLOAT, &m_scene.m_amplitude, 
               " label='amplitude' min=0 max=2 step=0.01 keyIncr=z keyDecr=Z help='amplitude' ");

    TwAddVarRW(m_bar, "followcam.x", TW_TYPE_FLOAT, &FollowCamDisplacement.x,
               " label='followcam.x' min=-30 max=30 step=0.01 help='followcam.x' group=camera ");
    TwAddVarRW(m_bar, "followcam.y", TW_TYPE_FLOAT, &FollowCamDisplacement.y,
               " label='followcam.y' min=-30 max=30 step=0.01 help='followcam.y' group=camera ");
    TwAddVarRW(m_bar, "followcam.z", TW_TYPE_FLOAT, &FollowCamDisplacement.z,
               " label='followcam.z' min=-30 max=30 step=0.01 help='followcam.z' group=camera ");

    TwAddVarRW(m_bar, "viewAngle", TW_TYPE_FLOAT, &m_viewAngleDeg,
               " label='viewAngle' min=30 max=90 step=0.1 help='viewAngle' group=camera ");


    TwAddVarRW(m_bar, "lensOff", TW_TYPE_FLOAT, &m_riftDist,
               " label='lensOff' min=0 max=0.1 step=0.001 help='lensOff' group=HMD ");
    TwAddButton(m_bar, "ResetDistortion", ResetDistortion, &m_riftDist,
        " label='ResetDistortion' group='HMD' ");

}
#endif

bool AntOculusAppSkeleton::initGL(int argc, char **argv)
{
#ifdef USE_ANTTWEAKBAR
    TwInit(TW_OPENGL, NULL);
    _InitializeBar();
#endif
    return OculusAppSkeleton::initGL(argc, argv);
}

void AntOculusAppSkeleton::display(bool useOculus)
{
    OculusAppSkeleton::display(useOculus);

#ifdef USE_ANTTWEAKBAR
    if (useOculus == false)
    {
        TwRefreshBar(m_bar);
        TwDraw(); ///@todo Should this go first? Will it write to a depth buffer?
    }
#endif
}

void AntOculusAppSkeleton::mouseDown(int button, int state, int x, int y)
{
#ifdef USE_ANTTWEAKBAR
    int ant = TwEventMouseButtonGLFW(button, state);
    if (ant != 0)
        return;
#endif
    OculusAppSkeleton::mouseDown(button, state, x, y);
}

void AntOculusAppSkeleton::mouseMove(int x, int y)
{
#ifdef USE_ANTTWEAKBAR
    TwEventMousePosGLFW(x, y);
#endif
    OculusAppSkeleton::mouseMove(x, y);
}

void AntOculusAppSkeleton::mouseWheel(int x, int y)
{
#ifdef USE_ANTTWEAKBAR
    TwEventMouseWheelGLFW(x);
#endif
    OculusAppSkeleton::mouseWheel(x, y);
}


void AntOculusAppSkeleton::keyboard(int key, int action, int x, int y)
{
#ifdef USE_ANTTWEAKBAR
    int ant = TwEventKeyGLFW(key, action);
    if (ant != 0)
        return;
#endif

    OculusAppSkeleton::keyboard(key, action, x, y);
}

void AntOculusAppSkeleton::charkey(unsigned int key)
{
#ifdef USE_ANTTWEAKBAR
    int ant = TwEventCharGLFW(key, 0);
    if (ant != 0)
        return;
#endif
   // OculusAppSkeleton::keyboard(key, 0, 0);
}

void AntOculusAppSkeleton::resize(int w, int h)
{
#ifdef USE_ANTTWEAKBAR
    TwWindowSize(m_windowWidth, m_windowHeight);
#endif
    OculusAppSkeleton::resize(w,h);
}
