//
//  Preconditioner.h
//  Vortex2D
//

#ifndef Vortex2D_Preconditioner_h
#define Vortex2D_Preconditioner_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Work.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief An interface to represent a linear solver preconditioner.
 */
struct Preconditioner
{
    virtual ~Preconditioner() {}

    /**
     * @brief Bind the linear equation buffers
     * @param d the diagonal of the matrix
     * @param l the lower matrix
     * @param b the right hand side
     * @param x the unknown buffer
     */
    virtual void Bind(Renderer::GenericBuffer& d,
                      Renderer::GenericBuffer& l,
                      Renderer::GenericBuffer& b,
                      Renderer::GenericBuffer& x) = 0;

    /**
     * @brief Record the preconditioner
     * @param commandBuffer the command buffer to record into.
     */
    virtual void Record(vk::CommandBuffer commandBuffer) = 0;
};

}}

#endif
