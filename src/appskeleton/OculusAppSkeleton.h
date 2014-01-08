// OculusAppSkeleton.h

#pragma once

#ifdef __APPLE__
#include "opengl/gl.h"
#endif

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <stdlib.h>
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#ifdef USE_CUDA
#else
#  include "vector_make_helpers.h"
#endif

#include "OVR.h"
#include "OVR_Shaders.h"
#include "Util/Util_Render_Stereo.h"

#include "AppSkeleton.h"
#include "Scene.h"
#include "OVRkill/OVRkill.h"
#include "Timer.h"

///@brief Encapsulates as much of the VR viewer state as possible,
/// pushing all viewer-independent stuff to Scene.
/// display takes a bool to indicate Oculus window or Control.
class OculusAppSkeleton : public AppSkeleton
{
public:
    OculusAppSkeleton();
    virtual ~OculusAppSkeleton();

    virtual void display(bool useOculus=false) const;
    virtual void mouseDown(int button, int state, int x, int y);
    virtual void mouseMove(int x, int y);
    virtual void mouseWheel(int x, int y);
    virtual void keyboard(int key, int action, int x, int y);
    virtual void resize(int w, int h);
    virtual bool initVR(bool fullScreen);
    virtual bool initJoysticks();
    virtual bool initGL(int argc, char **argv);
    virtual void timestep(float dt);

    int GetOculusWidth() const { return m_ok.GetOculusWidth(); }
    int GetOculusHeight() const { return m_ok.GetOculusHeight(); }

    OVR::Matrix4f GetRollPitchYaw() const {
        return OVR::Matrix4f::RotationY(EyeYaw) *
               OVR::Matrix4f::RotationX(EyePitch) *
               OVR::Matrix4f::RotationZ(EyeRoll);
    }

protected:
    void HandleGlfwJoystick();
    void HandleKeyboardMovement();
    void AccumulateInputs(float dt);
    void AssembleViewMatrix();

    /// VR view parameters
    const OVR::Vector3f UpVector;
    const OVR::Vector3f ForwardVector;
    const OVR::Vector3f RightVector;
    const float  YawInitial;
    const float  Sensitivity;
    const float  MoveSpeed;
    const float  m_standingHeight;
    const float  m_crouchingHeight;
    OVR::Vector3f EyePos;
    float EyeYaw;
    float EyePitch;
    float EyeRoll;
    float LastSensorYaw;
    OVR::Vector3f FollowCamDisplacement;
    OVR::Vector3f FollowCamPos;
    float m_viewAngleDeg; ///< For the no HMD case

    OVR::Matrix4f  m_oculusView; /// World modelview matrix for Oculus
    OVR::Matrix4f  m_controlView; /// World modelview matrix for Control window

    /// Viewing parameters fed in from joystick and the HMD
    OVR::Vector3f  GamepadMove, GamepadRotate;
    OVR::Vector3f  MouseMove, MouseRotate;
    OVR::Vector3f  KeyboardMove, KeyboardRotate;

    /// For choosing one connected joystick
    int preferredGamepadID;
    bool swapGamepadRAxes;

    /// Mouse motion internal state
    int oldx, oldy, newx, newy;
    int which_button;
    int modifier_mode;
    int m_keyStates[GLFW_KEY_LAST];

    OVRkill m_ok;
    Scene   m_scene;

    GLuint m_avatarProg;
    bool   m_displaySceneInControl;

public:
    RiftDistortionParams  m_riftDist;

private: // Disallow copy ctor and assignment operator
    OculusAppSkeleton(const OculusAppSkeleton&);
    OculusAppSkeleton& operator=(const OculusAppSkeleton&);
};
