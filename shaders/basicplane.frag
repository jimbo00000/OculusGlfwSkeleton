// basicplane.frag
// Apply a simple black and white checkerboard pattern to a quad
// with texture coordinates in the unit interval.

#version 130

in vec2 vfTexCoord;
out vec4 FragColor;

float pi = 3.14159265358979323846;

void main()
{
    float freq = 16.0 * pi;
    float lum = round(0.5 + 2.0*sin(freq*vfTexCoord.x) * sin(freq*vfTexCoord.y));
    FragColor = vec4(vec3(lum), 1.0);
}
