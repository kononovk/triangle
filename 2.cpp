
#include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

int main(int argc, char *argv[]) {
  Display *dpy;
  int screen;
  Window win;
  Window Child;
  XEvent event;

  /* For Buttons */
  int x, y;
  unsigned int h, w;
/* x, y. H = Height. W = Width */
  Window root_win;
  Colormap colormap;
  XColor button_color;
  XColor lightgray_color, darkgray_color;
  XGCValues gcv_lightgray, gcv_darkgray;
  GC gc_lightgray, gc_darkgray;

  unsigned int border_width, depth;


  dpy = XOpenDisplay(nullptr);

  if (dpy == nullptr) {
    std::cerr << "Cant open display" << std::endl;
    return 1;
  }


  screen = DefaultScreen(dpy);

  win = XCreateSimpleWindow(dpy, RootWindow(dpy, screen),
                            600, 600, 500, 300,
                            1, BlackPixel(dpy, screen), WhitePixel(dpy, screen));

  XSelectInput(dpy, win, ExposureMask | KeyPressMask);

/* Required To Display Window */
  XMapWindow(dpy, win);

/* Child window and Button */
  colormap = DefaultColormap(dpy, screen);
  XParseColor(dpy, colormap, "rgb:cc/cc/cc", &button_color);
  XAllocColor(dpy, colormap, &button_color);

  XParseColor(dpy, colormap, "rgb:ee/ee/ee", &lightgray_color);
  XAllocColor(dpy, colormap, &lightgray_color);
  gcv_lightgray.foreground = lightgray_color.pixel;
  gcv_lightgray.background = button_color.pixel;
  gc_lightgray = XCreateGC(dpy, RootWindow(dpy, screen),
                           GCForeground | GCBackground, &gcv_lightgray);

  XParseColor(dpy, colormap, "rgb:88/88/88", &darkgray_color);
  XAllocColor(dpy, colormap, &darkgray_color);
  gcv_darkgray.foreground = darkgray_color.pixel;
  gcv_darkgray.background = button_color.pixel;
  gc_darkgray = XCreateGC(dpy, RootWindow(dpy, screen),
                          GCForeground | GCBackground, &gcv_darkgray);


  Child = XCreateSimpleWindow(dpy, win,
                              20, 20, 200, 100,
                              1, BlackPixel(dpy, screen), button_color.pixel);

  XSelectInput(dpy, Child, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask);

  XMapWindow(dpy, Child);

/* Get Gemometry of Window */
  XGetGeometry(dpy, Child, &root_win, &x, &y, &w, &h, &border_width, &depth);

  while (true) {
    XNextEvent(dpy, &event);
    if (event.xany.window == Child) {
      if (event.type == Expose) {
        XDrawLine(dpy, Child, gc_lightgray, 0, 0, w - 1, 0);
        XDrawLine(dpy, Child, gc_lightgray, 0, 0, 0, h - 1);
        XDrawLine(dpy, Child, gc_darkgray, w - 1, 0, w - 1, h - 1);
        XDrawLine(dpy, Child, gc_darkgray, 0, h - 1, w - 1, h - 1);
      }
      if (event.type == ButtonPress) {
        if (event.xbutton.button == 1) {
          XDrawLine(dpy, Child, gc_darkgray, 0, 0, w - 1, 0);
          XDrawLine(dpy, Child, gc_darkgray, 0, 0, 0, h - 1);
          XDrawLine(dpy, Child, gc_lightgray, w - 1, 0, w - 1, h - 1);
          XDrawLine(dpy, Child, gc_lightgray, 0, h - 1, w - 1, h - 1);
          printf("Button Pressed\n");
        }
      }
      if (event.type == ButtonRelease) {
        if (event.xbutton.button == 1) {
          XDrawLine(dpy, Child, gc_lightgray, 0, 0, w - 1, 0);
          XDrawLine(dpy, Child, gc_lightgray, 0, 0, 0, h - 1);
          XDrawLine(dpy, Child, gc_darkgray, w - 1, 0, w - 1, h - 1);
          XDrawLine(dpy, Child, gc_darkgray, 0, h - 1, w - 1, h - 1);
          printf("Button Release\n");
        }
      }
    }
    if (event.type == KeyPress) {
      char str[25] = {0};
      KeySym keysym = 0;
      XLookupString(&event.xkey, str, 25, &keysym, nullptr);
      if (keysym == XK_Escape) {
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
        break;
      }
    }
  }


  return 0;
}