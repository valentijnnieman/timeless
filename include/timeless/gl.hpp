#pragma once

// OpenGL headers for the platform: OpenGL ES 3 on the web and on iOS (where
// the shaders, written for WebGL, run as they are), desktop GL through glad
// everywhere else.
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(__EMSCRIPTEN__)
#include <GL/gl.h>
#include <GLES3/gl3.h>
#elif TARGET_OS_IOS
#define GLES_SILENCE_DEPRECATION
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#define TIMELESS_GLES 1
#else
#include <glad/glad.h>
#endif

#if defined(__EMSCRIPTEN__)
#define TIMELESS_GLES 1
#endif

namespace TE {
// The framebuffer that reaches the screen: bind this, not 0, to draw to the
// window. It is 0 on desktop and the web, but on iOS SDL renders through a
// framebuffer object of its own, which WindowManager reads back after making
// the GL context current.
inline GLuint screen_framebuffer = 0;
} // namespace TE
