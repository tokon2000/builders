
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <va/va.h>
#include <va/va_drm.h>

#include <mfxvideo.h>
#include <mfxplugin.h>

struct _vars
{
    int fd;
    int va_major;
    int va_minor;
    int pad0;
    VADisplay va_display;
    mfxVersion version;
    mfxIMPL imp;
    mfxSession session;
    mfxVideoParam video_param;
    mfxVideoParam video_param_out;
    mfxFrameAllocRequest fa_request;
    mfxFrameAllocator frame_allocator;
    VASurfaceID va_surfaces[16];
    mfxMemId va_surfaces1[16];
    int num_va_surfaces;
    int pad1;
    mfxEncodeCtrl ctrl;
    mfxFrameSurface1 surface;
    mfxBitstream bs;
    mfxSyncPoint syncp;
    VASurfaceAttrib attribs;
    VASurfaceID rgb_surface;
    VASurfaceID enc_surface;
    VAImage va_image;
    VAImageFormat va_image_format;
    VAConfigID vp_config;
    VAContextID vp_context;
};

struct bmp_magic
{
    char magic[2];
};

struct bmp_hdr
{
    unsigned int   size;        /* file size in bytes */
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int   offset;      /* offset to image data, in bytes */
};

struct dib_hdr
{
    unsigned int   hdr_size;
    int            width;
    int            height;
    unsigned short nplanes;
    unsigned short bpp;
    unsigned int   compress_type;
    unsigned int   image_size;
    int            hres;
    int            vres;
    unsigned int   ncolors;
    unsigned int   nimpcolors;
};

/*****************************************************************************/
void
hexdump(const void *p, int len)
{
    unsigned char *line;
    int i;
    int thisline;
    int offset;

    line = (unsigned char *)p;
    offset = 0;
    while (offset < len)
    {
        printf("%04x ", offset);
        thisline = len - offset;
        if (thisline > 16)
        {
            thisline = 16;
        }
        for (i = 0; i < thisline; i++)
        {
            printf("%02x ", line[i]);
        }
        for (; i < 16; i++)
        {
            printf("   ");
        }
        for (i = 0; i < thisline; i++)
        {
            printf("%c", (line[i] >= 0x20 && line[i] < 0x7f) ? line[i] : '.');
        }
        printf("\n");
        offset += thisline;
        line += thisline;
    }
}

/*****************************************************************************/
mfxStatus MFX_CDECL
frame_alloc(mfxHDL pthis, mfxFrameAllocRequest *request,
            mfxFrameAllocResponse *response)
{
    struct _vars* vars;
    VAStatus va_status;
    int index;

    vars = (struct _vars*)pthis;
    printf("frame_alloc: Type 0x%8.8x NumFrameSuggested %d\n",
           request->Type, request->NumFrameSuggested);
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
    vars->num_va_surfaces = request->NumFrameSuggested;
    va_status = vaCreateSurfaces(vars->va_display, VA_RT_FORMAT_YUV420,
                                 request->Info.Width, request->Info.Height,
                                 vars->va_surfaces,
                                 vars->num_va_surfaces, NULL, 0);
    if (va_status != VA_STATUS_SUCCESS)
    {
        return MFX_ERR_MEMORY_ALLOC;
    }
    for (index = vars->num_va_surfaces - 1; index >= 0; index--)
    {
        vars->va_surfaces1[index] = &(vars->va_surfaces[index]);
        printf("frame_alloc: surfaceid %d\n", vars->va_surfaces[index]);
    }
    response->mids = vars->va_surfaces1;
    response->NumFrameActual = vars->num_va_surfaces;
    return MFX_ERR_NONE;
}

/*****************************************************************************/
mfxStatus MFX_CDECL
frame_lock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr)
{
    printf("frame_lock:\n");
    return MFX_ERR_NONE;
}

/*****************************************************************************/
mfxStatus MFX_CDECL
frame_unlock(mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr)
{
    printf("frame_unlock:\n");
    return MFX_ERR_NONE;
}

/*****************************************************************************/
mfxStatus MFX_CDECL
frame_get_hdl(mfxHDL pthis, mfxMemId mid, mfxHDL *handle)
{
    printf("frame_get_hdl: mid %d\n", *((int*)mid));
    *handle = mid;
    return MFX_ERR_NONE;
}

/*****************************************************************************/
mfxStatus MFX_CDECL
frame_free(mfxHDL pthis, mfxFrameAllocResponse *response)
{
    struct _vars* vars;
    int index;
    VASurfaceID sur;

    printf("frame_free:\n");
    vars = (struct _vars*)pthis;
    for (index = response->NumFrameActual - 1; index >= 0; index--)
    {
        sur = *((VASurfaceID*)(response->mids[index]));
        vaDestroySurfaces(vars->va_display, &sur, 1);
    }
    return MFX_ERR_NONE;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

char g_filename[] = "/tmp/h264_beef.bin";

/*****************************************************************************/
static int
save_data(char* data, int bytes, int width, int height)
{
    int fd;
    struct _header
    {
        char text[4];
        int width;
        int height;
        int bytes_follow;
    } header;

    header.text[0] = 'B';
    header.text[1] = 'E';
    header.text[2] = 'E';
    header.text[3] = 'F';
    header.width = width;
    header.height = height;
    header.bytes_follow = bytes;
    fd = open(g_filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    lseek(fd, 0, SEEK_END);
    if (write(fd, &header, sizeof(header)) != sizeof(header))
    {
        printf("save_data: write failed\n");
    }
    if (write(fd, data, bytes) != bytes)
    {
        printf("save_data: write failed\n");
    }
    close(fd);
    return 0;
}

#define CLAMP(_val, _lo, _hi) \
    ((_val) < (_lo) ? (_lo) : (_val) > (_hi) ? (_hi) : (_val))

/******************************************************************************/
int
r8g8b8_to_nv12_box(const unsigned char *s8, int src_stride,
                   unsigned char *d8_y, int dst_stride_y,
                   unsigned char *d8_uv, int dst_stride_uv,
                   int width, int height)
{
    int index;
    int jndex;
    int R;
    int G;
    int B;
    int Y;
    int U;
    int V;
    int U_sum;
    int V_sum;
    const unsigned char *s8a;
    const unsigned char *s8b;
    unsigned char *d8ya;
    unsigned char *d8yb;
    unsigned char *d8uv;

    for (jndex = 0; jndex < height; jndex += 2)
    {
        s8a = (const unsigned char *) (s8 + src_stride * jndex);
        s8b = (const unsigned char *) (s8 + src_stride * (jndex + 1));
        d8ya = d8_y + dst_stride_y * jndex;
        d8yb = d8_y + dst_stride_y * (jndex + 1);
        d8uv = d8_uv + dst_stride_uv * (jndex / 2);
        for (index = 0; index < width; index += 2)
        {
            U_sum = 0;
            V_sum = 0;

            B = *(s8a++);
            G = *(s8a++);
            R = *(s8a++);
            Y = (( 66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
            U = ((-38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
            V = ((112 * R -  94 * G -  18 * B + 128) >> 8) + 128;
            d8ya[0] = CLAMP(Y, 0, 255);
            d8ya++;
            U_sum += CLAMP(U, 0, 255);
            V_sum += CLAMP(V, 0, 255);

            B = *(s8a++);
            G = *(s8a++);
            R = *(s8a++);
            Y = (( 66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
            U = ((-38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
            V = ((112 * R -  94 * G -  18 * B + 128) >> 8) + 128;
            d8ya[0] = CLAMP(Y, 0, 255);
            d8ya++;
            U_sum += CLAMP(U, 0, 255);
            V_sum += CLAMP(V, 0, 255);

            B = *(s8b++);
            G = *(s8b++);
            R = *(s8b++);
            Y = (( 66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
            U = ((-38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
            V = ((112 * R -  94 * G -  18 * B + 128) >> 8) + 128;
            d8yb[0] = CLAMP(Y, 0, 255);
            d8yb++;
            U_sum += CLAMP(U, 0, 255);
            V_sum += CLAMP(V, 0, 255);

            B = *(s8b++);
            G = *(s8b++);
            R = *(s8b++);
            Y = (( 66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
            U = ((-38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
            V = ((112 * R -  94 * G -  18 * B + 128) >> 8) + 128;
            d8yb[0] = CLAMP(Y, 0, 255);
            d8yb++;
            U_sum += CLAMP(U, 0, 255);
            V_sum += CLAMP(V, 0, 255);

            d8uv[0] = (U_sum + 2) / 4;
            d8uv++;
            d8uv[0] = (V_sum + 2) / 4;
            d8uv++;
        }
    }
    return 0;
}

/*****************************************************************************/
static int
read_surface(struct _vars* vars, int index)
{
    int fd;
    char filename[256];
    struct bmp_magic mag;
    struct bmp_hdr hdr;
    struct dib_hdr dib;
    char* image;
    char* limg;
    int lindex;
    int ljndex;
    int Bpp;
    void *buf_ptr;
    VAStatus va_status;
    char* rgbdata;
    VAProcPipelineParameterBuffer params;
    VABufferID buf;
    int* dst32;

    snprintf(filename, 255, "/home/jsorg/enc_test/frame%2.2d.bmp", index);
    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        return 1;
    }
    if (read(fd, &mag, sizeof(mag)) != sizeof(mag))
    {
        close(fd);
        return 1;
    }
    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr))
    {
        close(fd);
        return 1;
    }
    if (read(fd, &dib, sizeof(dib)) != sizeof(dib))
    {
        close(fd);
        return 1;
    }
    printf("bpp %d\n", dib.bpp);
    Bpp = dib.bpp / 8;
    image = malloc(dib.width * dib.height * Bpp);
    for (lindex = 0; lindex < dib.height; lindex++)
    {
        limg = (image + (dib.width * dib.height * Bpp)) - (dib.width * Bpp * (lindex + 1));
        if (read(fd, limg, dib.width * Bpp) != dib.width * Bpp)
        {
            free(image);
            close(fd);
            return 1;
        }
    }

    vars->attribs.type = VASurfaceAttribPixelFormat;
    vars->attribs.flags = VA_SURFACE_ATTRIB_SETTABLE;
    vars->attribs.value.type = VAGenericValueTypeInteger;
    vars->attribs.value.value.i = VA_FOURCC_BGRA;
    //vars->attribs.value.value.i = VA_FOURCC_ARGB;
    va_status = vaCreateSurfaces(vars->va_display, VA_RT_FORMAT_RGB32,
                                 400, 300, &(vars->rgb_surface), 1, &vars->attribs, 1);
    printf("vaCreateSurfaces va_status %d\n", va_status);
    vars->va_image_format.fourcc = VA_FOURCC_BGRA;
    //vars->va_image_format.fourcc = VA_FOURCC_ARGB;
    vars->va_image_format.byte_order = VA_LSB_FIRST;
    vars->va_image_format.bits_per_pixel = 32;
    vars->va_image_format.depth = 24;
    vars->va_image_format.red_mask = 0x00FF0000;
    vars->va_image_format.green_mask = 0x0000FF00;
    vars->va_image_format.blue_mask = 0x000000FF;
    vars->va_image_format.alpha_mask = 0xFF000000;
    va_status = vaCreateImage(vars->va_display, &vars->va_image_format,
                              400, 300, &vars->va_image);
    printf("vaCreateImage va_status %d\n", va_status);
    va_status = vaMapBuffer(vars->va_display, vars->va_image.buf, &buf_ptr);
    printf("vaMapBuffer va_status %d pitch %d\n", va_status, vars->va_image.pitches[0]);
    rgbdata = buf_ptr;
    limg = image;
    for (lindex = 0; lindex < dib.height; lindex++)
    {
        dst32 = (int*) (rgbdata + vars->va_image.offsets[0] + lindex * vars->va_image.pitches[0]);
        for (ljndex = 0; ljndex < dib.width; ljndex++)
        {
            dst32[ljndex] = limg[0] | (limg[1] << 8) | (limg[2] << 16) | (0xFF << 24);
            limg += 3;
        }
    }
    va_status = vaUnmapBuffer(vars->va_display, vars->va_image.buf);
    printf("vaUnmapBuffer va_status %d\n", va_status);
    va_status = vaPutImage(vars->va_display, vars->rgb_surface,
                           vars->va_image.image_id,
                           0, 0, dib.width, dib.height,
                           0, 0, dib.width, dib.height);
    printf("vaPutImage va_status %d\n", va_status);
    va_status = vaSyncSurface(vars->va_display, vars->rgb_surface);
    printf("vaSyncSurface va_status %d\n", va_status);
    memset(&params, 0, sizeof(params));
    params.surface = vars->rgb_surface;
    //params.surface_color_standard = VAProcColorStandardNone;
    //params.output_color_standard = VAProcColorStandardBT709; // VAProcColorStandardBT601;
    params.output_color_standard = VAProcColorStandardBT601;
    va_status = vaCreateBuffer(vars->va_display, vars->vp_context,
                               VAProcPipelineParameterBufferType,
                               sizeof(params), 1, &params, &buf);
    printf("vaCreateBuffer va_status %d\n", va_status);
    va_status = vaBeginPicture(vars->va_display, vars->vp_context,
                               vars->enc_surface);
    printf("vaBeginPicture va_status %d\n", va_status);
    va_status = vaRenderPicture(vars->va_display, vars->vp_context, &buf, 1);
    printf("vaRenderPicture va_status %d\n", va_status);
    va_status = vaEndPicture(vars->va_display, vars->vp_context);
    printf("vaEndPicture va_status %d\n", va_status);
    va_status = vaSyncSurface(vars->va_display, vars->enc_surface);
    printf("vaSyncSurface va_status %d\n", va_status);
    vaDestroyBuffer(vars->va_display, buf);
    va_status = vaDestroyImage(vars->va_display, vars->va_image.image_id);
    printf("vaDestroyImage va_status %d\n", va_status);
    va_status = vaDestroySurfaces(vars->va_display, &(vars->rgb_surface), 1);
    printf("vaDestroySurfaces va_status %d\n", va_status);
    free(image);
    close(fd);
    return 0;
}

/*****************************************************************************/
static int
read_surface1(struct _vars* vars, int index)
{
    int fd;
    char filename[256];
    struct bmp_magic mag;
    struct bmp_hdr hdr;
    struct dib_hdr dib;
    char* image;
    char* limg;
    int lindex;
    int Bpp;
    void *buf_ptr;
    VAStatus va_status;
    char* yuvdata;

    snprintf(filename, 255, "/home/jsorg/enc_test/frame%2.2d.bmp", index);
    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        return 1;
    }
    if (read(fd, &mag, sizeof(mag)) != sizeof(mag))
    {
        close(fd);
        return 1;
    }
    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr))
    {
        close(fd);
        return 1;
    }
    if (read(fd, &dib, sizeof(dib)) != sizeof(dib))
    {
        close(fd);
        return 1;
    }
    printf("bpp %d\n", dib.bpp);
    Bpp = dib.bpp / 8;
    image = malloc(dib.width * dib.height * Bpp);
    for (lindex = 0; lindex < dib.height; lindex++)
    {
        limg = (image + (dib.width * dib.height * Bpp)) - (dib.width * Bpp * (lindex + 1));
        if (read(fd, limg, dib.width * Bpp) != dib.width * Bpp)
        {
            free(image);
            close(fd);
            return 1;
        }
    }

    vars->va_image_format.fourcc = VA_FOURCC_NV12;
    va_status = vaCreateImage(vars->va_display, &vars->va_image_format,
                              400, 300, &vars->va_image);

    va_status = vaMapBuffer(vars->va_display, vars->va_image.buf, &buf_ptr);
    printf("vaMapBuffer va_status %d\n", va_status);
    yuvdata = buf_ptr;
    r8g8b8_to_nv12_box(image, dib.width * Bpp,
                       yuvdata + vars->va_image.offsets[0], vars->va_image.pitches[0],
                       yuvdata + vars->va_image.offsets[1], vars->va_image.pitches[1],
                       dib.width, dib.height);
    va_status = vaUnmapBuffer(vars->va_display, vars->va_image.buf);
    printf("vaUnmapBuffer va_status %d\n", va_status);
    va_status = vaPutImage(vars->va_display, vars->enc_surface,
                           vars->va_image.image_id,
                           0, 0, dib.width, dib.height,
                           0, 0, dib.width, dib.height);
    printf("vaPutImage va_status %d\n", va_status);
    va_status = vaSyncSurface(vars->va_display, vars->enc_surface);
    printf("vaSyncSurface va_status %d\n", va_status);
    va_status = vaDestroyImage(vars->va_display, vars->va_image.image_id);
    printf("vaDestroyImage va_status %d\n", va_status);
    free(image);
    close(fd);
    return 0;
}

/*****************************************************************************/
int
main(int argc, char** argv)
{
    struct _vars* vars;
    VAStatus va_status;
    mfxStatus status;
    int index;

    vars = (struct _vars*)calloc(1, sizeof(struct _vars));
    vars->version.Minor = MFX_VERSION_MINOR;
    vars->version.Major = MFX_VERSION_MAJOR;
    //vars->version.Minor = 24;
    //vars->version.Major = 1;

    vars->frame_allocator.pthis = vars;
    vars->frame_allocator.Alloc = frame_alloc;
    vars->frame_allocator.Lock = frame_lock;
    vars->frame_allocator.Unlock = frame_unlock;
    vars->frame_allocator.GetHDL = frame_get_hdl;
    vars->frame_allocator.Free = frame_free;

    vars->fd = open("/dev/dri/renderD128", O_RDWR);
    if (vars->fd == -1)
    {
        printf("main: error open /dev/dri/renderD128\n");
        return 1;
    }
    vars->va_display = vaGetDisplayDRM(vars->fd);
    if (vars->va_display == 0)
    {
        printf("main: vaGetDisplayDRM failed\n");
        close(vars->fd);
        return 1;
    }
    va_status = vaInitialize(vars->va_display,
                             &(vars->va_major), &(vars->va_minor));
    if (va_status != VA_STATUS_SUCCESS)
    {
        printf("main: vaInitialize failed\n");
        close(vars->fd);
        return 1;
    }

    va_status = vaCreateConfig(vars->va_display, VAProfileNone,
                               VAEntrypointVideoProc, NULL, 0, &vars->vp_config);
    va_status = vaCreateContext(vars->va_display, vars->vp_config, 0, 0, 0, NULL, 0,
                                &vars->vp_context);

    vars->imp = MFX_IMPL_HARDWARE | MFX_IMPL_VIA_VAAPI;
    status = MFXInit(vars->imp, &(vars->version), &(vars->session));
    if (status != MFX_ERR_NONE)
    {
        printf("main: MFXInit failed %d\n", status);
        vaTerminate(vars->va_display);
        close(vars->fd);
        return 1;
    }
    status = MFXVideoCORE_SetHandle(vars->session, MFX_HANDLE_VA_DISPLAY,
                                    vars->va_display);
    if (status != MFX_ERR_NONE)
    {
        printf("main: MFXVideoCORE_SetHandle failed\n");
        MFXClose(vars->session);
        vaTerminate(vars->va_display);
        close(vars->fd);
        return 1;
    }
    status = MFXVideoCORE_SetFrameAllocator(vars->session,
                                            &(vars->frame_allocator));
    if (status != MFX_ERR_NONE)
    {
        printf("main: MFXVideoCORE_SetFrameAllocator failed\n");
        MFXClose(vars->session);
        vaTerminate(vars->va_display);
        close(vars->fd);
        return 1;
    }
#if 0
    //mfxPluginUID uid = MFX_PLUGINID_HEVCE_HW;
    mfxPluginUID uid = MFX_PLUGINID_VP9E_HW;
    //const char* path = "/opt/mssva/plugins";
    //const char* path = "/opt/mssva/plugins/libmfx_hevce_hw64.so";
    //status = MFXVideoUSER_LoadByPath(vars->session, &uid, 1, path, strlen(path));
    //printf("main: MFXVideoUSER_LoadByPath status %d\n", status);
    status = MFXVideoUSER_Load(vars->session, &uid, 1);
    printf("main: MFXVideoUSER_Load status %d\n", status);
    if (status != MFX_ERR_NONE)
    {
    }
#endif
    vars->video_param.mfx.CodecId = MFX_CODEC_AVC;
    vars->video_param.mfx.CodecProfile = MFX_PROFILE_AVC_MAIN;
    //vars->video_param.mfx.CodecId = MFX_CODEC_HEVC;
    //vars->video_param.mfx.CodecProfile = MFX_PROFILE_HEVC_MAIN;
    //vars->video_param.mfx.CodecId = MFX_CODEC_VP9;
    //vars->video_param.mfx.CodecProfile = MFX_PROFILE_VP9_0;
    vars->video_param.IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY;
    vars->video_param.AsyncDepth = 1;
    vars->video_param.mfx.TargetUsage = 4;
    vars->video_param.mfx.FrameInfo.FrameRateExtN = 24;
    vars->video_param.mfx.FrameInfo.FrameRateExtD = 1;
    vars->video_param.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
    vars->video_param.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    vars->video_param.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    //vars->video_param.mfx.FrameInfo.CropX = 0;
    //vars->video_param.mfx.FrameInfo.CropY = 0;
    //vars->video_param.mfx.FrameInfo.CropW = 1280;
    //vars->video_param.mfx.FrameInfo.CropH = 720;
    vars->video_param.mfx.FrameInfo.Width = (400 + 15) & ~15;
    vars->video_param.mfx.FrameInfo.Height = (300 + 15) & ~15;
    vars->video_param.mfx.RateControlMethod = MFX_RATECONTROL_CQP;
    vars->video_param.mfx.QPI = 28;
    vars->video_param.mfx.QPB = 28;
    vars->video_param.mfx.QPP = 28;
    vars->video_param.mfx.GopPicSize = 1024;
    vars->video_param.mfx.NumRefFrame = 1;
    /* low latency, I and P frames only */
    vars->video_param.mfx.GopRefDist = 1;
    vars->video_param.mfx.NumSlice = 1;

    vars->video_param_out = vars->video_param;
    status = MFXVideoENCODE_Query(vars->session, &(vars->video_param),
                                  &(vars->video_param_out));
    if (status != MFX_ERR_NONE)
    {
        if (status < MFX_ERR_NONE)
        {
            printf("main: MFXVideoENCODE_Query failed status %d\n", status);
            MFXClose(vars->session);
            vaTerminate(vars->va_display);
            close(vars->fd);
            return 1;
        }
        printf("main: MFXVideoENCODE_Query warning status %d\n", status);
    }

    //printf("main: video_param\n");
    //hexdump(&(vars->video_param), sizeof(vars->video_param));
    //printf("main: video_param_out\n");
    //hexdump(&(vars->video_param_out), sizeof(vars->video_param_out));

    status = MFXVideoENCODE_QueryIOSurf(vars->session,
                                        &(vars->video_param_out),
                                        &(vars->fa_request));
    if (status != MFX_ERR_NONE)
    {
        if (status < MFX_ERR_NONE)
        {
            printf("main: MFXVideoENCODE_QueryIOSurf failed status %d\n",
                   status);
            MFXClose(vars->session);
            vaTerminate(vars->va_display);
            close(vars->fd);
            return 1;
        }
        printf("main: MFXVideoENCODE_QueryIOSurf warning status %d\n", status);
    }
    status = MFXVideoENCODE_Init(vars->session, &(vars->video_param_out));
    if (status != MFX_ERR_NONE)
    {
        if (status < MFX_ERR_NONE)
        {
            printf("main: MFXVideoENCODE_Init failed status %d\n", status);
            MFXClose(vars->session);
            vaTerminate(vars->va_display);
            close(vars->fd);
            return 1;
        }
        printf("main: MFXVideoENCODE_Init warning status %d\n", status);
    }
    vars->attribs.type = VASurfaceAttribPixelFormat;
    vars->attribs.flags = VA_SURFACE_ATTRIB_SETTABLE;
    vars->attribs.value.type = VAGenericValueTypeInteger;
    vars->attribs.value.value.i = VA_FOURCC_NV12;
    va_status = vaCreateSurfaces(vars->va_display, VA_RT_FORMAT_YUV420,
                                 400, 300, &(vars->enc_surface), 1, &vars->attribs, 1);
    if (va_status != VA_STATUS_SUCCESS)
    {
        printf("main: vaCreateSurfaces failed status %d\n", va_status);
        MFXVideoENCODE_Close(vars->session);
        MFXClose(vars->session);
        vaTerminate(vars->va_display);
        close(vars->fd);
        return 1;
    }
    printf("main: surfaceid %d\n", vars->enc_surface);

    vars->surface.Info = vars->video_param_out.mfx.FrameInfo;
    vars->bs.MaxLength = 1024 * 1024 * 16;
    vars->bs.Data = (unsigned char*)malloc(vars->bs.MaxLength);
    index = 0;
    while (1)
    {
        if (read_surface(vars, index) != 0)
        {
            break;
        }
        index++;
        vars->syncp = NULL;
        vars->bs.DataLength = 0;
        vars->surface.Data.MemId = &(vars->enc_surface);
        status = MFXVideoENCODE_EncodeFrameAsync(vars->session, &(vars->ctrl),
                                                 &(vars->surface), &(vars->bs),
                                                 &(vars->syncp));
        printf("main: MFXVideoENCODE_EncodeFrameAsync status %d\n", status);
        if (status >= MFX_ERR_NONE)
        {
            if (vars->syncp != NULL)
            {
                status = MFXVideoCORE_SyncOperation(vars->session,
                                                    vars->syncp, 1000);
                printf("main: MFXVideoCORE_SyncOperation status %d\n", status);
            }
        }
        printf("main: MFXVideoCORE_SyncOperation DataLength %d\n",
               vars->bs.DataLength);
        hexdump(vars->bs.Data, vars->bs.DataLength);
        save_data(vars->bs.Data, vars->bs.DataLength, 400, 300);
    }
    vaDestroySurfaces(vars->va_display, &(vars->enc_surface), 1);
    free(vars->bs.Data);
    MFXVideoENCODE_Close(vars->session);
    MFXClose(vars->session);
    vaTerminate(vars->va_display);
    close(vars->fd);
    free(vars);
    return 0;
}
