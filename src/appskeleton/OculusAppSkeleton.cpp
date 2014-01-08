// OculusAppSkeleton.cpp

#include "OculusAppSkeleton.h"
#include "VectorMath.h"

#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Draw_Helpers.h"
#include "GL/ShaderFunctions.h"
#include "Logger.h"

OculusAppSkeleton::OculusAppSkeleton()
: AppSkeleton()
// The world RHS coordinate system is defines as follows (as seen in perspective view):
//  Y - Up
//  Z - Back
//  X - Right
, UpVector(0.0f, 1.0f, 0.0f)
, ForwardVector(0.0f, 0.0f, -1.0f)
, RightVector(1.0f, 0.0f, 0.0f)
, YawInitial(3.141592f) /// We start out looking in the positive Z (180 degree rotation).
, Sensitivity(1.0f)
, MoveSpeed(3.0f) // m/s
, m_standingHeight(1.78f) /// From Oculus SDK p.13: 1.78m ~= 5'10"
, m_crouchingHeight(0.6f)
, EyePos(0.0f, m_standingHeight, -5.0f)
, EyeYaw(YawInitial)
, EyePitch(0)
, EyeRoll(0)
, LastSensorYaw(0)
, FollowCamDisplacement(0, 1.0f, 3.0f)
, FollowCamPos(EyePos + FollowCamDisplacement)
, m_viewAngleDeg(45.0) ///< For the no HMD case
, preferredGamepadID(0)
, swapGamepadRAxes(false)
, which_button(-1)
, modifier_mode(0)
, m_scene()
, m_avatarProg(0)
, m_displaySceneInControl(true)
{
    memset(m_keyStates, 0, GLFW_KEY_LAST*sizeof(int));
}

OculusAppSkeleton::~OculusAppSkeleton()
{
    glDeleteProgram(m_avatarProg);
    m_ok.DestroyOVR();
    glfwTerminate();
}

bool OculusAppSkeleton::initGL(int argc, char **argv)
{
    bool ret = AppSkeleton::initGL(argc, argv); /// calls _InitShaders

    LOG_INFO("Initializing shaders.");
    {
        m_scene.initGL();
        m_avatarProg = makeShaderByName("avatar");
    }
    m_ok.CreateShaders();
    m_ok.CreateRenderBuffer();

    return ret;
}

bool OculusAppSkeleton::initVR(bool fullScreen)
{
    m_ok.InitOVR();
    m_ok.SetDisplayMode(OVRkill::StereoWithDistortion);

    return true;
}

///@brief Check out what joysticks we have and select a preferred one
bool OculusAppSkeleton::initJoysticks()
{
    for (int i=GLFW_JOYSTICK_1; i<GLFW_JOYSTICK_16; ++i)
    {
        const int present = glfwJoystickPresent(i);
        if (present == GL_TRUE)
        {
            /// Nostromo:                   6 axes, 24 buttons
            /// Gravis Gamepad Pro:         2 axes, 10 buttons
            /// Generic wireless dualshock: 4 axes, 12 buttons
            /// Eliminator Aftershock:      6 axes, 10 buttons
            int numAxes = 0;
            int numButs = 0;
            glfwGetJoystickAxes(i, &numAxes);
            glfwGetJoystickButtons(i, &numButs);
            printf("Joystick %d:  %d axes, %d buttons\n", i, numAxes, numButs);

            /// This just finds the first available joystick.
            if ( (numAxes == 2) && (numButs == 10))
            {
                preferredGamepadID = i;
                swapGamepadRAxes = false;
            }
            else if ( (numAxes == 6) && (numButs == 10))
            {
                preferredGamepadID = i;
                swapGamepadRAxes = false;
            }
            else if ( (numAxes == 4) && (numButs == 12))
            {
                preferredGamepadID = i;
                swapGamepadRAxes = true;
            }
        }
    }
    return true;
}

/// HandleGlfwJoystick - translates joystick states into movement vectors.
void OculusAppSkeleton::HandleGlfwJoystick()
{
    const int joyID = preferredGamepadID;
    int joyStick1Present = GL_FALSE;
    joyStick1Present = glfwJoystickPresent(joyID);
    if (joyStick1Present == GL_TRUE)
    {
        int numAxes = 0;
        int numButs = 0;
        glfwGetJoystickAxes(joyID, &numAxes);
        glfwGetJoystickButtons(joyID, &numButs);

        int retAxes = 0;
        const float* joy1pos = glfwGetJoystickAxes(joyID, &retAxes);

        int retButs = 0;
        const unsigned char* joy1but = glfwGetJoystickButtons(joyID, &retButs);

        GamepadMove = OVR::Vector3f(0,0,0);
        if (retAxes > 0)
        {
            float padLx = joy1pos[0];
            float padLy = joy1pos[1];
            float padRx = joy1pos[2];
            float padRy = joy1pos[3];

            if (swapGamepadRAxes)
            {
                float temp = padRx;
                padRx = padRy;
                padRy = temp;
            }

            const float threshold = 0.2f;
            if (fabs(padLx) < threshold)
                padLx = 0.0f;
            if (fabs(padLy) < threshold)
                padLy = 0.0f;
            if (fabs(padRx) < threshold)
                padRx = 0.0f;
            if (fabs(padRy) < threshold)
                padRy = 0.0f;

            GamepadRotate = OVR::Vector3f(2 * padLx, -2 * padLy,  0);
            GamepadMove  += OVR::Vector3f(2 * padRy, 0         , 2 * padRx);
        }

        if (retButs > 0)
        {
            float joy1buts[4] = {
                joy1but[0] == GLFW_PRESS ? -1.0f : 0.0f,
                joy1but[1] == GLFW_PRESS ? -1.0f : 0.0f,
                joy1but[2] == GLFW_PRESS ? 1.0f : 0.0f,
                joy1but[3] == GLFW_PRESS ? 1.0f : 0.0f,
            };
            if (swapGamepadRAxes)
            {
                float temp = -joy1buts[0];
                joy1buts[0] = -joy1buts[3];
                joy1buts[3] = temp;

                temp = -joy1buts[2];
                joy1buts[2] = -joy1buts[1];
                joy1buts[1] = temp;
            }
            float padLx = joy1buts[0] + joy1buts[2];
            float padLy = joy1buts[1] + joy1buts[3];

            GamepadMove += OVR::Vector3f(padLx * padLx * (padLx > 0 ? 1 : -1),
                                         0,
                                         padLy * padLy * (padLy > 0 ? -1 : 1));

            /// Two right shoulder buttons are [5] and [7] on gravis Gamepad pro
            if (retButs > 7)
            {
                /// Top shoulder button rises, bottom lowers
                float joy1shoulderbuts[4] = {
                    joy1but[4] == GLFW_PRESS ? 1.0f : 0.0f,
                    joy1but[5] == GLFW_PRESS ? 1.0f : 0.0f,
                    joy1but[6] == GLFW_PRESS ? -1.0f : 0.0f,
                    joy1but[7] == GLFW_PRESS ? -1.0f : 0.0f,
                };
                float padLup   = joy1shoulderbuts[0] + joy1shoulderbuts[1];
                float padLdown = joy1shoulderbuts[2] + joy1shoulderbuts[3];
                padLup += padLdown;

                GamepadMove += OVR::Vector3f(0,
                                             padLup * padLup * (padLup > 0 ? 1 : -1),
                                             0);
            }
        }
    }
}


/// Handle WASD keys to move camera
void OculusAppSkeleton::HandleKeyboardMovement()
{
    /// Handle keyboard movement(WASD keys)
    KeyboardMove = OVR::Vector3f(0.0f, 0.0f, 0.0f);
    if (m_keyStates['W'] != GLFW_RELEASE)
    {
        KeyboardMove += OVR::Vector3f(0.0f, 0.0f, -1.0f);
    }
    if (m_keyStates['S'] != GLFW_RELEASE)
    {
        KeyboardMove += OVR::Vector3f(0.0f, 0.0f, 1.0f);
    }
    if (m_keyStates['A'] != GLFW_RELEASE)
    {
        KeyboardMove += OVR::Vector3f(-1.0f, 0.0f, 0.0f);
    }
    if (m_keyStates['D'] != GLFW_RELEASE)
    {
        KeyboardMove += OVR::Vector3f(1.0f, 0.0f, 0.0f);
    }
}


/// Handle input's influence on orientation variables.
void OculusAppSkeleton::AccumulateInputs(float dt)
{
    // Handle Sensor motion.
    // We extract Yaw, Pitch, Roll instead of directly using the orientation
    // to allow "additional" yaw manipulation with mouse/controller.
    if (m_ok.SensorActive())
    {
        OVR::Quatf    hmdOrient = m_ok.GetOrientation();
        float    yaw = 0.0f;

        hmdOrient.GetEulerAngles<OVR::Axis_Y, OVR::Axis_X, OVR::Axis_Z>(&yaw, &EyePitch, &EyeRoll);

        EyeYaw += (yaw - LastSensorYaw);
        LastSensorYaw = yaw;
    }


    // Gamepad rotation.
    EyeYaw -= GamepadRotate.x * dt;

    if (!m_ok.SensorActive())
    {
        // Allow gamepad to look up/down, but only if there is no Rift sensor.
        EyePitch -= GamepadRotate.y * dt;
        EyePitch -= MouseRotate.y * dt;
        EyeYaw   -= MouseRotate.x * dt;

        const float maxPitch = ((3.1415f/2)*0.98f);
        if (EyePitch > maxPitch)
            EyePitch = maxPitch;
        if (EyePitch < -maxPitch)
            EyePitch = -maxPitch;
    }

    if (GamepadMove.LengthSq() > 0)
    {
        OVR::Matrix4f yawRotate = OVR::Matrix4f::RotationY(EyeYaw);
        OVR::Vector3f orientationVector = yawRotate.Transform(GamepadMove);
        orientationVector *= MoveSpeed * dt;
        EyePos += orientationVector;
    }

    if (MouseMove.LengthSq() > 0)
    {
        OVR::Matrix4f yawRotate = OVR::Matrix4f::RotationY(EyeYaw);
        OVR::Vector3f orientationVector = yawRotate.Transform(MouseMove);
        orientationVector *= MoveSpeed * dt;
        EyePos += orientationVector;
    }

    if (KeyboardMove.LengthSq() > 0)
    {
        OVR::Matrix4f yawRotate = OVR::Matrix4f::RotationY(EyeYaw);
        OVR::Vector3f orientationVector = yawRotate.Transform(KeyboardMove);
        orientationVector *= MoveSpeed * dt;
        EyePos += orientationVector;
    }
}

/// From the OVR SDK.
void OculusAppSkeleton::AssembleViewMatrix()
{
    // Rotate and position m_oculusView Camera, using YawPitchRoll in BodyFrame coordinates.
    // 
    OVR::Matrix4f rollPitchYaw = GetRollPitchYaw();
    OVR::Vector3f up      = rollPitchYaw.Transform(UpVector);
    OVR::Vector3f forward = rollPitchYaw.Transform(ForwardVector);

    // Minimal head modelling.
    float headBaseToEyeHeight     = 0.15f;  // Vertical height of eye from base of head
    float headBaseToEyeProtrusion = 0.09f;  // Distance forward of eye from base of head

    OVR::Vector3f eyeCenterInHeadFrame(0.0f, headBaseToEyeHeight, -headBaseToEyeProtrusion);
    OVR::Vector3f shiftedEyePos = EyePos + rollPitchYaw.Transform(eyeCenterInHeadFrame);
    shiftedEyePos.y -= eyeCenterInHeadFrame.y; // Bring the head back down to original height

    m_oculusView = OVR::Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + forward, up); 

    // This is what transformation would be without head modeling.
    // m_oculusView = Matrix4f::LookAtRH(EyePos, EyePos + forward, up);


    /// Set up a third person(or otherwise) view for control window
    {
        OVR::Vector3f txFollowDisp = rollPitchYaw.Transform(FollowCamDisplacement);
        FollowCamPos = EyePos + txFollowDisp;
        m_controlView = OVR::Matrix4f::LookAtRH(FollowCamPos, EyePos, up);
    }
}

void OculusAppSkeleton::mouseDown(int button, int state, int x, int y)
{
    which_button = button;
    oldx = newx;
    oldy = newy;
    if (state == GLFW_RELEASE)
    {
        which_button = -1;
    }
}

void OculusAppSkeleton::mouseMove(int x, int y)
{
    const float thresh = 32;

    oldx = newx;
    oldy = newy;
    newx = x;
    newy = y;
    const int mmx = x-oldx;
    const int mmy = y-oldy;
    const float rx = (float)mmx/thresh;
    const float ry = (float)mmy/thresh;
    
    MouseRotate = OVR::Vector3f(0, 0, 0);
    MouseMove   = OVR::Vector3f(0, 0, 0);
    
    if (which_button == GLFW_MOUSE_BUTTON_LEFT)
    {
        MouseRotate = OVR::Vector3f(rx, ry, 0);
    }
    else if (which_button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        MouseMove   = OVR::Vector3f(rx * rx * (rx > 0 ? 1 : -1),
                                    0,
                                    ry * ry * (ry > 0 ? 1 : -1));
    }
}

void OculusAppSkeleton::mouseWheel(int x, int y)
{
    const float sens = 0.1f;
    const float angleRatio = 0.2f;
    FollowCamDisplacement.z += (float)y * sens;
    FollowCamDisplacement.y = angleRatio * FollowCamDisplacement.z;
}

void OculusAppSkeleton::keyboard(int key, int action, int x, int y)
{
    if ((key > -1) && (key <= GLFW_KEY_LAST))
    {
        m_keyStates[key] = action;
        printf("key ac  %d %d\n", key, action);
    }

    if (key == GLFW_KEY_LEFT_SHIFT)
    {
        if (action == GLFW_PRESS)
        {
            EyePos.y = m_crouchingHeight;
        }
        else if (action == GLFW_RELEASE)
        {
            EyePos.y = m_standingHeight;
        }
    }

    if (action != GLFW_PRESS)
    {
        return;
    }

    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        exit(0);
        break;

    case GLFW_KEY_F1:
        m_ok.SetDisplayMode(OVRkill::SingleEye);
        break;

    case GLFW_KEY_F2:
        m_ok.SetDisplayMode(OVRkill::Stereo);
        break;

    case GLFW_KEY_F3:
        m_ok.SetDisplayMode(OVRkill::StereoWithDistortion);
        break;

    case GLFW_KEY_F4:
        m_ok.SetDisplayMode(OVRkill::SingleEye);
        break;

    case 'Z':
        m_displaySceneInControl = !m_displaySceneInControl;
        break;

    default:
        printf("%d: %c\n", key, (char)key);
        break;
    }

    fflush(stdout);
}

void OculusAppSkeleton::resize(int w, int h)
{
    m_windowWidth = w;
    m_windowHeight = h;
    AppSkeleton::resize(w,h);
}


///////////////////////////////////////////////////////////////////////////////


/// Handle animations, joystick states and viewing matrix
void OculusAppSkeleton::timestep(float dt)
{
    m_scene.m_phaseVal += dt;

    const float frequency = 5.0f;
    const float amplitude = 0.2f;

    HandleGlfwJoystick();
    HandleKeyboardMovement();
    AccumulateInputs(dt);
    AssembleViewMatrix();
    m_ok.UpdateEyeParams();
}


///////////////////////////////////////////////////////////////////////////////


/// Set up view matrices, then draw scene
void OculusAppSkeleton::display(bool useOculus) const
{
    /// This may save us some frame rate
    if (!useOculus && !m_displaySceneInControl)
    {
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }

    glEnable(GL_DEPTH_TEST);

    m_ok.BindRenderBuffer();
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const int fboWidth = m_ok.GetRenderBufferWidth();
        const int fboHeight = m_ok.GetRenderBufferHeight();
        const int halfWidth = fboWidth/2;
        if (useOculus)
        {
            const OVR::HMDInfo& hmd = m_ok.GetHMD();
            // Compute Aspect Ratio. Stereo mode cuts width in half.
            float aspectRatio = float(hmd.HResolution * 0.5f) / float(hmd.VResolution);

            // Compute Vertical FOV based on distance.
            float halfScreenDistance = (hmd.VScreenSize / 2);
            float yfov = 2.0f * atan(halfScreenDistance/hmd.EyeToScreenDistance);

            // Post-projection viewport coordinates range from (-1.0, 1.0), with the
            // center of the left viewport falling at (1/4) of horizontal screen size.
            // We need to shift this projection center to match with the lens center.
            // We compute this shift in physical units (meters) to correct
            // for different screen sizes and then rescale to viewport coordinates.
            float viewCenterValue = hmd.HScreenSize * 0.25f;
            float eyeProjectionShift = viewCenterValue - hmd.LensSeparationDistance * 0.5f;
            float projectionCenterOffset = 4.0f * eyeProjectionShift / hmd.HScreenSize;

            // Projection matrix for the "center eye", which the left/right matrices are based on.
            OVR::Matrix4f projCenter = OVR::Matrix4f::PerspectiveRH(yfov, aspectRatio, 0.3f, 1000.0f);
            OVR::Matrix4f projLeft   = OVR::Matrix4f::Translation(projectionCenterOffset, 0, 0) * projCenter;
            OVR::Matrix4f projRight  = OVR::Matrix4f::Translation(-projectionCenterOffset, 0, 0) * projCenter;
        
            // m_oculusView transformation translation in world units.
            float halfIPD = hmd.InterpupillaryDistance * 0.5f;
            OVR::Matrix4f viewLeft = OVR::Matrix4f::Translation(halfIPD, 0, 0) * m_oculusView;
            OVR::Matrix4f viewRight= OVR::Matrix4f::Translation(-halfIPD, 0, 0) * m_oculusView;

            glViewport(0        ,0,(GLsizei)halfWidth, (GLsizei)fboHeight);
            glScissor (0        ,0,(GLsizei)halfWidth, (GLsizei)fboHeight);
            m_scene.RenderForOneEye(viewLeft, projLeft);

            glViewport(halfWidth,0,(GLsizei)halfWidth, (GLsizei)fboHeight);
            glScissor (halfWidth,0,(GLsizei)halfWidth, (GLsizei)fboHeight);
            m_scene.RenderForOneEye(viewRight, projRight);
        }
        else
        {
            /// Set up our 3D transformation matrices
            /// Remember DX and OpenGL use transposed conventions. And doesn't DX use left-handed coords?
            OVR::Matrix4f mview = m_controlView;
            OVR::Matrix4f persp = OVR::Matrix4f::PerspectiveRH(
                m_viewAngleDeg * M_PI / 180.0f,
                (float)m_windowWidth/(float)m_windowHeight,
                0.004f,
                500.0f);

            glViewport(0,0,(GLsizei)fboWidth, (GLsizei)fboHeight);
            m_scene.RenderForOneEye(mview, persp);


            /// Render avatar of Oculus user
            //if (UseFollowCam)
            const GLuint prog = m_avatarProg;
            glUseProgram(prog);
            {
                OVR::Matrix4f rollPitchYaw = GetRollPitchYaw();
                OVR::Matrix4f eyetx = mview
                    * OVR::Matrix4f::Translation(EyePos.x, EyePos.y, EyePos.z)
                    * rollPitchYaw;

                glUniformMatrix4fv(getUniLoc(prog, "mvmtx"), 1, false, &eyetx.Transposed().M[0][0]);
                glUniformMatrix4fv(getUniLoc(prog, "prmtx"), 1, false, &persp.Transposed().M[0][0]);

                glLineWidth(4.0f);
                DrawOriginLines();
                const float aspect = (float)GetOculusWidth() / (float)GetOculusHeight();
                DrawViewFrustum(aspect);
                glLineWidth(1.0f);
            }
        }
    }
    m_ok.UnBindRenderBuffer();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    const OVRkill::PostProcessType post = useOculus ? OVRkill::PostProcess_Distortion : OVRkill::PostProcess_None;
    m_ok.PresentFbo(post);
}
