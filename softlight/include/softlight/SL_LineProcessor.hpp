
#ifndef SL_LINE_PROCESSOR_HPP
#define SL_LINE_PROCESSOR_HPP

#include "softlight/SL_RasterProcessor.hpp"



/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
class SL_LineProcessor final : public SL_RasterProcessor
{
  private:
    void push_bin(size_t primIndex, float fboW, float fboH, const SL_TransformedVert& v0, const SL_TransformedVert& v1) const noexcept;

    void process_verts(const SL_Mesh& m, size_t instanceId) noexcept;

  public:
    virtual ~SL_LineProcessor() override {}

    virtual void execute() noexcept override;
};



#endif /* SL_LINE_PROCESSOR_HPP */
