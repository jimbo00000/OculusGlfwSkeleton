// Scene.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <stdlib.h>
#include <GL/glew.h>

///@brief The Scene class renders everything in the VR world that will be the same
/// in the Oculus and Control windows. The RenderForOneEye function is the display entry point.
class Scene
{
public:
    Scene();
    virtual ~Scene();

    void initGL();
    void RenderForOneEye(const float* pMview, const float* pPersp) const;

protected:
    void DrawColorCube() const;
    void DrawGrid() const;
    void DrawOrigin() const;
    void DrawScene(const float* pMview, const float* pPersp) const;

protected:
    void _DrawBouncingCubes(const float* pMview) const;
    void _DrawScenePlanes(const float* pMview) const;

    GLuint m_progBasic;
    GLuint m_progPlane;

public:
    /// Scene animation state
    float m_phaseVal;
    float m_cubeScale;
    float m_amplitude;

private: // Disallow copy ctor and assignment operator
    Scene(const Scene&);
    Scene& operator=(const Scene&);
};
