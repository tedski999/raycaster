#ifndef RC_PLATFORM_H
#define RC_PLATFORM_H

#define PI 3.14159265358979323846
#define DEG2RAD(degrees) (degrees * PI / 180)
#define RAD2DEG(radians) (radians * 180 / PI)

#if (defined linux || defined _linux || defined __linux__)
#define RC_LINUX
#elif (defined _WIN32 || defined _WIN64)
#define RC_WINDOWS
#else
#error Unsupported/Unrecognised Platform
#endif

#define RC_OPENGL_VERSION_MAJOR 3
#define RC_OPENGL_VERSION_MINOR 2

#endif
