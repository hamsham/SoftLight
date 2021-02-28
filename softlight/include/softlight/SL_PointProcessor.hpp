
#ifndef SL_POINT_PROCESSOR_HPP
#define SL_POINT_PROCESSOR_HPP

#include "softlight/SL_VertexProcessor.hpp"



/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
class SL_PointProcessor final : public SL_VertexProcessor
{
  private:
    void push_bin(size_t primIndex, float fboW, float fboH, const SL_TransformedVert& v) const noexcept;

    void process_verts(const SL_Mesh& m, size_t instanceId) noexcept;

  public:
    virtual ~SL_PointProcessor() override {}

    virtual void execute() noexcept override;
};



#endif /* SL_POINT_PROCESSOR_HPP */
