
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

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
    VASurfaceID enc_surface;
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
    vars->video_param.mfx.CodecId = MFX_CODEC_AVC;
    vars->video_param.mfx.CodecProfile = MFX_PROFILE_AVC_MAIN;
    vars->video_param.IOPattern = MFX_IOPATTERN_IN_VIDEO_MEMORY;
    vars->video_param.AsyncDepth = 1;
    vars->video_param.mfx.TargetUsage = 4;
    vars->video_param.mfx.FrameInfo.FrameRateExtN = 24;
    vars->video_param.mfx.FrameInfo.FrameRateExtD = 1;
    vars->video_param.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
    vars->video_param.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
    vars->video_param.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
    vars->video_param.mfx.FrameInfo.Width = 1280;
    vars->video_param.mfx.FrameInfo.Height = 720;
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
    va_status = vaCreateSurfaces(vars->va_display, VA_RT_FORMAT_YUV420,
                                 1280, 720, &(vars->enc_surface), 1, NULL, 0);
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
    index = 10;
    while (--index)
    {
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
