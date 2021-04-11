#include <iostream>
#include <cmath>
#include <memory>

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/gl.h>

namespace globals {

int width, height;
bool redisp_pending;
Atom xa_wm_prot, xa_wm_del_win;
int global_x = 0, global_y = 0;
bool to_draw = false;

} // namespace globals;


namespace {
void redraw(Display *dpy, Window win) {
  glClearColor(0.145, 0.521, 0.294, 0.004);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (globals::to_draw) {
    glLoadIdentity();
    XWindowAttributes winattr;
    XGetWindowAttributes(dpy, win, &winattr);
    glViewport(0, 0, winattr.width, winattr.height);

    float x = 2.f * static_cast<float>(globals::global_x) / static_cast<float>(globals::width) - 1.f;
    float y = -(2.f * static_cast<float>(globals::global_y) / static_cast<float>(globals::height) - 1.f);
    float h = 200.f;
    float segment = h * 2.f / std::sqrt(3.f); // len of triangle segment
    float left_x = static_cast<float>(globals::global_x) - segment / 2.f; // left x coordinate
    float right_x = static_cast<float>(globals::global_x) + segment / 2.f; // right x coordinate
    float other_y = static_cast<float>(globals::global_y) + h;

    glColor3f(1.f, 0.f, 0.f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y);
    glVertex2f(2.f * left_x / static_cast<float>(globals::width) - 1.f,
               -(2.f * other_y / static_cast<float>(globals::height) - 1.f));
    glVertex2f(2.f * right_x / static_cast<float>(globals::width) - 1.f,
               -(2.f * other_y / static_cast<float>(globals::height) - 1.f));
    glEnd();
  }
  glXSwapBuffers(dpy, win);
}

Window create_window(Display *dpy, int xsz, int ysz) {
  GLXContext ctx;
  Window w, root;
  XClassHint chint;
  XSetWindowAttributes xattr;
  unsigned int evmask, xattr_mask;
  int scr;
  int glxattr[] = {
    GLX_RGBA, GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_USE_GL, 1,
    None
  };

  scr = DefaultScreen(dpy);
  root = RootWindow(dpy, scr);

  auto deleter_vis = [](XVisualInfo *vis) {
    XFree(vis);
  };
  auto vis = std::unique_ptr<XVisualInfo, decltype(deleter_vis)> {glXChooseVisual(dpy, scr, glxattr), deleter_vis};
  if (!vis) {
    std::cout << "failed to find a suitable visual" << std::endl;
    return 0;
  }

  if (!(ctx = glXCreateContext(dpy, vis.get(), nullptr, true))) {
    return 0;
  }

  xattr.background_pixel = xattr.border_pixel = BlackPixel(dpy, scr);
  xattr.colormap = XCreateColormap(dpy, root, vis->visual, AllocNone);
  xattr_mask = CWColormap | CWBackPixel | CWBorderPixel;

  if (!(w = XCreateWindow(dpy, root, 0, 0, xsz, ysz, 0, vis->depth, InputOutput,
                          vis->visual, xattr_mask, &xattr))) {
    std::cout << "failed to create window" << std::endl;
    glXDestroyContext(dpy, ctx);
    return 0;
  }

  evmask = StructureNotifyMask | VisibilityChangeMask | KeyPressMask |
           ExposureMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask;
  XSelectInput(dpy, w, evmask);

  XSetWMProtocols(dpy, w, &globals::xa_wm_del_win, 1);

  char name[] = {"glx-example"};
  chint.res_name = chint.res_class = name;
  XSetClassHint(dpy, w, &chint);
  const char *title = "TRIANGLE";

  XTextProperty wm_name;
  XStringListToTextProperty(const_cast<char **>(&title), 1, &wm_name);
  XSetWMName(dpy, w, &wm_name);
  XFree(wm_name.value);
  glXMakeCurrent(dpy, w, ctx);
  XMapWindow(dpy, w);
  return w;
}

bool handle_event(XEvent *ev) {
  bool quit = false;
  switch (ev->type) {
    case Expose: {
      globals::redisp_pending = true;
      break;
    }
    case KeyPress: {
      KeySym sym = XLookupKeysym(&ev->xkey, 0);
      switch (sym & 0xff) {
        case 27:
        case 'q':
        case 'Q':
          quit = true;
        default:
          break;
      }
      break;
    }
    case ConfigureNotify: {
      if (ev->xconfigure.width != globals::width || ev->xconfigure.height != globals::height) {
        globals::width = ev->xconfigure.width;
        globals::height = ev->xconfigure.height;
        break;
      }
    }
    case ClientMessage: {
      if (ev->xclient.message_type == globals::xa_wm_prot &&
          ev->xclient.data.l[0] == globals::xa_wm_del_win) {
        quit = true;
      }
      break;
    }
    case ButtonPress: {
      globals::to_draw = true;
      globals::redisp_pending = true;
      globals::global_x = ev->xbutton.x;
      globals::global_y = ev->xbutton.y;
      break;
    }
    case ButtonRelease: {
      globals::to_draw = false;
      globals::redisp_pending = true;
      break;
    }
    case MotionNotify: {
      globals::global_x = ev->xmotion.x;
      globals::global_y = ev->xmotion.y;
      globals::to_draw = true;
      globals::redisp_pending = true;
      break;
    }
  }
  return quit;
}

}

int main() {
  Display *dpy;
  Window win;

  if ((dpy = XOpenDisplay(nullptr)) == nullptr) {
    std::cerr << "failed in open display" << std::endl;
    return 1;
  }

  globals::xa_wm_prot = XInternAtom(dpy, "WM_PROTOCOLS", false);
  globals::xa_wm_del_win = XInternAtom(dpy, "WM_DELETE_WINDOW", false);

  if (!(win = create_window(dpy, 800, 600))) {
    std::cerr << "fail create window" << std::endl;
    return 2;
  }

  while (true) {
    XEvent ev;
    do {
      XNextEvent(dpy, &ev);
      if (handle_event(&ev)) {
        XUnmapWindow(dpy, win);
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
        return 0;
      }
    } while (XPending(dpy));

    if (globals::redisp_pending) {
      globals::redisp_pending = false;
      redraw(dpy, win);
    }
  }
}

