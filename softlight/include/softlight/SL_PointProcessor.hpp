
#ifndef SL_POINT_PROCESSOR_HPP
#define SL_POINT_PROCESSOR_HPP

#include "softlight/SL_VertexProcessor.hpp"



/*-----------------------------------------------------------------------------
 * Vertex processor for points
-----------------------------------------------------------------------------*/
class SL_PointProcessor final : public SL_VertexProcessor
{
  private:
    void push_bin(size_t primIndex, const ls::math::vec4_t<float>& viewportDims, const SL_TransformedVert& v) noexcept;

    void process_verts(
        const SL_Mesh& m,
        size_t instanceId,
        const ls::math::mat4_t<float>& scissorMat,
        const ls::math::vec4_t<float>& viewportDims
    ) noexcept;

  public:
    virtual ~SL_PointProcessor() noexcept override {}

    virtual void execute() noexcept override;
};



#endif /* SL_POINT_PROCESSOR_HPP */
