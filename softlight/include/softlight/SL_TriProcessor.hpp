
#ifndef SL_TRI_PROCESSOR_HPP
#define SL_TRI_PROCESSOR_HPP

#include "softlight/SL_VertexProcessor.hpp"



/*-----------------------------------------------------------------------------
 * Vertex processor for triangles & wireframe
-----------------------------------------------------------------------------*/
class SL_TriProcessor final : public SL_VertexProcessor
{
  private:
    void push_bin(size_t primIndex, const SL_TransformedVert& v0, const SL_TransformedVert& v1, const SL_TransformedVert& v2) const noexcept;

    void clip_and_process_tris(
        size_t primIndex,
        const ls::math::vec4& viewportDims,
        const SL_TransformedVert& a,
        const SL_TransformedVert& b,
        const SL_TransformedVert& c
    ) noexcept;

    template <bool usingIndices>
    void process_verts(
        const SL_Mesh& m,
        size_t instanceId,
        const ls::math::mat4_t<float>& scissorMat,
        const ls::math::vec4_t<float>& viewportDims
    ) noexcept;

  public:
    virtual ~SL_TriProcessor() noexcept override {}

    virtual void execute() noexcept override;
};



extern template void SL_TriProcessor::process_verts<true>(
    const SL_Mesh&,
    size_t,
    const ls::math::mat4_t<float>&,
    const ls::math::vec4_t<float>&
) noexcept;



extern template void SL_TriProcessor::process_verts<false>(
    const SL_Mesh&,
    size_t,
    const ls::math::mat4_t<float>&,
    const ls::math::vec4_t<float>&
) noexcept;



#endif /* SL_TRI_PROCESSOR_HPP */
