#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>

#include <unistd.h>
#include <time.h>
#include <math.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__);

#define dienull(p) _dienull(p, __LINE__);
void
_dienull(void *p, int line)
{
    if (!p) {
        eprintf("%d: null m8\n", line);
        exit(1);
    }
}

xcb_screen_t *
screen_of_display (xcb_connection_t *c,
                                 int               screen)
{
  xcb_screen_iterator_t iter;

  iter = xcb_setup_roots_iterator (xcb_get_setup (c));
  for (; iter.rem; --screen, xcb_screen_next (&iter))
    if (screen == 0)
      return iter.data;

  return NULL;
}

unsigned long
gettime()
{
    static struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int
main(void)
{
    xcb_connection_t *c;
    xcb_screen_t *scr;
    xcb_window_t w = {0};
    xcb_gcontext_t gc;

    c = xcb_connect(0, 0);

    scr = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
    dienull(scr);
    w = scr->root;

    gc = xcb_generate_id(c);
    xcb_create_gc(c, gc, w, XCB_GC_FOREGROUND, &scr->black_pixel);
    xcb_gcontext_t white = xcb_generate_id(c);
    xcb_create_gc(c, white, w, XCB_GC_FOREGROUND, &scr->white_pixel);

    xcb_rectangle_t rect = {
        .x = 2820,
        .y = 100,
        .width = 100,
        .height = 100};
    xcb_rectangle_t rectbg = {
        .x = 2620,
        .y = 100,
        .width = 500,
        .height = 100};


    xcb_pixmap_t wall = xcb_generate_id(c);
    xcb_create_pixmap(c, 24, wall, w, 1920, 1080);
    xcb_copy_area(c, w, wall, gc, 1080, 0, 0, 0, 1920, 1080);

    xcb_map_window(c, w);

    for (;;) {
        unsigned long start = gettime();
        double coeff = (start % 4000) / 2000.0;
        double delta = sin(M_PI * coeff) * 200;
        rect.x = 2820 + delta;
        xcb_copy_area(c, wall, w, gc, 0, 0, 1080, 0, 1920, 1080);
        xcb_poly_fill_rectangle(c, w, white, 1, &rectbg);
        xcb_poly_fill_rectangle(c, w, gc, 1, &rect);
        xcb_flush(c);
        long elapsed = gettime() - start;
        if (elapsed < 1000/68) {
            long remaining = 1000/68 - elapsed;
            struct timespec rts = {
                .tv_sec = 0,
                .tv_nsec = remaining * 1000000
            };
            nanosleep(&rts, 0);
        }
    }

    xcb_copy_area(c, wall, w, gc, 0, 0, 1080, 0, 1920, 1080);
    xcb_free_pixmap(c, wall);
    /* pause(); */
    xcb_disconnect(c);
}
