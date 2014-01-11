// basicplane.frag

varying vec2 vfTexCoord;

float pi = 3.14159265358979323846;

void main()
{
    float freq = 16.0 * pi;
    float lum = round(0.5 + 2.0*sin(freq*vfTexCoord.x) * sin(freq*vfTexCoord.y));
    gl_FragColor = vec4(vec3(lum), 1.0);
}
