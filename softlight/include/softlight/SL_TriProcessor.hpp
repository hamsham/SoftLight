
#ifndef SL_TRI_PROCESSOR_HPP
#define SL_TRI_PROCESSOR_HPP

#include "softlight/SL_RasterProcessor.hpp"

struct SL_TransformedVert;



/*-----------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
class SL_TriProcessor final : public SL_RasterProcessor
{
  private:
    void push_bin(size_t primIndex, float fboW, float fboH, const SL_TransformedVert& v0, const SL_TransformedVert& v1, const SL_TransformedVert& v2) const noexcept;

    void clip_and_process_tris(
        size_t primIndex,
        float fboW,
        float fboH,
        const SL_TransformedVert& a,
        const SL_TransformedVert& b,
        const SL_TransformedVert& c
    ) noexcept;

    void process_verts(const SL_Mesh& m, size_t instanceId) noexcept;

  public:
    virtual ~SL_TriProcessor() override {}

    virtual void execute() noexcept override;
};



#endif /* SL_TRI_PROCESSOR_HPP */
