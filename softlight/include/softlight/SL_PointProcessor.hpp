
#ifndef SL_POINT_PROCESSOR_HPP
#define SL_POINT_PROCESSOR_HPP

#include "softlight/SL_VertexProcessor.hpp"



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
    namespace math
    {
    template <typename num_type>
        union vec4_t;
    }
}



/*-----------------------------------------------------------------------------
 * Vertex processor for points
-----------------------------------------------------------------------------*/
class SL_PointProcessor final : public SL_VertexProcessor
{
  private:
    void push_bin(size_t primIndex, const ls::math::vec4_t<float>& viewportDims, const SL_TransformedVert& v) const noexcept;

    void process_verts(const SL_Mesh& m, size_t instanceId) noexcept;

  public:
    virtual ~SL_PointProcessor() noexcept override {}

    virtual void execute() noexcept override;
};



#endif /* SL_POINT_PROCESSOR_HPP */
