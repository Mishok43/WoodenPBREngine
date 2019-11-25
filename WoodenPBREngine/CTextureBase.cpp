#include "pch.h"
#include "CTextureBase.h"


WPBR_BEGIN
DVector3i Texture3DBase<DVectorPacked1f>::blockSize = DVector3i(2, 2, 4);
DVector3i Texture3DBase<DVectorPacked3f>::blockSize = DVector3i(2, 2, 4);

DECL_OUT_COMP_DATA(CTexture2DR)
DECL_OUT_COMP_DATA(CTexture2DRGB)
DECL_OUT_COMP_DATA(CTexture3DR)
DECL_OUT_COMP_DATA(CTexture3DRGB)


DECL_OUT_COMP_DATA(CTextureBinding2DRGB)
DECL_OUT_COMP_DATA(CTextureBinding2DR)
DECL_OUT_COMP_DATA(CTextureBinding3DRGB)
DECL_OUT_COMP_DATA(CTextureBinding3DR)

WPBR_END

