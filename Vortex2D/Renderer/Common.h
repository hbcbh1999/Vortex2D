//
//  Common.h
//  Vortex
//
//  Created by Maximilian Maldacker on 07/04/2014.
//
//

#ifndef Vortex_Common_h
#define Vortex_Common_h
 
#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <OpenGL/gl3.h>
#endif

#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#define GLSL(src) "#version 150 core\n" #src

#endif