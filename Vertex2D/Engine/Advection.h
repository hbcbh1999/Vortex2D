//
//  Advection.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 08/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Advection__
#define __Vertex2D__Advection__

#include "Size.h"
#include "Quad.h"
#include "RenderTexture.h"
#include "Reader.h"

namespace Fluid
{

class Boundaries;

class Advection
{
public:
    Advection(Dimensions dimensions, float dt);

    void RenderVelocity(const std::vector<Renderer::Drawable*> & objects);
    void RenderDensity(const std::vector<Renderer::Drawable*> & objects);

    Renderer::Reader GetVelocityReader();
    Renderer::Sprite & GetDensity();

    void Advect();

    friend class Boundaries;
private:
    void Advect(Renderer::PingPong & renderTexture, Renderer::Program & program);

    Dimensions mDimensions;
    Renderer::Quad mQuad;
    Renderer::PingPong mVelocity;
    Renderer::PingPong mDensity;

    Renderer::Program mAdvectShader;
    Renderer::Program mAdvectDensityShader;

    Renderer::Sprite mDensitySprite;
};


}

#endif /* defined(__Vertex2D__Advection__) */
