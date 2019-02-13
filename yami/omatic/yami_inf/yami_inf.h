
#ifndef __YAMI_INF_H__
#define __YAMI_INF_H__ 1

#define YAMI_INF_MAJOR 0
#define YAMI_INF_MINOR 1

int
yami_get_version(int *version);
int
yami_init(int type, void *display);
int
yami_deinit(void);

int
yami_encoder_create(void **obj, int width, int height, int type, int flags);
int
yami_encoder_delete(void *obj);
int
yami_encoder_get_width(void *obj, int *width);
int
yami_encoder_get_height(void *obj, int *height);
int
yami_encoder_resize(void *obj, int width, int height);
int
yami_encoder_get_ybuffer(void *obj, void **ydata, int *ydata_stride_bytes);
int
yami_encoder_get_uvbuffer(void *obj, void **uvdata, int *uvdata_stride_bytes);
int
yami_encoder_set_fd_src(void *obj, int fd, int fd_width, int fd_height,
                        int fd_stride, int fd_size, int fd_bpp);
int
yami_encoder_encode(void *obj, void *cdata, int *cdata_max_bytes);

int
yami_decoder_create(void **obj, int width, int height, int type, int flags);
int
yami_decoder_delete(void *obj);
int
yami_decoder_decode(void *obj, void *cdata, int cdata_bytes);
int
yami_decoder_get_pixmap(void *obj, void* display,
                        int width, int height, int *pixmap);

#endif
