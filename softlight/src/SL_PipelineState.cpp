
#include "softlight/SL_PipelineState.hpp"



/*-------------------------------------
 * Reset Internal State
-------------------------------------*/
void SL_PipelineState::reset() noexcept
{
    SL_PipelineState temp{};

    this->mStates = temp.mStates;

    // use getters & setters here if any validation is needed
    //this->cull_mode(temp.cull_mode());
    //this->depth_test(temp.depth_test());
    //this->depth_mask(temp.depth_mask());
    //this->blend_mode(temp.blend_mode());
    //this->num_varyings(temp.num_varyings());
    //this->num_render_targets(temp.num_render_targets());
}



/*-------------------------------------
 * Reset External State
-------------------------------------*/
void sl_reset(SL_PipelineState& state) noexcept
{
    state.reset();
}
