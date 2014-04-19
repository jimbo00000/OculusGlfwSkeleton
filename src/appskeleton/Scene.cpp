// Scene.cpp

#include "Scene.h"

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

#include "MatrixMath.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include "GL/ShaderFunctions.h"
#include "Logger.h"

Scene::Scene()
: m_progBasic(0)
, m_progPlane(0)
, m_phaseVal(0.0f)
, m_cubeScale(1.0f)
, m_amplitude(1.0f)
{
}

Scene::~Scene()
{
    glDeleteProgram(m_progBasic);
    glDeleteProgram(m_progPlane);
}

void Scene::initGL()
{
    m_progBasic = makeShaderByName("basic");
    m_progPlane = makeShaderByName("basicplane");
}

/// Draw an RGB color cube
void Scene::DrawColorCube() const
{
    const float3 minPt = {0,0,0};
    const float3 maxPt = {1,1,1};
    const float3 verts[] = {
        minPt,
        {maxPt.x, minPt.y, minPt.z},
        {maxPt.x, maxPt.y, minPt.z},
        {minPt.x, maxPt.y, minPt.z},
        {minPt.x, minPt.y, maxPt.z},
        {maxPt.x, minPt.y, maxPt.z},
        maxPt,
        {minPt.x, maxPt.y, maxPt.z}
    };
    const uint3 quads[] = {
        {0,3,2}, {1,0,2}, // ccw
        {4,5,6}, {7,4,6},
        {1,2,6}, {5,1,6},
        {2,3,7}, {6,2,7},
        {3,0,4}, {7,3,4},
        {0,1,5}, {4,0,5},
    };

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES,
                   6*3*2, // 6 triangle pairs
                   GL_UNSIGNED_INT,
                   &quads[0]);
}

/// Utility function to draw colored line segments along unit x,y,z axes
void Scene::DrawOrigin() const
{
    const float3 minPt = {0,0,0};
    const float3 maxPt = {1,1,1};
    const float3 verts[] = {
        {0,0,0},
        {1,0,0},
        {0,1,0},
        {0,0,1},
    };
    const unsigned int lines[] = {
        0,1,
        0,2,
        0,3,
    };

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_LINES,
                   3*2,
                   GL_UNSIGNED_INT,
                   &lines[0]);
}

/// Draw a circle of color cubes(why not)
void Scene::_DrawBouncingCubes(const float* pMview) const
{
    float sinmtx[16];
    const int numCubes = 12;
    for (int i=0; i<numCubes; ++i)
    {
        const float radius = 15.0f;
        const float posPhase = 2.0f * (float)M_PI * (float)i / (float)numCubes;
        const float3 cubePosition = {radius * sin(posPhase), 0.0f, radius * cos(posPhase)};

        const float frequency = 3.0f;
        const float amplitude = m_amplitude;
        float oscVal = amplitude * sin(frequency * (m_phaseVal + posPhase));

        memcpy(sinmtx, pMview, 16*sizeof(float));
        glhTranslate(sinmtx, cubePosition.x, oscVal, cubePosition.z);

        const float scale = m_cubeScale;
        glhScale(sinmtx, scale, scale, scale);

        glUniformMatrix4fv(getUniLoc(m_progBasic, "mvmtx"), 1, false, sinmtx);
        DrawColorCube();
    }
}


void DrawPlane()
{
    const float3 minPt = {-10.0f, 0.0f, -10.0f};
    const float3 maxPt = {10.0f, 0.0f, 10.0f};
    const float3 verts[] = {
        minPt.x, minPt.y, minPt.z,
        minPt.x, minPt.y, maxPt.z,
        maxPt.x, minPt.y, maxPt.z,
        maxPt.x, minPt.y, minPt.z,
    };
    const float2 texs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };
    const uint3 quads[] = {
        {0,3,2}, {1,0,2}, // ccw
    };

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texs);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES,
                   3*2, // 2 triangle pairs
                   GL_UNSIGNED_INT,
                   &quads[0]);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void Scene::_DrawScenePlanes(const float* pMview) const
{
    DrawPlane(); // matrix uniform is already set by caller

    float mv[16];
    memcpy(mv, pMview, 16*sizeof(float));
    const float ceilHeight = 3.0f;
    glhTranslate(mv, 0.0f, ceilHeight, 0.0f);

    glUniformMatrix4fv(getUniLoc(m_progBasic, "mvmtx"), 1, false, mv);
    DrawPlane();
}


/// Draw the scene(matrices have already been set up).
void Scene::DrawScene(const float* pMview, const float* pPersp) const
{
    glUseProgram(m_progPlane);
    {
        glUniformMatrix4fv(getUniLoc(m_progPlane, "mvmtx"), 1, false, pMview);
        glUniformMatrix4fv(getUniLoc(m_progPlane, "prmtx"), 1, false, pPersp);

        _DrawScenePlanes(pMview);
    }
    glUseProgram(0);

    glUseProgram(m_progBasic);
    {
        glUniformMatrix4fv(getUniLoc(m_progBasic, "mvmtx"), 1, false, pMview);
        glUniformMatrix4fv(getUniLoc(m_progBasic, "prmtx"), 1, false, pPersp);

        _DrawBouncingCubes(pMview);
    }
    glUseProgram(0);
}


void Scene::RenderForOneEye(const OVR::Matrix4f& mview, const OVR::Matrix4f& persp) const
{
    const float* pMview = &mview.Transposed().M[0][0];
    const float* pPersp = &persp.Transposed().M[0][0];

    DrawScene(pMview, pPersp);
}
