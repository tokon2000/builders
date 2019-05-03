
#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <va/va.h>
#include <va/va_drm.h>
#include <va/va_drmcommon.h>

#include <mfxvideo.h>
#include <mfxplugin.h>

#include "mssva_inf.h"

static VADisplay g_va_display = 0;
static VAConfigID g_vp_config = 0;
static VAContextID g_vp_context = 0;

struct mssva_inf_enc_priv
{
    mfxSession session;
    mfxVideoParam video_param;
    mfxVideoParam video_param_out;
    mfxExtCodingOption eco;
    mfxExtBuffer *ecop[2];
    mfxFrameAllocRequest fa_request;
    mfxFrameAllocator frame_allocator;
    VASurfaceID va_surfaces[16];
    mfxMemId va_surfaces1[16];
    int num_va_surfaces;
    int width;
    int height;
    int type;
    int flags;
    VASurfaceID va_surface[1];
    VASurfaceID fd_va_surface[1];
    VAImage va_image[1];
    char *yuvdata;
};

/*****************************************************************************/
int
mssva_get_version(int *version)
{
    *version = (MI_MAJOR << 16) | MI_MINOR;
    return MI_SUCCESS;
}

#if !defined(CONFIG_PREFIX)
#define CONFIG_PREFIX "/opt/mssva"
#endif

/*****************************************************************************/
int
mssva_init(int type, void *display)
{
    int fd;
    int major_version;
    int minor_version;
    VAStatus va_status;
    const char *libva_driver_path;
    const char *libva_driver_name;

    if (type == MI_TYPE_DRM)
    {
        fd = (int) (size_t) display;
        g_va_display = vaGetDisplayDRM(fd);
    }
    else
    {
        return MI_ERROR_TYPE;
    }
    libva_driver_path = getenv("LIBVA_DRIVERS_PATH");
    libva_driver_name = getenv("LIBVA_DRIVER_NAME");
    setenv("LIBVA_DRIVERS_PATH", CONFIG_PREFIX "/lib", 1);
    setenv("LIBVA_DRIVER_NAME", "iHD", 1);
    va_status = vaInitialize(g_va_display, &major_version, &minor_version);
    if (libva_driver_path == NULL)
    {
        unsetenv("LIBVA_DRIVERS_PATH");
    }
    else
    {
        setenv("LIBVA_DRIVERS_PATH", libva_driver_path, 1);
    }
    if (libva_driver_name == NULL)
    {
        unsetenv("LIBVA_DRIVER_NAME");
    }
    else
    {
        setenv("LIBVA_DRIVER_NAME", libva_driver_name, 1);
    }
    if (va_status != VA_STATUS_SUCCESS)
    {
        return MI_ERROR_VAINITIALIZE;
    }
    va_status = vaCreateConfig(g_va_display, VAProfileNone,
                               VAEntrypointVideoProc, NULL, 0, &g_vp_config);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return MI_ERROR_VACREATECONFIG;
    }
    va_status = vaCreateContext(g_va_display, g_vp_config, 0, 0, 0, NULL, 0,
                                &g_vp_context);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return MI_ERROR_VACREATECONTEXT;
    }
    return MI_SUCCESS;
}

/*****************************************************************************/
int
mssva_deinit(void)
{
    vaDestroyContext(g_va_display, g_vp_context);
    g_vp_context = 0;
    vaDestroyConfig(g_va_display, g_vp_config);
    g_vp_config = 0;
    vaTerminate(g_va_display);
    g_va_display = 0;
    return 0;
}

/*****************************************************************************/
static mfxStatus MFX_CDECL
frame_alloc(mfxHDL pthis, mfxFrameAllocRequest *request,
            mfxFrameAllocResponse *response)
{
    struct mssva_inf_enc_priv* enc;
    VAStatus va_status;
    int index;

    enc = (struct mssva_inf_enc_priv *) pthis;
    if ((request->Type & MFX_MEMTYPE_INTERNAL_FRAME) &&
        (request->Type & MFX_MEMTYPE_FROM_ENCODE))
    {
    }
    else
    {
        return MFX_ERR_UNSUPPORTED;
    }
    if (request->NumFrameSuggested > 16)
    {
        return MFX_ERR_UNSUPPORTED;
    }
    enc->num_va_surfaces = request->NumFrameSuggested;
    va_status = vaCreateSurfaces(g_va_display, VA_RT_FORMAT_YUV420,
                                 request->Info.Width, request->Info.Height,
                                 enc->va_surfaces,
                                 enc->num_va_surfaces, NULL, 0);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return MFX_ERR_MEMORY_ALLOC;
    }
    for (index = enc->num_va_surfaces - 1; index >= 0; index--)
    {
        enc->va_surfaces1[index] = &(enc->va_surfaces[index]);
    }
    response->mids = enc->va_surfaces1;
    response->NumFrameActual = enc->num_va_surfaces;
    return MFX_ERR_NONE;
}

/*****************************************************************************/
static mfxStatus MFX_CDECL
frame_lock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr)
{
    return MFX_ERR_NONE;
}

/*****************************************************************************/
static mfxStatus MFX_CDECL
frame_unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr)
{
    return MFX_ERR_NONE;
}

/*****************************************************************************/
static mfxStatus MFX_CDECL
frame_get_hdl(mfxHDL pthis, mfxMemId mid, mfxHDL *handle)
{
    *handle = mid;
    return MFX_ERR_NONE;
}

/*****************************************************************************/
static mfxStatus MFX_CDECL
frame_free(mfxHDL pthis, mfxFrameAllocResponse *response)
{
    int index;
    VASurfaceID sur;

    for (index = response->NumFrameActual - 1; index >= 0; index--)
    {
        sur = *((VASurfaceID *) (response->mids[index]));
        vaDestroySurfaces(g_va_display, &sur, 1);
    }
    return MFX_ERR_NONE;
}

/*****************************************************************************/
int
mssva_encoder_create(void **obj, int width, int height, int type, int flags)
{
    struct mssva_inf_enc_priv *enc;
    VASurfaceAttrib attribs[1];
    VAStatus va_status;
    VAImageFormat va_image_format;
    mfxVersion version;
    mfxIMPL imp;
    mfxStatus status;

    enc = (struct mssva_inf_enc_priv *)
          calloc(1, sizeof(struct mssva_inf_enc_priv));
    if (enc == NULL)
    {
        return MI_ERROR_MEMORY;
    }
    if (type == MI_TYPE_H264)
    {
        enc->video_param.mfx.CodecId = MFX_CODEC_AVC;
        switch (flags & MI_H264_ENC_FLAGS_PROFILE_MASK)
        {
            case MI_H264_ENC_FLAGS_PROFILE_MAIN:
                enc->video_param.mfx.CodecProfile = MFX_PROFILE_AVC_MAIN;
                break;
            case MI_H264_ENC_FLAGS_PROFILE_HIGH:
                enc->video_param.mfx.CodecProfile = MFX_PROFILE_AVC_HIGH;
                break;
            default:
                enc->video_param.mfx.CodecProfile = MFX_PROFILE_AVC_BASELINE;
                break;
        }
    }
    else
    {
        free(enc);
        return MI_ERROR_TYPE;
    }
    memset(attribs, 0, sizeof(attribs));
    attribs[0].type = VASurfaceAttribPixelFormat;
    attribs[0].flags = VA_SURFACE_ATTRIB_SETTABLE;
    attribs[0].value.type = VAGenericValueTypeInteger;
    attribs[0].value.value.i = VA_FOURCC_NV12;
    va_status = vaCreateSurfaces(g_va_display, VA_RT_FORMAT_YUV420,
                                 width, height,
                                 enc->va_surface, 1, attribs, 1);
    if (va_status != VA_STATUS_SUCCESS)
    {
        free(enc);
        return MI_ERROR_VACREATESURFACES;
    }
    memset(&va_image_format, 0, sizeof(va_image_format));
    va_image_format.fourcc = VA_FOURCC_NV12;
    va_status = vaCreateImage(g_va_display, &va_image_format,
                              width, height, enc->va_image);
    if (va_status != VA_STATUS_SUCCESS)
    {
        vaDestroySurfaces(g_va_display, enc->va_surface, 1);
        free(enc);
        return MI_ERROR_VACREATEIMAGE;
    }
    enc->frame_allocator.pthis = enc;
    enc->frame_allocator.Alloc = frame_alloc;
    enc->frame_allocator.Lock = frame_lock;
    enc->frame_allocator.Unlock = frame_unlock;
    enc->frame_allocator.GetHDL = frame_get_hdl;
    enc->frame_allocator.Free = frame_free;
    memset(&version, 0, sizeof(version));
    version.Minor = MFX_VERSION_MINOR;
    version.Major = MFX_VERSION_MAJOR;
    imp = MFX_IMPL_HARDWARE | MFX_IMPL_VIA_VAAPI;
    status = MFXInit(imp, &version, &(enc->session));
    if (status != MFX_ERR_NONE)
    {
        vaDestroyImage(g_va_display, enc->va_image[0].image_id);
        vaDestroySurfaces(g_va_display, enc->va_surface, 1);
        free(enc);
        return MI_ERROR_MFXINIT;
    }
    status = MFXVideoCORE_SetHandle(enc->session, MFX_HANDLE_VA_DISPLAY,
                                    g_va_display);
    if (status != MFX_ERR_NONE)
    {
        MFXClose(enc->session);
        vaDestroyImage(g_va_display, enc->va_image[0].image_id);
        vaDestroySurfaces(g_va_display, enc->va_surface, 1);
        free(enc);
        return MI_ERROR_MFXVIDEOCORE_SETHANDLE;
    }
    status = MFXVideoCORE_SetFrameAllocator(enc->session,
                                            &(enc->frame_allocator));
    if (status != MFX_ERR_NONE)
    {
        MFXClose(enc->session);
        vaDestroyImage(g_va_display, enc->va_image[0].image_id);
        vaDestroySurfaces(g_va_display, enc->va_surface, 1);
        free(enc);
        return MI_ERROR_MFXVIDEOCORE_SETFRAMEALLOCATOR;
    }
    enc->video_param.IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY;
    enc->video_param.AsyncDepth = 1;
    enc->video_param.mfx.TargetUsage = 4;
    enc->video_param.mfx.FrameInfo.FrameRateExtN = 24;
    enc->video_param.mfx.FrameInfo.FrameRateExtD = 1;
    enc->video_param.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
    enc->video_param.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    enc->video_param.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    enc->video_param.mfx.FrameInfo.Width = (width + 15) & ~15;
    enc->video_param.mfx.FrameInfo.Height = (height + 15) & ~15;
    enc->video_param.mfx.RateControlMethod = MFX_RATECONTROL_CQP;
    enc->video_param.mfx.QPI = 28;
    enc->video_param.mfx.QPB = 28;
    enc->video_param.mfx.QPP = 28;
    enc->video_param.mfx.GopPicSize = 1024;
    enc->video_param.mfx.NumRefFrame = 1;
    /* low latency, I and P frames only */
    enc->video_param.mfx.GopRefDist = 1;
    enc->video_param.mfx.NumSlice = 1;
    /* turn off AUD in NALU */
    enc->eco.Header.BufferId = MFX_EXTBUFF_CODING_OPTION;
    enc->eco.Header.BufferSz = sizeof(enc->eco);
    enc->eco.AUDelimiter = MFX_CODINGOPTION_OFF;
    enc->ecop[0] = &(enc->eco.Header);
    enc->ecop[1] = NULL;
    enc->video_param.ExtParam = enc->ecop;
    enc->video_param.NumExtParam = 1;
    enc->video_param_out = enc->video_param;
    status = MFXVideoENCODE_Query(enc->session, &(enc->video_param),
                                  &(enc->video_param_out));
    if (status != MFX_ERR_NONE)
    {
        MFXClose(enc->session);
        vaDestroyImage(g_va_display, enc->va_image[0].image_id);
        vaDestroySurfaces(g_va_display, enc->va_surface, 1);
        free(enc);
        return MI_ERROR_MFXVIDEOENCODE_QUERY;
    }
    status = MFXVideoENCODE_QueryIOSurf(enc->session,
                                        &(enc->video_param_out),
                                        &(enc->fa_request));
    if (status != MFX_ERR_NONE)
    {
        MFXClose(enc->session);
        vaDestroyImage(g_va_display, enc->va_image[0].image_id);
        vaDestroySurfaces(g_va_display, enc->va_surface, 1);
        free(enc);
        return MI_ERROR_MFXVIDEOENCODE_QUERYIOSURF;
    }
    status = MFXVideoENCODE_Init(enc->session, &(enc->video_param_out));
    if (status != MFX_ERR_NONE)
    {
        MFXClose(enc->session);
        vaDestroyImage(g_va_display, enc->va_image[0].image_id);
        vaDestroySurfaces(g_va_display, enc->va_surface, 1);
        free(enc);
        return MI_ERROR_MFXVIDEOENCODE_INIT;
    }
    enc->width = width;
    enc->height = height;
    enc->type = type;
    enc->flags = flags;
    *obj = enc;
    return MI_SUCCESS;
}

/*****************************************************************************/
int
mssva_encoder_delete(void *obj)
{
    struct mssva_inf_enc_priv *enc;

    enc = (struct mssva_inf_enc_priv *) obj;
    if (enc == NULL)
    {
        return 0;
    }
    MFXVideoENCODE_Close(enc->session);
    MFXClose(enc->session);
    vaDestroyImage(g_va_display, enc->va_image[0].image_id);
    vaDestroySurfaces(g_va_display, enc->va_surface, 1);
    if (enc->fd_va_surface[0] != 0)
    {
        vaDestroySurfaces(g_va_display, enc->fd_va_surface, 1);
    }
    free(enc);
    return MI_SUCCESS;
}

/*****************************************************************************/
int
mssva_encoder_get_width(void *obj, int *width)
{
    struct mssva_inf_enc_priv *enc;

    enc = (struct mssva_inf_enc_priv *) obj;
    *width = enc->width;
    return MI_SUCCESS;
}

/*****************************************************************************/
int
mssva_encoder_get_height(void *obj, int *height)
{
    struct mssva_inf_enc_priv *enc;

    enc = (struct mssva_inf_enc_priv *) obj;
    *height = enc->height;
    return MI_SUCCESS;
}

/*****************************************************************************/
int
mssva_encoder_resize(void *obj, int width, int height)
{
    struct mssva_inf_enc_priv *enc;
    VAStatus va_status;
    VAImageFormat va_image_format;
    VASurfaceAttrib attribs[1];
    mfxStatus status;

    enc = (struct mssva_inf_enc_priv *) obj;
    status = MFXVideoENCODE_Close(enc->session);
    if (status != MFX_ERR_NONE)
    {
        return MI_ERROR_MFXVIDEOENCODE_CLOSE;
    }
    enc->video_param.mfx.FrameInfo.Width = (width + 15) & ~15;
    enc->video_param.mfx.FrameInfo.Height = (height + 15) & ~15;
    status = MFXVideoENCODE_Query(enc->session, &(enc->video_param),
                                  &(enc->video_param_out));
    if (status != MFX_ERR_NONE)
    {
        return MI_ERROR_MFXVIDEOENCODE_QUERY;
    }
    status = MFXVideoENCODE_QueryIOSurf(enc->session,
                                        &(enc->video_param_out),
                                        &(enc->fa_request));
    if (status != MFX_ERR_NONE)
    {
        return MI_ERROR_MFXVIDEOENCODE_QUERYIOSURF;
    }
    status = MFXVideoENCODE_Init(enc->session, &(enc->video_param_out));
    if (status != MFX_ERR_NONE)
    {
        return MI_ERROR_MFXVIDEOENCODE_INIT;

    }
    va_status = vaDestroyImage(g_va_display, enc->va_image[0].image_id);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return MI_ERROR_VADESTROYIMAGE;
    }
    va_status = vaDestroySurfaces(g_va_display, enc->va_surface, 1);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return MI_ERROR_VADESTROYSURFACES;
    }
    memset(attribs, 0, sizeof(attribs));
    attribs[0].type = VASurfaceAttribPixelFormat;
    attribs[0].flags = VA_SURFACE_ATTRIB_SETTABLE;
    attribs[0].value.type = VAGenericValueTypeInteger;
    attribs[0].value.value.i = VA_FOURCC_NV12;
    va_status = vaCreateSurfaces(g_va_display, VA_RT_FORMAT_YUV420,
                                 width, height,
                                 enc->va_surface, 1, attribs, 1);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return MI_ERROR_VACREATESURFACES;
    }
    memset(&va_image_format, 0, sizeof(va_image_format));
    va_image_format.fourcc = VA_FOURCC_NV12;
    va_status = vaCreateImage(g_va_display, &va_image_format,
                              width, height, enc->va_image);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return MI_ERROR_VACREATEIMAGE;
    }
    if (enc->fd_va_surface[0] != 0)
    {
        va_status = vaDestroySurfaces(g_va_display, enc->fd_va_surface, 1);
        if (va_status != VA_STATUS_SUCCESS)
        {
            return MI_ERROR_VADESTROYSURFACES;
        }
        enc->fd_va_surface[0] = 0;
    }
    enc->yuvdata = NULL;
    enc->width = width;
    enc->height = height;
    return MI_SUCCESS;
}

/*****************************************************************************/
int
mssva_encoder_get_ybuffer(void *obj, void **ydata, int *ydata_stride_bytes)
{
    struct mssva_inf_enc_priv *enc;
    VAStatus va_status;
    void *buf_ptr;

    enc = (struct mssva_inf_enc_priv *) obj;
    if (enc->yuvdata == NULL)
    {
        va_status = vaMapBuffer(g_va_display, enc->va_image[0].buf, &buf_ptr);
        if (va_status != VA_STATUS_SUCCESS)
        {
            return MI_ERROR_VAMAPBUFFER;
        }
        enc->yuvdata = (char *) buf_ptr;
    }
    *ydata = enc->yuvdata + enc->va_image[0].offsets[0];
    *ydata_stride_bytes = enc->va_image[0].pitches[0];
    return MI_SUCCESS;
}

/*****************************************************************************/
int
mssva_encoder_get_uvbuffer(void *obj, void **uvdata, int *uvdata_stride_bytes)
{
    struct mssva_inf_enc_priv *enc;
    VAStatus va_status;
    void *buf_ptr;

    enc = (struct mssva_inf_enc_priv *) obj;
    if (enc->yuvdata == NULL)
    {
        va_status = vaMapBuffer(g_va_display, enc->va_image[0].buf, &buf_ptr);
        if (va_status != VA_STATUS_SUCCESS)
        {
            return MI_ERROR_VAMAPBUFFER;
        }
        enc->yuvdata = (char *) buf_ptr;
    }
    *uvdata = enc->yuvdata + enc->va_image[0].offsets[1];
    *uvdata_stride_bytes = enc->va_image[0].pitches[1];
    return MI_SUCCESS;
}

/*****************************************************************************/
int
mssva_encoder_set_fd_src(void *obj, int fd, int fd_width, int fd_height,
                         int fd_stride, int fd_size, int fd_bpp)
{
    struct mssva_inf_enc_priv *enc;
    VASurfaceAttribExternalBuffers external;
    VASurfaceAttrib attribs[2];
    unsigned long fd_handle;
    VAStatus va_status;
    VASurfaceID surface;

    enc = (struct mssva_inf_enc_priv *) obj;
    fd_handle = (unsigned long) fd;
    memset(&external, 0, sizeof(external));
    external.pixel_format = VA_FOURCC_BGRX;
    external.width = fd_width;
    external.height = fd_height;
    external.data_size = fd_size;
    external.num_planes = 1;
    external.pitches[0] = fd_stride;
    external.buffers = &fd_handle;
    external.num_buffers = 1;
    memset(attribs, 0, sizeof(attribs));
    attribs[0].flags = VA_SURFACE_ATTRIB_SETTABLE;
    attribs[0].type = VASurfaceAttribMemoryType;
    attribs[0].value.type = VAGenericValueTypeInteger;
    attribs[0].value.value.i = VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
    attribs[1].flags = VA_SURFACE_ATTRIB_SETTABLE;
    attribs[1].type = VASurfaceAttribExternalBufferDescriptor;
    attribs[1].value.type = VAGenericValueTypePointer;
    attribs[1].value.value.p = &external;
    va_status = vaCreateSurfaces(g_va_display, VA_RT_FORMAT_RGB32,
                                 fd_width, fd_height, &surface, 1,
                                 attribs, 2);
    if (va_status != 0)
    {
        return MI_ERROR_VACREATESURFACES;
    }
    if (enc->fd_va_surface[0] != 0)
    {
        vaDestroySurfaces(g_va_display, enc->fd_va_surface, 1);
    }
    enc->fd_va_surface[0] = surface;
    return MI_SUCCESS;
}

/*****************************************************************************/
int
mssva_encoder_encode(void *obj, void *cdata, int *cdata_max_bytes)
{
    struct mssva_inf_enc_priv *enc;
    VAProcPipelineParameterBuffer params;
    VABufferID buf[1];
    VAStatus va_status;
    int width;
    int height;
    mfxBitstream bs;
    mfxSyncPoint syncp;
    mfxFrameSurface1 surface;
    mfxEncodeCtrl ctrl;
    mfxStatus status;

    enc = (struct mssva_inf_enc_priv *) obj;
    if (enc->yuvdata != NULL)
    {
        va_status = vaUnmapBuffer(g_va_display, enc->va_image[0].buf);
        if (va_status != VA_STATUS_SUCCESS)
        {
            return MI_ERROR_VAUNMAPBUFFER;
        }
        enc->yuvdata = NULL;
    }
    if (enc->fd_va_surface[0] != 0)
    {
        /* fd_va_surface is RGB, must convert it to YUV */
        memset(&params, 0, sizeof(params));
        params.surface = enc->fd_va_surface[0];
        params.output_color_standard = VAProcColorStandardBT601;
        va_status = vaCreateBuffer(g_va_display, g_vp_context,
                                   VAProcPipelineParameterBufferType,
                                   sizeof(params), 1, &params, buf);
        if (va_status == VA_STATUS_SUCCESS)
        {
            va_status = vaBeginPicture(g_va_display, g_vp_context,
                                       enc->va_surface[0]);
            if (va_status == VA_STATUS_SUCCESS)
            {
                vaRenderPicture(g_va_display, g_vp_context, buf, 1);
                vaEndPicture(g_va_display, g_vp_context);
            }
            vaDestroyBuffer(g_va_display, buf[0]);
        }
    }
    else
    {
        width = enc->va_image[0].width;
        height = enc->va_image[0].height;
        va_status = vaPutImage(g_va_display, enc->va_surface[0],
                               enc->va_image[0].image_id,
                               0, 0, width, height,
                               0, 0, width, height);
        if (va_status != VA_STATUS_SUCCESS)
        {
            return MI_ERROR_VAPUTIMAGE;
        }
    }
    va_status = vaSyncSurface(g_va_display, enc->va_surface[0]);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return MI_ERROR_VASYNCSURFACE;
    }
    memset(&surface, 0, sizeof(surface));
    memset(&bs, 0, sizeof(bs));
    memset(&ctrl, 0, sizeof(ctrl));
    surface.Data.MemId = enc->va_surface;
    surface.Info = enc->video_param_out.mfx.FrameInfo;
    syncp = NULL;
    bs.MaxLength = *cdata_max_bytes;
    bs.Data = (unsigned char *) cdata;
    status = MFXVideoENCODE_EncodeFrameAsync(enc->session, &ctrl,
                                             &surface, &bs, &syncp);
    if (status >= MFX_ERR_NONE)
    {
        if (syncp != NULL)
        {
            status = MFXVideoCORE_SyncOperation(enc->session, syncp, 1000);
            if (status != MFX_ERR_NONE)
            {
                return MI_ERROR_MFXVIDEOCORE_SYNCOPERATION;
            }
        }
    }
    else
    {
        return MI_ERROR_MFXVIDEOENCODE_ENCODEFRAMEASYNC;
    }
    *cdata_max_bytes = bs.DataLength;
    return MI_SUCCESS;
}
