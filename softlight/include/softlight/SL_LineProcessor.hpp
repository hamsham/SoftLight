
#ifndef SL_LINE_PROCESSOR_HPP
#define SL_LINE_PROCESSOR_HPP

#include "softlight/SL_VertexProcessor.hpp"



/*-----------------------------------------------------------------------------
 * Vertex processor for lines
-----------------------------------------------------------------------------*/
class SL_LineProcessor final : public SL_VertexProcessor
{
  private:
    void push_bin(size_t primIndex, const ls::math::vec4_t<float>& viewportDims, const SL_TransformedVert& v0, const SL_TransformedVert& v1) const noexcept;

    void process_verts(
        const SL_Mesh& m,
        size_t instanceId,
        const ls::math::mat4_t<float>& scissorMat,
        const ls::math::vec4_t<float>& viewportDims
    ) noexcept;

  public:
    virtual ~SL_LineProcessor() noexcept override {}

    virtual void execute() noexcept override;
};



#endif /* SL_LINE_PROCESSOR_HPP */
