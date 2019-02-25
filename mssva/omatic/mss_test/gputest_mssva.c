
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

int main(int argc, char** argv)
{
    int fd;
    int va_major;
    int va_minor;
    VADisplay va_display;
    VAStatus va_status;

    fd = open("/dev/dri/renderD128", O_RDONLY);
    if (fd == -1)
    {
        printf("main: error open /dev/renderD128\n");
        return 1;
    }
    va_display = vaGetDisplayDRM(fd);
    if (va_display == 0)
    {
        printf("main: vaGetDisplayDRM failed\n");
        close(fd);
        return 1;
    }
    va_status = vaInitialize(va_display, &va_major, &va_minor);
    vaTerminate(va_display);
    close(fd);
    return 0;
}
