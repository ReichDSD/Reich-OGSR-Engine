//=================================================================================================
// Gamma Correct Tonemapping and Color Grading
#include "ACES.h"
#include "source_lighting.h"
//=================================================================================================

float3 tonemap_sRGB(float3 x, float w)
{
#if defined(USE_SOURCE_LIGHTING) && defined(USE_SOURCE_TONEMAP)
    // Source Engine (Source SDK 2013 / Black Mesa):
    // HDR компрессия светов с мягким плечом и кинематографичным контрастом средних тонов
    float3 linear_x = SRGBToLinear(x);
    float3 source_x = LinearTosRGB(Source_Tonemap(linear_x, w));
    float3 orig_x = ACES(x);
    x = lerp(orig_x, source_x, source_lighting_control.x);
#elif defined(USE_ACES)
    // use ACES workflow for color grading and tonemapping
    x = ACES(x);
#else
    // convert into linear gamma space
    x = SRGBToLinear(x);

    // color grading
    ACES_LMT(x);

    // clamp negative values
    x = max(0.0, x);

    // Boost the contrast to match ACES RRT
    float Contrast_Boost = 1.42857;
    x = pow(x, Contrast_Boost) * 0.18 / pow(0.18, Contrast_Boost);

    // reinhard tonemapping
    x = x / (x + 1);
    x /= w / (w + 1);

    // convert into sRGB gamma space
    x = LinearTosRGB(x);
#endif
    /*
    //debug highlights for clipping values
    x = x < 0.0 ? float3(0.0,0.0,1.0) : x;
    x = x > 1.0 ? float3(1.0,0.0,0.0) : x;
    */

    // return with saturate, everything should be in LDR sRGB
    return saturate(x);
}
