// The stb implementation macros (STB_IMAGE_IMPLEMENTATION  and STB_IMAGE_WRITE_IMPLEMENTATION)
// may be defined from the compile command if the option
// HELLOIMGUI_STB_IMAGE_IMPLEMENTATION and HELLOIMGUI_STB_IMAGE_WRITE_IMPLEMENTATION are set.
// (which is the default for HelloImGui)

#include "stb_image.h"
#include "stb_image_write.h"

inline int SomeCompilersMayNotLikeEmptyLibs_Stb()
{
    return 0;
}
