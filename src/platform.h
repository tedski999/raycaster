#ifndef RAYCASTER_PLATFORM_H
#define RAYCASTER_PLATFORM_H

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
