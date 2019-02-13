
#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <va/va.h>
#include <va/va_drm.h>

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
    VASurfaceID va_surface[1];
    VAImage va_image[1];
};

struct yami_inf_dec_priv
{
    DecodeHandler decoder;
    int width;
    int height;
};

/*****************************************************************************/
int
yami_get_version(int *version)
{
    *version = (YAMI_INF_MAJOR << 16) | YAMI_INF_MINOR;
    return 0;
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
    Display *dpy
#endif

    if (type == 1) /* drm */
    {
        fd = (int) (size_t) display;
        g_va_display = vaGetDisplayDRM(fd);
    }
    else if (type == 2) /* x11 */
    {
#if defined(YAMI_INF_X11)
        dpy = (Display *) display;
        g_va_display = vaGetDisplay(dpy);
#else
        return 1;
#endif
    }
    else
    {
        return 1;
    }
    va_status = vaInitialize(g_va_display, &major_version, &minor_version);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return 1;
    }
    va_status = vaCreateConfig(g_va_display, VAProfileNone,
                               VAEntrypointVideoProc, NULL, 0, &g_vp_config);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return 1;
    }
    va_status = vaCreateContext(g_va_display, g_vp_config, 0, 0, 0, NULL, 0,
                                &g_vp_context);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return 1;
    }
    yamiGetApiVersion(&yami_version);
    return 0;
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

    enc = (struct yami_inf_enc_priv *)
          calloc(1, sizeof(struct yami_inf_enc_priv));
    if (enc == NULL)
    {
        return 1;
    }
    if (type == 1) /* h264 */
    {
        enc->encoder = createEncoder(YAMI_MIME_H264);
        if (enc->encoder == NULL)
        {
            free(enc);
            return 0;
        }
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
        return 0;
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
        return 0;
    }

    enc->width = width;
    enc->height = height;
    *obj = enc;
    return 0;
}

/*****************************************************************************/
int
yami_encoder_delete(void *obj)
{
    struct yami_inf_enc_priv *enc;

    enc = (struct yami_inf_enc_priv *) obj;
    if (enc == NULL)
    {
        return 0;
    }
    vaDestroyImage(g_va_display, enc->va_image[0].image_id);
    vaDestroySurfaces(g_va_display, enc->va_surface, 1);
    releaseEncoder(enc->encoder);
    free(enc);
    return 0;
}

/*****************************************************************************/
int
yami_encoder_get_width(void *obj, int *width)
{
    struct yami_inf_enc_priv *enc;

    enc = (struct yami_inf_enc_priv *) obj;
    *width = enc->width;
    return 0;
}

/*****************************************************************************/
int
yami_encoder_get_height(void *obj, int *height)
{
    struct yami_inf_enc_priv *enc;

    enc = (struct yami_inf_enc_priv *) obj;
    *height = enc->height;
    return 0;
}

/*****************************************************************************/
int
yami_encoder_resize(void *obj, int width, int height)
{
    return 0;
}

/*****************************************************************************/
int
yami_encoder_get_ybuffer(void *obj, void **ydata, int *ydata_stride_bytes)
{
    return 0;
}

/*****************************************************************************/
int
yami_encoder_get_uvbuffer(void *obj, void **uvdata, int *uvdata_stride_bytes)
{
    return 0;
}

/*****************************************************************************/
int
yami_encoder_set_fd_src(void *obj, int fd, int fd_width, int fd_height,
                        int fd_stride, int fd_size, int fd_bpp)
{
    return 0;
}

/*****************************************************************************/
int
yami_encoder_encode(void *obj, void *cdata, int *cdata_max_bytes)
{
    return 0;
}

/*****************************************************************************/
int
yami_decoder_create(void **obj, int width, int height, int type, int flags)
{
    return 0;
}

/*****************************************************************************/
int
yami_decoder_delete(void *obj)
{
    return 0;
}

/*****************************************************************************/
int
yami_decoder_decode(void *obj, void *cdata, int cdata_bytes)
{
    return 0;
}

/*****************************************************************************/
int
yami_decoder_get_pixmap(void *obj, void* display,
                        int width, int height, int *pixmap)
{
    return 0;
}
