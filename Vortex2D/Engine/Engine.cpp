//
//  Engine.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#include "Engine.h"
#include "Disable.h"
#include "LevelSet.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

namespace Fluid
{

const char * DivFrag = GLSL(
    uniform sampler2D u_texture; // this is the velocity texture
    uniform sampler2D u_obstacles;
    uniform sampler2D u_obstacles_velocity;

    in vec2 v_texCoord;
    out vec4 out_color;

    void main()
    {
        vec2  uv  = texture(u_texture, v_texCoord).xy;
        float uxp = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
        float uxn = uv.x;
        float vyp = textureOffset(u_texture, v_texCoord, ivec2(0,1)).y;
        float vyn = uv.y;

        float c   = 1.0 - texture(u_obstacles, v_texCoord).x;
        float cxp = 1.0 - textureOffset(u_obstacles, v_texCoord, ivec2(2,0)).x;
        float cxn = 1.0 - textureOffset(u_obstacles, v_texCoord, ivec2(-2,0)).x;
        float cyp = 1.0 - textureOffset(u_obstacles, v_texCoord, ivec2(0,2)).x;
        float cyn = 1.0 - textureOffset(u_obstacles, v_texCoord, ivec2(0,-2)).x;

        float solid_uxp = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(1,0)).x;
        float solid_uxn = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(-1,0)).x;
        float solid_vyp = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,1)).y;
        float solid_vyn = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,-1)).y;

        float dx = 1.0;
        float div = -(cxp * uxp - cxn * uxn + cyp * vyp - cyn * vyn) / dx;
        div += ((cxp-c)*solid_uxp - (cxn-c)*solid_uxn + (cyp-c)*solid_vyp - (cyn-c)*solid_vyn) / dx;
        
        //pressure, div, 0, 0
        out_color = vec4(0.0, div, 0.0, 0.0);
    }
);

const char * ProjectFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_texture; // this is the velocity texture
    uniform sampler2D u_pressure;
    uniform sampler2D u_obstacles;
    uniform sampler2D u_obstacles_velocity;
    uniform float delta;

    void main()
    {

        vec2 cell = texture(u_texture, v_texCoord).xy;

        float pxp = textureOffset(u_pressure, v_texCoord, ivec2(1,0)).x;
        float pxn = textureOffset(u_pressure, v_texCoord, ivec2(-1,0)).x;
        float pyp = textureOffset(u_pressure, v_texCoord, ivec2(0,1)).x;
        float pyn = textureOffset(u_pressure, v_texCoord, ivec2(0,-1)).x;

        float cxp = textureOffset(u_obstacles, v_texCoord, ivec2(2,0)).x;
        float cxn = textureOffset(u_obstacles, v_texCoord, ivec2(-2,0)).x;
        float cyp = textureOffset(u_obstacles, v_texCoord, ivec2(0,2)).x;
        float cyn = textureOffset(u_obstacles, v_texCoord, ivec2(0,-2)).x;

        vec2 pGrad = vec2(pxp-pxn, pyp-pyn);

        vec2 mask = vec2(1.0);
        vec2 obsV = vec2(0.0);

        if (cxp > 0.0)
        {
            mask.x = 0.0;
            obsV.x = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(1,0)).x;
        }
        if (cxn > 0.0)
        {
            mask.x = 0.0;
            obsV.x = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(-1,0)).x;
        }
        if (cyp > 0.0)
        {
            mask.y = 0.0;
            obsV.y = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,1)).y;
        }
        if (cyn > 0.0)
        {
            mask.y = 0.0;
            obsV.y = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,-1)).y;
        }
        
        float dx = 1.0;
        vec2 new_cell = cell - delta * pGrad / dx;
        out_color = vec4(mask * new_cell + obsV, 0.0, 0.0);
    }
);

const char * BoundaryMaskFrag = GLSL(
    in vec2 v_texCoord;

    uniform sampler2D u_dirichlet;
    uniform sampler2D u_neumann;

    void main()
    {
        float x = texture(u_dirichlet, v_texCoord).x;
        float y = texture(u_neumann, v_texCoord).x;

        if(x < 1.0 && y < 1.0)
        {
            discard;
        }
    }
);

const char * WeightsFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_dirichlet;
    uniform sampler2D u_neumann;
    uniform float delta;

    void main()
    {
        vec4 p;
        p.x = textureOffset(u_dirichlet, v_texCoord, ivec2(1,0)).x;
        p.y = textureOffset(u_dirichlet, v_texCoord, ivec2(-1,0)).x;
        p.z = textureOffset(u_dirichlet, v_texCoord, ivec2(0,1)).x;
        p.w = textureOffset(u_dirichlet, v_texCoord, ivec2(0,-1)).x;

        vec4 q;
        q.x = textureOffset(u_neumann, v_texCoord, ivec2(2,0)).x;
        q.y = textureOffset(u_neumann, v_texCoord, ivec2(-2,0)).x;
        q.z = textureOffset(u_neumann, v_texCoord, ivec2(0,2)).x;
        q.w = textureOffset(u_neumann, v_texCoord, ivec2(0,-2)).x;

        float dx = 1.0;
        out_color = delta * (1.0 - max(p,q)) / (dx*dx);
    }
);

const char * DiagonalsFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_texture; // this is the obstacles
    uniform float delta;

    const vec4 q = vec4(1.0);

    void main()
    {
        vec4 p;
        p.x = 1.0 - textureOffset(u_texture, v_texCoord, ivec2(2,0)).x;
        p.y = 1.0 - textureOffset(u_texture, v_texCoord, ivec2(-2,0)).x;
        p.z = 1.0 - textureOffset(u_texture, v_texCoord, ivec2(0,2)).x;
        p.w = 1.0 - textureOffset(u_texture, v_texCoord, ivec2(0,-2)).x;

        float dx = 1.0;
        out_color = vec4(delta * dot(p,q) / (dx*dx),0.0, 0.0, 0.0);
    }
);

const char * AdvectFrag = GLSL(
    uniform sampler2D u_texture;
    uniform sampler2D u_velocity;
    uniform float delta;

    in vec2 v_texCoord;
    out vec4 out_color;

    vec4 cubic(vec4 f1, vec4 f2, vec4 f3, vec4 f4, float xd)
    {
        float xd2 = xd * xd;
        float xd3 = xd2 * xd;

        return f1*(-0.5*xd  +      xd2 - 0.5*xd3) +
               f2*( 1.0     - 2.5 *xd2 + 1.5*xd3) +
               f3*( 0.5 *xd + 2.0 *xd2 - 1.5*xd3) +
               f4*(-0.5*xd2 + 0.5 *xd3);
    }

    vec4 bicubic(sampler2D u_texture, vec2 xy)
    {
        vec2 ij0 = floor(xy) - 1.0;
        vec2 ij1 = ij0 + 1.0;
        vec2 ij2 = ij1 + 1.0;
        vec2 ij3 = ij2 + 1.0;

        vec2 f = xy - ij1.xy;

        vec2 h = textureSize(u_texture, 0);
        vec2 s0 = (ij0 + 0.5) / h;
        vec2 s1 = (ij1 + 0.5) / h;
        vec2 s2 = (ij2 + 0.5) / h;
        vec2 s3 = (ij3 + 0.5) / h;

        vec4 t00 = texture(u_texture, vec2(s0.x, s0.y));
        vec4 t10 = texture(u_texture, vec2(s1.x, s0.y));
        vec4 t20 = texture(u_texture, vec2(s2.x, s0.y));
        vec4 t30 = texture(u_texture, vec2(s3.x, s0.y));
        vec4 t01 = texture(u_texture, vec2(s0.x, s1.y));
        vec4 t11 = texture(u_texture, vec2(s1.x, s1.y));
        vec4 t21 = texture(u_texture, vec2(s2.x, s1.y));
        vec4 t31 = texture(u_texture, vec2(s3.x, s1.y));
        vec4 t02 = texture(u_texture, vec2(s0.x, s2.y));
        vec4 t12 = texture(u_texture, vec2(s1.x, s2.y));
        vec4 t22 = texture(u_texture, vec2(s2.x, s2.y));
        vec4 t32 = texture(u_texture, vec2(s3.x, s2.y));
        vec4 t03 = texture(u_texture, vec2(s0.x, s3.y));
        vec4 t13 = texture(u_texture, vec2(s1.x, s3.y));
        vec4 t23 = texture(u_texture, vec2(s2.x, s3.y));
        vec4 t33 = texture(u_texture, vec2(s3.x, s3.y));

        return cubic(
                    cubic(t00, t01, t02, t03, f.y),
                    cubic(t10, t11, t12, t13, f.y),
                    cubic(t20, t21, t22, t23, f.y),
                    cubic(t30, t31, t32, t33, f.y),
                    f.x
                );
    }

    const float a = 2.0/9.0;
    const float b = 3.0/9.0;
    const float c = 4.0/9.0;

    void main(void)
    {
        vec2 pos = gl_FragCoord.xy - 0.5;

        vec2 k1 = texture(u_velocity, v_texCoord).xy;
        vec2 k2 = bicubic(u_velocity, pos - 0.5*delta*k1).xy;
        vec2 k3 = bicubic(u_velocity, pos - 0.75*delta*k2).xy;

        out_color = bicubic(u_texture, pos - a*delta*k1 - b*delta*k2 - c*delta*k3);
    }
);

const char * ExtrapolateFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_mask;
    uniform sampler2D u_velocity;

    void main()
    {
        vec2 sum = vec2(0.0);
        float count = 0.0;

        if(texture(u_mask, v_texCoord).x == 0.0)
        {
            if(textureOffset(u_mask, v_texCoord, ivec2(1,0)).x == 1.0)
            {
                sum += textureOffset(u_velocity, v_texCoord, ivec2(1,0)).xy;
                count += 1.0;
            }
            if(textureOffset(u_mask, v_texCoord, ivec2(0,1)).x == 1.0)
            {
                sum += textureOffset(u_velocity, v_texCoord, ivec2(0,1)).xy;
                count += 1.0;
            }
            if(textureOffset(u_mask, v_texCoord, ivec2(-1,0)).x == 1.0)
            {
                sum += textureOffset(u_velocity, v_texCoord, ivec2(-1,0)).xy;
                count += 1.0;
            }
            if(textureOffset(u_mask, v_texCoord, ivec2(0,-1)).x == 1.0)
            {
                sum += textureOffset(u_velocity, v_texCoord, ivec2(0,-1)).xy;
                count += 1.0;
            }

            if(count > 0)
            {
                out_color = vec4(sum/count, 0.0, 0.0);
            }
            else
            {
                out_color = vec4(texture(u_velocity, v_texCoord).xy, 0.0, 0.0);
            }
        }
        else
        {
            out_color = vec4(texture(u_velocity, v_texCoord).xy, 0.0, 0.0);
        }
    }
);

const char * ExtrapolateMaskFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_mask;

    void main()
    {
        if(texture(u_mask, v_texCoord).x == 1.0 ||
           textureOffset(u_mask, v_texCoord, ivec2(1,0)).x == 1.0 ||
           textureOffset(u_mask, v_texCoord, ivec2(0,1)).x == 1.0 ||
           textureOffset(u_mask, v_texCoord, ivec2(-1,0)).x == 1.0 ||
           textureOffset(u_mask, v_texCoord, ivec2(0,-1)).x == 1.0)
        {
            out_color = vec4(1.0, 0.0, 0.0, 0.0);
        }
        else
        {
            out_color = vec4(0.0);
        }
    }
);

Engine::Engine(Dimensions dimensions, LinearSolver * linearSolver, float dt)
    : mDimensions(dimensions)
    , mData(dimensions.Size)
    , mLinearSolver(linearSolver)
    , mVelocity(dimensions.Size, 2, true, true)
    , mDirichletBoundaries(dimensions.Size, 1)
    , mNeumannBoundaries(glm::vec2(2.0f)*dimensions.Size, 1)
    , mBoundariesVelocity(dimensions.Size, 2)
    , mExtrapolateValid(dimensions.Size, 1, true, true)
    , mDiv(Renderer::Shader::TexturePositionVert, DivFrag)
    , mProject(Renderer::Shader::TexturePositionVert, ProjectFrag)
    , mAdvect(Renderer::Shader::TexturePositionVert, AdvectFrag)
    , mExtrapolate(Renderer::Shader::TexturePositionVert, ExtrapolateFrag)
    , mExtrapolateMask(Renderer::Shader::TexturePositionVert, ExtrapolateMaskFrag)
    , mIdentity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
    , mWeights(Renderer::Shader::TexturePositionVert, WeightsFrag)
    , mDiagonals(Renderer::Shader::TexturePositionVert, DiagonalsFrag)
    , mBoundaryMask(Renderer::Shader::TexturePositionVert, BoundaryMaskFrag)
    , mSurface(dimensions.Size)
{
    mVelocity.clear();
    mDirichletBoundaries.clear();

    mNeumannBoundaries.clear();
    mNeumannBoundaries.linear();

    mBoundariesVelocity.clear();

    mExtrapolateValid.clear();

    mProject.Use().Set("u_texture", 0).Set("u_pressure", 1).Set("u_obstacles", 2).Set("u_obstacles_velocity", 3).Set("delta", dt).Unuse();
    mDiv.Use().Set("u_texture", 0).Set("u_obstacles", 1).Set("u_obstacles_velocity", 2).Unuse();
    mAdvect.Use().Set("delta", dt).Set("u_texture", 0).Set("u_velocity", 1).Unuse();
    mExtrapolate.Use().Set("u_mask", 0).Set("u_velocity", 1).Unuse();
    mExtrapolateMask.Use().Set("u_mask", 0).Unuse();
    mIdentity.Use().Set("u_texture", 0).Unuse();
    mWeights.Use().Set("u_dirichlet", 0).Set("u_neumann", 1).Set("delta", dt).Unuse();
    mDiagonals.Use().Set("u_texture", 0).Set("delta", dt).Unuse();
    mBoundaryMask.Use().Set("u_dirichlet", 0).Set("u_neumann", 1).Unuse();
}

void Engine::Solve()
{
    mData.Pressure.clear();

    RenderMask(mData.Pressure);
    RenderMask(mData.Pressure.swap());

    RenderMask(mVelocity.swap());
    RenderMask(mVelocity.swap());

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    mData.Pressure = mDiv(mVelocity, mNeumannBoundaries, mBoundariesVelocity);
    mData.Weights = mWeights(mDirichletBoundaries, mNeumannBoundaries);
    mData.Diagonal = mDiagonals(mNeumannBoundaries);

    mLinearSolver->Init(mData);
    mLinearSolver->Solve(mData);

    mVelocity.swap() = mProject(Back(mVelocity), mData.Pressure, mNeumannBoundaries, mBoundariesVelocity);

    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    mVelocity.begin();
    mSurface.Colour = glm::vec4{0.0f};
    mSurface.Render(mVelocity.Orth);
    mVelocity.end();

    Extrapolate();
    Advect(mVelocity);
}

void Engine::RenderDirichlet(const std::vector<Renderer::Drawable*> & objects)
{
    mDirichletBoundaries.begin();
    for(auto object : objects)
    {
        object->Render(mDirichletBoundaries.Orth*mDimensions.InvScale);
    }
    mDirichletBoundaries.end();
}

void Engine::RenderNeumann(const std::vector<Renderer::Drawable*> & objects)
{
    mNeumannBoundaries.begin();
    auto scaled = glm::scale(mNeumannBoundaries.Orth, glm::vec3(2.0f, 2.0f, 1.0f));
    for(auto object : objects)
    {
        object->Render(scaled*mDimensions.InvScale);
    }
    mNeumannBoundaries.end();
}

void Engine::RenderMask(Buffer & mask)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    Renderer::DisableColorMask c;

    glStencilFunc(GL_ALWAYS, 1, 0xFF); // write 1 in stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // replace value with above
    glStencilMask(0xFF); // enable stencil writing

    // clear stencil buffer
    mask.begin();
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    mask.end();

    mask = mBoundaryMask(mDirichletBoundaries, mNeumannBoundaries);

    glStencilMask(0x00); // disable stencil writing
}

void Engine::RenderVelocities(const std::vector<Renderer::Drawable*> & objects)
{
    mBoundariesVelocity.begin({0.0f, 0.0f, 0.0f, 0.0f});
    for(auto object : objects)
    {
        object->Render(mBoundariesVelocity.Orth*mDimensions.InvScale);
    }
    mBoundariesVelocity.end();
}

void Engine::RenderFluid(Fluid::LevelSet &levelSet)
{
    mDirichletBoundaries = levelSet.GetBoundaries();
}

void Engine::Clear()
{
    mDirichletBoundaries.clear();
    mNeumannBoundaries.clear();
    mBoundariesVelocity.clear();
}

void Engine::RenderForce(const std::vector<Renderer::Drawable*> & objects)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    Renderer::Enable b(GL_BLEND);
    Renderer::BlendState s(GL_FUNC_ADD, GL_ONE, GL_ONE);

    mVelocity.begin();
    for(auto object : objects)
    {
        object->Render(mVelocity.Orth*mDimensions.InvScale);
    }
    mVelocity.end();
}

void Engine::Advect(Fluid::Buffer & buffer)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    buffer.swap() = mAdvect(Back(buffer), Back(mVelocity));
}

void Engine::Extrapolate()
{
    RenderMask(mExtrapolateValid);
    RenderMask(mExtrapolateValid.swap());

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0, 0xFF);

    mExtrapolateValid.begin();
    mSurface.Colour = glm::vec4{1.0f};
    mSurface.Render(mExtrapolateValid.Orth);
    mExtrapolateValid.end();

    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

    for(int i = 0 ; i < 20 ; i++)
    {
        mVelocity.swap() = mExtrapolate(mExtrapolateValid, Back(mVelocity));
        mExtrapolateValid.swap() = mExtrapolateMask(Back(mExtrapolateValid));
    }
}

}