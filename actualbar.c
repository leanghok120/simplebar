#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t running = 1;

void handle_sigint(int sig) {
  running = 0;
}

int main() {
  signal(SIGINT, handle_sigint);
  signal(SIGTERM, handle_sigint);

  Display *dpy = XOpenDisplay(NULL);
  if (dpy == NULL) {
    fprintf(stderr, "cant open display lol bozo");
    return 1;
  }
  int scr = DefaultScreen(dpy);
  int width = DisplayWidth(dpy, scr);
  int height = 20;

  // create the bar window
  Window bar = XCreateSimpleWindow(dpy, RootWindow(dpy, scr), 0, 0, width, height, 0, BlackPixel(dpy, scr), WhitePixel(dpy, scr));
  XStoreName(dpy, bar, "actualbar");

  // add text
  Font font = XLoadFont(dpy, "fixed");
  GC gc = XCreateGC(dpy, bar, 0, NULL);
  XSetFont(dpy, gc, font);
  XSetForeground(dpy, gc, BlackPixel(dpy, scr));

  // set bar windows type to dock
  Atom wm_window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
  Atom wm_window_type_dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
  XChangeProperty(dpy, bar, wm_window_type, XA_ATOM, 32, PropModeReplace, (unsigned char *)&wm_window_type_dock, 1);

  // actually show the bar window
  XMapWindow(dpy, bar);
  XFlush(dpy);

  // add the time to the bar window
  char buf[64];
  while (running) {
    time_t now = time(NULL);
    strftime(buf, sizeof(buf), "%a %d %b %Y %I:%M:%S %p", localtime(&now));

    int text_width = XTextWidth(XQueryFont(dpy, font), buf, strlen(buf));
    int text_x = (width - text_width) / 2;

    XClearWindow(dpy, bar);
    XDrawString(dpy, bar, gc, text_x, height - 6, buf, strlen(buf));
    XFlush(dpy);

    sleep(1);
  }

  // clean up
  XUnloadFont(dpy, font);
  XFreeGC(dpy, gc);
  XDestroyWindow(dpy, bar);
  XCloseDisplay(dpy);

  return 0;
}
