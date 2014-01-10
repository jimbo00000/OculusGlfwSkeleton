// Scene.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <stdlib.h>
#include <GL/glew.h>

#include "OVRkill/OVRkill.h"

///@brief The Scene class renders everything in the VR world that will be the same
/// in the Oculus and Control windows. The RenderForOneEye function is the display entry point.
class Scene
{
public:
    Scene();
    virtual ~Scene();

    void initGL();
    void RenderForOneEye(const OVR::Matrix4f& mview, const OVR::Matrix4f& persp) const;

protected:
    void DrawColorCube() const;
    void DrawGrid() const;
    void DrawOrigin() const;
    void DrawScene(const OVR::Matrix4f& mview, const OVR::Matrix4f& persp) const;

protected:
    void _DrawBouncingCubes(const OVR::Matrix4f& mview) const;
    void _DrawSceneWireFrame(const OVR::Matrix4f& mview) const;
    void _DrawScenePlanes(const OVR::Matrix4f& mview) const;

    GLuint m_progBasic;

public:
    /// Scene animation state
    float m_phaseVal;
    float m_cubeScale;
    float m_amplitude;

private: // Disallow copy ctor and assignment operator
    Scene(const Scene&);
    Scene& operator=(const Scene&);
};
