
#if !defined( __MSSVA_INF_H__)
#define __MSSVA_INF_H__ 1

#define MI_MAJOR                                    0
#define MI_MINOR                                    1

#define MI_SUCCESS                                  0
#define MI_ERROR_MEMORY                             1
#define MI_ERROR_UNIMP                              2
#define MI_ERROR_TYPE                               3
#define MI_ERROR_OTHER                              4
#define MI_ERROR_VAINITIALIZE                       100
#define MI_ERROR_VACREATECONFIG                     101
#define MI_ERROR_VACREATECONTEXT                    102
#define MI_ERROR_VACREATESURFACES                   103
#define MI_ERROR_VACREATEIMAGE                      104
#define MI_ERROR_VAMAPBUFFER                        105
#define MI_ERROR_VAUNMAPBUFFER                      106
#define MI_ERROR_VAPUTIMAGE                         107
#define MI_ERROR_VASYNCSURFACE                      108
#define MI_ERROR_VAPUTSURFACE                       109
#define MI_ERROR_VADESTROYIMAGE                     110
#define MI_ERROR_VADESTROYSURFACES                  111

#define MI_ERROR_MFXVIDEOENCODE_QUERY               200
#define MI_ERROR_MFXVIDEOENCODE_QUERYIOSURF         201
#define MI_ERROR_MFXVIDEOENCODE_INIT                202
#define MI_ERROR_MFXVIDEOENCODE_CLOSE               203
#define MI_ERROR_MFXINIT                            204
#define MI_ERROR_MFXVIDEOCORE_SETHANDLE             205
#define MI_ERROR_MFXVIDEOCORE_SETFRAMEALLOCATOR     206
#define MI_ERROR_MFXVIDEOENCODE_ENCODEFRAMEASYNC    207
#define MI_ERROR_MFXVIDEOCORE_SYNCOPERATION         208
#define MI_ERROR_MFXVIDEOENCODE_RESET               209

#define MI_TYPE_H264                                1

#define MI_H264_ENC_FLAGS_PROFILE_MASK              0x0000000F
#define MI_H264_ENC_FLAGS_PROFILE_MAIN              (1 << 0)
#define MI_H264_ENC_FLAGS_PROFILE_HIGH              (2 << 0)

#define MI_TYPE_DRM                                 1

#define MI_H264_DEC_FLAG_LOWLATENCY                 1

int
mssva_get_version(int *version);
int
mssva_init(int type, void *display);
int
mssva_deinit(void);

int
mssva_encoder_create(void **obj, int width, int height, int type, int flags);
int
mssva_encoder_delete(void *obj);
int
mssva_encoder_get_width(void *obj, int *width);
int
mssva_encoder_get_height(void *obj, int *height);
int
mssva_encoder_resize(void *obj, int width, int height);
int
mssva_encoder_get_ybuffer(void *obj, void **ydata, int *ydata_stride_bytes);
int
mssva_encoder_get_uvbuffer(void *obj, void **uvdata, int *uvdata_stride_bytes);
int
mssva_encoder_set_fd_src(void *obj, int fd, int fd_width, int fd_height,
                         int fd_stride, int fd_size, int fd_bpp);
int
mssva_encoder_encode(void *obj, void *cdata, int *cdata_max_bytes);

#endif
