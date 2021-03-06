// AntOculusAppSkeleton.cpp

#include "AntOculusAppSkeleton.h"

AntOculusAppSkeleton::AntOculusAppSkeleton()
: m_timer()
, m_fps(0.0f)
#ifdef USE_ANTTWEAKBAR
, m_pBar(NULL)
#endif
{
}

AntOculusAppSkeleton::~AntOculusAppSkeleton()
{
    ///@todo Delete this before glfw
    //delete m_pBar;
}

#ifdef USE_ANTTWEAKBAR

static void TW_CALL ResetDistortionParams(void *clientData)
{
    if (clientData)
    {
        RiftDistortionParams* pP = static_cast<RiftDistortionParams *>(clientData);
        RiftDistortionParams reset;
        memcpy(pP, &reset, sizeof(RiftDistortionParams));
    }
}

static void TW_CALL GetDistortionFboWidth(void *value, void *clientData)
{
    *static_cast<int *>(value) = static_cast<const OVRkill *>(clientData)->GetRenderBufferWidth();
}

static void TW_CALL GetDistortionFboHeight(void *value, void *clientData)
{
    *static_cast<int *>(value) = static_cast<const OVRkill *>(clientData)->GetRenderBufferHeight();
}

static void TW_CALL ReallocateDistortionFbo(void *clientData)
{
    if (clientData)
    {
        static_cast<AntOculusAppSkeleton *>(clientData)->ResizeFbo();
    }
}

static void TW_CALL SetBufferScaleCallback(const void *value, void *clientData)
{
    if (clientData)
    {
        static_cast<AntOculusAppSkeleton *>(clientData)->SetBufferScaleUp( *(float *)value);
        static_cast<AntOculusAppSkeleton *>(clientData)->ResizeFbo();
    }
}

static void TW_CALL GetBufferScaleCallback(void *value, void *clientData)
{
    if (clientData)
    {
        *(float *)value = static_cast<AntOculusAppSkeleton *>(clientData)->GetBufferScaleUp();
    }
}

static void TW_CALL GetMegaPxCount(void *value, void *clientData)
{
    *static_cast<float *>(value) = static_cast<const AntOculusAppSkeleton *>(clientData)->GetMegaPixelCount();
}

static void TW_CALL GetMegaPxPerSecond(void *value, void *clientData)
{
    *static_cast<float *>(value) = static_cast<const AntOculusAppSkeleton *>(clientData)->GetMegaPixelsPerSecond();
}

static void TW_CALL ResetEyePositionCB(void *clientData)
{
    static_cast<AntOculusAppSkeleton *>(clientData)->ResetEyePosition();
}


static void TW_CALL SetEyeHeightCallback(const void *value, void *clientData)
{
    if (clientData)
    {
        static_cast<AntOculusAppSkeleton *>(clientData)->SetEyeHeight( *(float *)value);
    }
}

static void TW_CALL GetEyeHeightCallback(void *value, void *clientData)
{
    if (clientData)
    {
        *(float *)value = static_cast<AntOculusAppSkeleton *>(clientData)->GetEyeHeight();
    }
}



void AntOculusAppSkeleton::_InitializeBar()
{
    ///@note Bad size errors will be thrown if this is not called at init
    TwWindowSize(m_windowWidth, m_windowHeight);

    // Create a tweak bar
    m_pBar = TwNewBar("TweakBar");
    TwDefine(" TweakBar refresh=0.1 ");
    TwDefine(" TweakBar fontsize=3 ");
    TwDefine(" TweakBar size='240 500' ");


    //
    // Performance Group
    //
    TwAddVarRO(m_pBar, "fps", TW_TYPE_FLOAT, &m_fps, 
               " label='fps' precision=0 group='Performance' ");

    TwAddVarCB(m_pBar, "FBO width", TW_TYPE_INT32, NULL, GetDistortionFboWidth, &m_ok,
        "precision=0 group='Performance' ");
    TwAddVarCB(m_pBar, "FBO height", TW_TYPE_INT32, NULL, GetDistortionFboHeight, &m_ok,
        "precision=0 group='Performance' ");
    TwAddVarCB(m_pBar, "MPixels", TW_TYPE_FLOAT, NULL, GetMegaPxCount, this,
        "precision=2 group='Performance' ");
    TwAddVarCB(m_pBar, "MPixels/sec", TW_TYPE_FLOAT, NULL, GetMegaPxPerSecond, this,
        "precision=0 group='Performance' ");

    TwAddVarCB(m_pBar, "FBO ScaleUp", TW_TYPE_FLOAT,
        SetBufferScaleCallback, GetBufferScaleCallback, this,
        " label='FBO ScaleUp' min=0.25 max=16.0 step=0.01 group=Performance ");

    TwAddVarRW(m_pBar, "FBO gutter size", TW_TYPE_INT32, &m_bufferGutterPx,
        " min=0 precision=0 group='Performance' ");



    //
    // HMD params passed to OVR Post Process Distortion shader
    //
    TwAddVarRW(m_pBar, "lensOff", TW_TYPE_FLOAT, &m_riftDist.lensOff,
               " label='lensOff'     min=0 max=0.1 step=0.001 group=HMD ");
    TwAddVarRW(m_pBar, "LensCenterX", TW_TYPE_FLOAT, &m_riftDist.LensCenterX,
               " label='LensCenterX' min=0 max=1.0 step=0.01 group=HMD ");
    TwAddVarRW(m_pBar, "LensCenterY", TW_TYPE_FLOAT, &m_riftDist.LensCenterY,
               " label='LensCenterY' min=0 max=1.0 step=0.01 group=HMD ");
    TwAddVarRW(m_pBar, "ScreenCenterX", TW_TYPE_FLOAT, &m_riftDist.ScreenCenterX,
               " label='ScreenCenterX' min=0 max=1.0 step=0.01 group=HMD ");
    TwAddVarRW(m_pBar, "ScreenCenterY", TW_TYPE_FLOAT, &m_riftDist.ScreenCenterY,
               " label='ScreenCenterY' min=0 max=1.0 step=0.01 group=HMD ");
    TwAddVarRW(m_pBar, "ScaleX", TW_TYPE_FLOAT, &m_riftDist.ScaleX,
               " label='ScaleX' min=0 max=1.0 step=0.01 group=HMD ");
    TwAddVarRW(m_pBar, "ScaleY", TW_TYPE_FLOAT, &m_riftDist.ScaleY,
               " label='ScaleY' min=0 max=1.0 step=0.01 group=HMD ");
    TwAddVarRW(m_pBar, "ScaleInX", TW_TYPE_FLOAT, &m_riftDist.ScaleInX,
               " label='ScaleInX' min=0 max=10.0 step=0.1 group=HMD ");
    TwAddVarRW(m_pBar, "ScaleInY", TW_TYPE_FLOAT, &m_riftDist.ScaleInY,
               " label='ScaleInY' min=0 max=10.0 step=0.1 group=HMD ");
    TwAddVarRW(m_pBar, "DistScale", TW_TYPE_FLOAT, &m_riftDist.DistScale,
               " label='DistScale' min=0 max=3.0 step=0.01 group=HMD ");

    TwAddButton(m_pBar, "ResetDistortionParams", ResetDistortionParams, &m_riftDist,
        " label='ResetDistortionParams' group='HMD' ");

    TwAddVarRW(m_pBar, "Flatten Stereo", TW_TYPE_BOOLCPP, &m_flattenStereo,
               " label='Flatten Stereo' group=HMD ");


    //
    // Camera params
    //
    TwAddVarRW(m_pBar, "followcam.x", TW_TYPE_FLOAT, &FollowCamDisplacement.x,
               " label='followcam.x' min=-30 max=30 step=0.01 group='Third Person Camera' ");
    TwAddVarRW(m_pBar, "followcam.y", TW_TYPE_FLOAT, &FollowCamDisplacement.y,
               " label='followcam.y' min=-30 max=30 step=0.01 group='Third Person Camera' ");
    TwAddVarRW(m_pBar, "followcam.z", TW_TYPE_FLOAT, &FollowCamDisplacement.z,
               " label='followcam.z' min=-30 max=30 step=0.01 group='Third Person Camera' ");

    TwAddVarRW(m_pBar, "viewAngle", TW_TYPE_FLOAT, &m_viewAngleDeg,
               " label='viewAngle' min=30 max=90 step=0.1 help='viewAngle' group='Third Person Camera' ");


    //
    // Scene parameters
    //
    TwAddVarRW(m_pBar, "cube scale", TW_TYPE_FLOAT, &m_scene.m_cubeScale, 
               " label='cube scale' min=1 max=20 step=1.0 group=Scene ");
    TwAddVarRW(m_pBar, "amplitude", TW_TYPE_FLOAT, &m_scene.m_amplitude, 
               " label='amplitude' min=0 max=2 step=0.01 group=Scene ");

    TwAddButton(m_pBar, "Reset Eye Position", ResetEyePositionCB, this,
               " label='Reset Eye Position' group='Scene' ");

    TwAddVarCB(m_pBar, "Eye height", TW_TYPE_FLOAT,
        SetEyeHeightCallback, GetEyeHeightCallback, this,
        " label='Eye height' min=0.25 max=16.0 step=0.01 group=Scene ");

    int opened = 0;
    TwSetParam(m_pBar, "Scene", "opened", TW_PARAM_INT32, 1, &opened);
    TwSetParam(m_pBar, "Camera", "opened", TW_PARAM_INT32, 1, &opened);

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

void AntOculusAppSkeleton::display(bool isControl, OVRkill::DisplayMode mode)
{
    OculusAppSkeleton::display(isControl, mode);

#ifdef USE_ANTTWEAKBAR
    // Displaying the maximized tweakbar is surprisingly not painfully obstrusive
    // on a mirrored 1920x1080. It can also be minimized.
    if (isControl)
    {
        TwRefreshBar(m_pBar);
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
