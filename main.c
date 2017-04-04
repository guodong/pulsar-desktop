#include <stdio.h>
#include <stdlib.h>
#include <webp/encode.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/Xdamage.h>
#include <sys/time.h>
#include <libwebsockets.h>
#include <string.h>
#include <pthread.h>
static int callback_protocol(struct libwebsocket_context *this,
                             struct libwebsocket *wsi,
                             enum libwebsocket_callback_reasons reason,
                             void *user,
                             void *in,
                             size_t len);

Display *display;
Window root;
struct libwebsocket_context *context;

struct Region {
    int x;
    int y;
    int width;
    int height;
    int init;
};

struct Region region;

static struct libwebsocket_protocols protocols[] = {
    {
        "cloudware-interacting-protocol",
        callback_protocol,
        0
    },
    {
        NULL, NULL, 0
    }
};

void region_add(int x, int y, int width, int height)
{
    if (region.init) {
        region.x = x;
        region.y = y;
        region.width = width;
        region.height = height;
        region.init = 0;
        return;
    }
    int x1 = x + width;
    int y1 = y + height;
    int ori_x1 = region.x + region.width;
    int ori_y1 = region.y + region.height;
    if (x < region.x) {
        region.x = x;
    }
    if (y < region.y) {
        region.y = y;
    }
    if (x1 > ori_x1) {
        region.width = x1 - region.x;
    } else {
        region.width = ori_x1 - region.x;
    }

    if (y1 > ori_y1) {
        region.height = y1 - region.y;
    } else {
        region.height = ori_y1 - region.y;
    }
}

void xserver_thread(void *data)
{
    struct libwebsocket *wsi = (struct libwebsocket*)data;

    Display *display = XOpenDisplay(NULL);
    root = RootWindow(display, DefaultScreen(display));

    int damage_event_base, damage_error_base;
    XDamageQueryExtension(display, &damage_event_base, &damage_error_base);
    Damage damage_handle = XDamageCreate(display, root, XDamageReportRawRectangles);
    XEvent xevt;
    XDamageNotifyEvent *damage_event;
    uint8_t *output;
    int output_size, i;

    while (1) {
        XNextEvent(display, &xevt);
        switch (xevt.type) {
        default:
            if (xevt.type == damage_event_base + XDamageNotify) {
                damage_event = (XDamageNotifyEvent*)&xevt;

                int x = damage_event->area.x;
                int y = damage_event->area.y;
                int width = damage_event->area.width;
                int height = damage_event->area.height;
                if (width == 0 || height == 0) {
                    break;
                }
                region_add(x, y, width, height);
            }
            break;
        }
    }

}

static int callback_protocol(struct libwebsocket_context *this,
                             struct libwebsocket *wsi,
                             enum libwebsocket_callback_reasons reason,
                             void *user,
                             void *in,
                             size_t len)
{
    switch (reason) {
    case LWS_CALLBACK_ESTABLISHED:
        printf("connection established\n");

        pthread_t xserver_thrd;
        pthread_create(&xserver_thrd, NULL, (void*)xserver_thread, wsi);
        region_add(0, 0, 1440, 900);
        break;
    case LWS_CALLBACK_RECEIVE: {
        char *data = (char*)in;
        uint8_t type;
        memcpy(&type, in, 1);
        switch (type) {
        case 0: {
            uint16_t x, y;
            memcpy(&x, in + 1, 2);
            memcpy(&y, in + 3, 2);
            XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
            XFlush(display);
            break;
        }
        case 1: {
            uint32_t code = 0;
            memcpy(&code, in + 1, 4);
            XTestFakeButtonEvent(display, code, True, CurrentTime);
            XFlush(display);
            break;
        }
        case 2: {
            uint32_t code = 0;
            memcpy(&code, in + 1, 4);
            XTestFakeButtonEvent(display, code, False, CurrentTime);
            XFlush(display);
            break;
        }
        case 3: {
            uint32_t code = 0;
            memcpy(&code, in + 1, 4);
            XTestFakeKeyEvent(display, code, True, CurrentTime);
            XFlush(display);
            break;
        }
        case 4: {
            uint32_t code = 0;
            memcpy(&code, in + 1, 4);
            XTestFakeKeyEvent(display, code, False, CurrentTime);
            XFlush(display);
            break;
        }
        default:
            break;
        }

        break;
    }
    case LWS_CALLBACK_SERVER_WRITEABLE: {
        int output_size, i;
        uint8_t *output;
        if (!region.init) {
            if (region.width == 0 || region.height == 0) {
                break;
            }
            int x = region.x;
            int y = region.y;
            int width = region.width;
            int height = region.height;
            region.init = 1;
            XImage *ximg;
            ximg = XGetImage(display, root, x, y, width, height, AllPlanes, ZPixmap);

            for (i = 3; i < (width * height << 2); i += 4) {
                ximg->data[i] = 0xff;
            }
            output_size = WebPEncodeBGRA(ximg->data, width, height, width << 2, 80, &output);
            unsigned char *buf = malloc(LWS_SEND_BUFFER_PRE_PADDING + output_size + 8 + LWS_SEND_BUFFER_POST_PADDING);
            memcpy(buf + LWS_SEND_BUFFER_PRE_PADDING, &x, 4);
            memcpy(buf + LWS_SEND_BUFFER_PRE_PADDING + 4, &y, 4);
            memcpy(buf + LWS_SEND_BUFFER_PRE_PADDING + 8, output, output_size);
            libwebsocket_write(wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING], output_size + 8, LWS_WRITE_BINARY);
            free(output);
            free(ximg->data);
            free(ximg);
        }
        break;
    }
    default:
        break;
    }
    return 0;
}

int main()
{
    printf("trying to connect xserver...\n");
    while (1) {
        display = XOpenDisplay(NULL);
        if (display) {
            printf("success connected to xserver\n");
            break;
        }
        sleep(1);
    }
    region.init = 1;
    region.x = 0;
    region.y = 0;
    region.width = 0;
    region.height = 0;
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = 5678;
    info.protocols = protocols;

    context = libwebsocket_create_context(&info);

    while (1) {
        libwebsocket_callback_on_writable_all_protocol(protocols);
        libwebsocket_service(context, 20);
        usleep(1000);
    }
    return 0;
}
