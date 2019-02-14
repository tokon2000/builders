
#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <va/va.h>
#include <va/va_drm.h>
#include <va/va_drmcommon.h>

#if defined(YAMI_INF_X11)
#include <va/va_x11.h>
#endif

#include <YamiVersion.h>
#include <VideoEncoderCapi.h>
#include <VideoDecoderCapi.h>

#include "yami_inf.h"

static VADisplay g_va_display = 0;
static VAConfigID g_vp_config = 0;
static VAContextID g_vp_context = 0;

struct yami_inf_enc_priv
{
    EncodeHandler encoder;
    int width;
    int height;
    int type;
    int flags;
    VASurfaceID va_surface[1];
    VASurfaceID fd_va_surface[1];
    VAImage va_image[1];
    char *yuvdata;
};

struct yami_inf_dec_priv
{
    DecodeHandler decoder;
    int width;
    int height;
    int type;
    int flags;
};

/*****************************************************************************/
int
yami_get_version(int *version)
{
    *version = (YI_MAJOR << 16) | YI_MINOR;
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_init(int type, void *display)
{
    int fd;
    int major_version;
    int minor_version;
    uint32_t yami_version;
    VAStatus va_status;
#if defined(YAMI_INF_X11)
    Display *dpy;
#endif

    if (type == YI_TYPE_DRM)
    {
        fd = (int) (size_t) display;
        g_va_display = vaGetDisplayDRM(fd);
    }
    else if (type == YI_TYPE_X11)
    {
#if defined(YAMI_INF_X11)
        dpy = (Display *) display;
        g_va_display = vaGetDisplay(dpy);
#else
        return YI_ERROR_UNIMP;
#endif
    }
    else
    {
        return YI_ERROR_TYPE;
    }
    va_status = vaInitialize(g_va_display, &major_version, &minor_version);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return YI_ERROR_VAINITIALIZE;
    }
    va_status = vaCreateConfig(g_va_display, VAProfileNone,
                               VAEntrypointVideoProc, NULL, 0, &g_vp_config);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return YI_ERROR_VACREATECONFIG;
    }
    va_status = vaCreateContext(g_va_display, g_vp_config, 0, 0, 0, NULL, 0,
                                &g_vp_context);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return YI_ERROR_VACREATECONTEXT;
    }
    yamiGetApiVersion(&yami_version);
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_deinit(void)
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
int
yami_encoder_create(void **obj, int width, int height, int type, int flags)
{
    struct yami_inf_enc_priv *enc;
    VASurfaceAttrib attribs[1];
    VAStatus va_status;
    VAImageFormat va_image_format;
    NativeDisplay nd;
    YamiStatus yami_status;
    VideoParamsCommon encVideoParams;
    VideoConfigAVCStreamFormat streamFormat;

    enc = (struct yami_inf_enc_priv *)
          calloc(1, sizeof(struct yami_inf_enc_priv));
    if (enc == NULL)
    {
        return YI_ERROR_MEMORY;
    }
    if (type == YI_TYPE_H264)
    {
        enc->encoder = createEncoder(YAMI_MIME_H264);
        if (enc->encoder == NULL)
        {
            free(enc);
            return YI_ERROR_CREATEENCODER;
        }
    }
    else
    {
        free(enc);
        return YI_ERROR_TYPE;
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
        releaseEncoder(enc->encoder);
        free(enc);
        return YI_ERROR_VACREATESURFACES;
    }
    memset(&va_image_format, 0, sizeof(va_image_format));
    va_image_format.fourcc = VA_FOURCC_NV12;
    va_status = vaCreateImage(g_va_display, &va_image_format,
                              width, height, enc->va_image);
    if (va_status != VA_STATUS_SUCCESS)
    {
        releaseEncoder(enc->encoder);
        vaDestroySurfaces(g_va_display, enc->va_surface, 1);
        free(enc);
        return YI_ERROR_VACREATEIMAGE;
    }
    memset(&nd, 0, sizeof(nd));
    nd.handle = (intptr_t) g_va_display;
    nd.type = NATIVE_DISPLAY_VA;
    encodeSetNativeDisplay(enc->encoder, &nd);
    memset(&encVideoParams, 0, sizeof(encVideoParams));
    encVideoParams.size = sizeof(VideoParamsCommon);
    yami_status = encodeGetParameters(enc->encoder, VideoParamsTypeCommon,
                                      &encVideoParams);
    if (yami_status != ENCODE_SUCCESS)
    {
        vaDestroyImage(g_va_display, enc->va_image[0].image_id);
        vaDestroySurfaces(g_va_display, enc->va_surface, 1);
        releaseEncoder(enc->encoder);
        free(enc);
        return YI_ERROR_ENCODEGETPARAMETERS;
    }
    encVideoParams.size = sizeof(VideoParamsCommon);
    encVideoParams.resolution.width = width;
    encVideoParams.resolution.height = height;
    encVideoParams.rcMode = RATE_CONTROL_CQP;
    encVideoParams.rcParams.initQP = 28;
    encVideoParams.intraPeriod = 1024;
    if (type == YI_TYPE_H264)
    {
        switch (flags & YI_H264_ENC_FLAGS_PROFILE_MASK)
        {
            case YI_H264_ENC_FLAGS_PROFILE_HIGH:
                encVideoParams.profile = VAProfileH264High;
                break;
            case YI_H264_ENC_FLAGS_PROFILE_MAIN:
                encVideoParams.profile = VAProfileH264Main;
                break;
            default:
                encVideoParams.profile = VAProfileH264ConstrainedBaseline;
                break;
        }
    }
    yami_status = encodeSetParameters(enc->encoder, VideoParamsTypeCommon,
                                      &encVideoParams);
    if (yami_status != ENCODE_SUCCESS)
    {
        vaDestroyImage(g_va_display, enc->va_image[0].image_id);
        vaDestroySurfaces(g_va_display, enc->va_surface, 1);
        releaseEncoder(enc->encoder);
        free(enc);
        return YI_ERROR_ENCODESETPARAMETERS;
    }
    memset(&streamFormat, 0, sizeof(streamFormat));
    streamFormat.size = sizeof(VideoConfigAVCStreamFormat);
    yami_status = encodeGetParameters(enc->encoder,
                                      VideoConfigTypeAVCStreamFormat,
                                      &streamFormat);
    if (yami_status != ENCODE_SUCCESS)
    {
        vaDestroyImage(g_va_display, enc->va_image[0].image_id);
        vaDestroySurfaces(g_va_display, enc->va_surface, 1);
        releaseEncoder(enc->encoder);
        free(enc);
        return YI_ERROR_ENCODEGETPARAMETERS;
    }
    streamFormat.streamFormat = AVC_STREAM_FORMAT_ANNEXB;
    yami_status = encodeSetParameters(enc->encoder,
                                      VideoConfigTypeAVCStreamFormat,
                                      &streamFormat);
    if (yami_status != ENCODE_SUCCESS)
    {
        vaDestroyImage(g_va_display, enc->va_image[0].image_id);
        vaDestroySurfaces(g_va_display, enc->va_surface, 1);
        releaseEncoder(enc->encoder);
        free(enc);
        return YI_ERROR_ENCODESETPARAMETERS;
    }
    yami_status = encodeStart(enc->encoder);
    if (yami_status != ENCODE_SUCCESS)
    {
        vaDestroyImage(g_va_display, enc->va_image[0].image_id);
        vaDestroySurfaces(g_va_display, enc->va_surface, 1);
        releaseEncoder(enc->encoder);
        free(enc);
        return YI_ERROR_ENCODESTART;
    }
    enc->width = width;
    enc->height = height;
    enc->type = type;
    enc->flags = flags;
    *obj = enc;
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_encoder_delete(void *obj)
{
    struct yami_inf_enc_priv *enc;

    enc = (struct yami_inf_enc_priv *) obj;
    if (enc == NULL)
    {
        return YI_SUCCESS;
    }
    encodeStop(enc->encoder);
    vaDestroyImage(g_va_display, enc->va_image[0].image_id);
    vaDestroySurfaces(g_va_display, enc->va_surface, 1);
    releaseEncoder(enc->encoder);
    free(enc);
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_encoder_get_width(void *obj, int *width)
{
    struct yami_inf_enc_priv *enc;

    enc = (struct yami_inf_enc_priv *) obj;
    *width = enc->width;
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_encoder_get_height(void *obj, int *height)
{
    struct yami_inf_enc_priv *enc;

    enc = (struct yami_inf_enc_priv *) obj;
    *height = enc->height;
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_encoder_resize(void *obj, int width, int height)
{
    struct yami_inf_enc_priv *enc;
    VideoParamsCommon encVideoParams;
    YamiStatus yami_status;
    VAStatus va_status;
    VAImageFormat va_image_format;
    VASurfaceAttrib attribs[1];

    enc = (struct yami_inf_enc_priv *) obj;
    encodeStop(enc->encoder);
    memset(&encVideoParams, 0, sizeof(encVideoParams));
    encVideoParams.size = sizeof(VideoParamsCommon);
    yami_status = encodeGetParameters(enc->encoder, VideoParamsTypeCommon,
                                      &encVideoParams);
    if (yami_status != ENCODE_SUCCESS)
    {
        return YI_ERROR_ENCODEGETPARAMETERS;
    }
    encVideoParams.resolution.width = width;
    encVideoParams.resolution.height = height;
    encVideoParams.size = sizeof(VideoParamsCommon);
    yami_status = encodeSetParameters(enc->encoder, VideoParamsTypeCommon,
                                      &encVideoParams);
    if (yami_status != ENCODE_SUCCESS)
    {
        return YI_ERROR_ENCODESETPARAMETERS;
    }
    yami_status = encodeStart(enc->encoder);
    if (yami_status != ENCODE_SUCCESS)
    {
        return YI_ERROR_ENCODESTART;
    }
    va_status = vaDestroyImage(g_va_display, enc->va_image[0].image_id);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return 1;
    }
    va_status = vaDestroySurfaces(g_va_display, enc->va_surface, 1);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return 1;
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
        return YI_ERROR_VACREATESURFACES;
    }
    memset(&va_image_format, 0, sizeof(va_image_format));
    va_image_format.fourcc = VA_FOURCC_NV12;
    va_status = vaCreateImage(g_va_display, &va_image_format,
                              width, height, enc->va_image);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return 1;
    }
    if (enc->fd_va_surface[0] != 0)
    {
        va_status = vaDestroySurfaces(g_va_display, enc->fd_va_surface, 1);
        if (va_status != VA_STATUS_SUCCESS)
        {
            return YI_ERROR_VACREATESURFACES;
        }
        enc->fd_va_surface[0] = 0;
    }
    enc->yuvdata = NULL;
    enc->width = width;
    enc->height = height;
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_encoder_get_ybuffer(void *obj, void **ydata, int *ydata_stride_bytes)
{
    struct yami_inf_enc_priv *enc;
    VAStatus va_status;
    void *buf_ptr;

    enc = (struct yami_inf_enc_priv *) obj;
    if (enc->yuvdata == NULL)
    {
        va_status = vaMapBuffer(g_va_display, enc->va_image[0].buf, &buf_ptr);
        if (va_status != VA_STATUS_SUCCESS)
        {
            return YI_ERROR_VAMAPBUFFER;
        }
        enc->yuvdata = (char *) buf_ptr;
    }
    *ydata = enc->yuvdata + enc->va_image[0].offsets[0];
    *ydata_stride_bytes = enc->va_image[0].pitches[0];
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_encoder_get_uvbuffer(void *obj, void **uvdata, int *uvdata_stride_bytes)
{
    struct yami_inf_enc_priv *enc;
    VAStatus va_status;
    void *buf_ptr;

    enc = (struct yami_inf_enc_priv *) obj;
    if (enc->yuvdata == NULL)
    {
        va_status = vaMapBuffer(g_va_display, enc->va_image[0].buf, &buf_ptr);
        if (va_status != VA_STATUS_SUCCESS)
        {
            return YI_ERROR_VAMAPBUFFER;
        }
        enc->yuvdata = (char *) buf_ptr;
    }
    *uvdata = enc->yuvdata + enc->va_image[0].offsets[1];
    *uvdata_stride_bytes = enc->va_image[0].pitches[1];
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_encoder_set_fd_src(void *obj, int fd, int fd_width, int fd_height,
                        int fd_stride, int fd_size, int fd_bpp)
{
    struct yami_inf_enc_priv *enc;
    VASurfaceAttribExternalBuffers external;
    VASurfaceAttrib attribs[2];
    unsigned long fd_handle;
    VAStatus va_status;
    VASurfaceID surface;

    enc = (struct yami_inf_enc_priv *) obj;
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
        return YI_ERROR_VACREATESURFACES;
    }
    if (enc->fd_va_surface[0] != 0)
    {
        vaDestroySurfaces(g_va_display, enc->fd_va_surface, 1);
    }
    enc->fd_va_surface[0] = surface;
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_encoder_encode(void *obj, void *cdata, int *cdata_max_bytes)
{
    struct yami_inf_enc_priv *enc;
    VAStatus va_status;
    VideoFrame yami_vf;
    YamiStatus yami_status;
    VideoEncOutputBuffer outb;
    int width;
    int height;

    enc = (struct yami_inf_enc_priv *) obj;
    if (enc->yuvdata != NULL)
    {
        va_status = vaUnmapBuffer(g_va_display, enc->va_image[0].buf);
        if (va_status != VA_STATUS_SUCCESS)
        {
            return YI_ERROR_VAUNMAPBUFFER;
        }
        enc->yuvdata = NULL;
    }
    memset(&yami_vf, 0, sizeof(yami_vf));
    yami_vf.crop.width = enc->width;
    yami_vf.crop.width = enc->height;
    if (enc->fd_va_surface[0] != 0)
    {
        yami_vf.surface = enc->fd_va_surface[0];
        yami_vf.fourcc = YAMI_FOURCC_RGBX;
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
            return YI_ERROR_VAPUTIMAGE;
        }
        yami_vf.surface = enc->va_surface[0];
        yami_vf.fourcc = YAMI_FOURCC_NV12;
    }
    va_status = vaSyncSurface(g_va_display, yami_vf.surface);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return YI_ERROR_VASYNCSURFACE;
    }
    yami_status = encodeEncode(enc->encoder, &yami_vf);
    if (yami_status != YAMI_SUCCESS)
    {
        return YI_ERROR_ENCODEENCODE;
    }
    memset(&outb, 0, sizeof(outb));
    outb.data = (unsigned char *) cdata;
    outb.bufferSize = *cdata_max_bytes;
    outb.format = OUTPUT_EVERYTHING;
    yami_status = encodeGetOutput(enc->encoder, &outb, 1);
    if (yami_status != 0)
    {
        return YI_ERROR_ENCODEGETOUTPUT;
    }
    *cdata_max_bytes = outb.dataSize;
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_decoder_create(void **obj, int width, int height, int type, int flags)
{
    struct yami_inf_dec_priv *dec;
    NativeDisplay nd;
    VideoConfigBuffer cb;

    dec = (struct yami_inf_dec_priv *)
          calloc(1, sizeof(struct yami_inf_dec_priv));
    if (dec == NULL)
    {
        return YI_ERROR_MEMORY;
    }
    if (type == YI_TYPE_H264)
    {
        dec->decoder = createDecoder(YAMI_MIME_H264);
        if (dec->decoder == NULL)
        {
            free(dec);
            return YI_ERROR_CREATEDECODER;
        }
    }
    else
    {
        free(dec);
        return YI_ERROR_UNIMP;
    }
    memset(&nd, 0, sizeof(nd));
    nd.type = NATIVE_DISPLAY_VA;
    nd.handle = (long) g_va_display;
    decodeSetNativeDisplay(dec->decoder, &nd);
    memset(&cb, 0, sizeof(cb));
    cb.profile = VAProfileNone;
    if ((type == YI_TYPE_H264) && (flags & YI_H264_DEC_FLAG_LOWLATENCY))
    {
        cb.enableLowLatency = 1;
    }
    if (decodeStart(dec->decoder, &cb) != YAMI_SUCCESS)
    {
        releaseDecoder(dec->decoder);
        free(dec);
        return YI_ERROR_CREATEDECODER;
    }
    dec->width = width;
    dec->height = height;
    *obj = dec;
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_decoder_delete(void *obj)
{
    struct yami_inf_dec_priv *dec;

    dec = (struct yami_inf_dec_priv *) obj;
    if (dec == NULL)
    {
        return YI_SUCCESS;
    }
    releaseDecoder(dec->decoder);
    free(dec);
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_decoder_decode(void *obj, void *cdata, int cdata_bytes)
{
    struct yami_inf_dec_priv *dec;
    YamiStatus yami_status;
    VideoDecodeBuffer ib;
    const VideoFormatInfo *fi;

    dec = (struct yami_inf_dec_priv *) obj;
    memset(&ib, 0, sizeof(ib));
    ib.data = cdata;
    ib.size = cdata_bytes;
    ib.flag = VIDEO_DECODE_BUFFER_FLAG_FRAME_END;
    yami_status = decodeDecode(dec->decoder, &ib);
    if (yami_status == YAMI_DECODE_FORMAT_CHANGE)
    {
        fi = decodeGetFormatInfo(dec->decoder);
        yami_status = decodeDecode(dec->decoder, &ib);
    }
    if (yami_status != DECODE_SUCCESS)
    {
        return YI_ERROR_DECODEDECODE;
    }
    return YI_SUCCESS;
}

/*****************************************************************************/
int
yami_decoder_get_pixmap(void *obj, void* display,
                        int width, int height, int *pixmap)
{
#if defined(YAMI_INF_X11)
    struct yami_inf_dec_priv *dec;
    VideoFrame *vf;
    VAStatus va_status;

    dec = (struct yami_inf_dec_priv *) obj;
    vf = decodeGetOutput(dec->decoder);
    if (vf == NULL)
    {
        return YI_ERROR_DECODEGETOUTPUT;
    }
    if (vf->surface == 0)
    {
        return YI_ERROR_DECODEGETOUTPUT;
    }
    va_status = vaPutSurface(g_va_display, vf->surface, pixmap,
                             0, 0, dec->width, dec->height,
                             0, 0, width, height,
                             0, 0, 0);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return YI_ERROR_VAPUTSURFACE;
    }
    return YI_SUCCESS;
#else
    return YI_ERROR_UNIMP;
#endif
}
