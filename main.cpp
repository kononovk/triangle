#include <iostream>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>

Display *dpy;
Window win;
GLXContext ctx;
int width, height;
bool redisp_pending;
Atom xa_wm_prot, xa_wm_del_win;
/* camera control */
float cam_theta, cam_phi = 25, cam_dist = 6;
int mbutton[8], prev_mx, prev_my;
float ldir[] = {-1, 1, 2, 0};

int global_x = 0;
int global_y = 0;

bool to_draw = false;

void reshape(int x, int y) {
  glViewport(0, 0, x, y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(50.0, (float) x / (float) y, 0.5, 500.0);
}

void redraw() {
  glClearColor(0.145, 0.521, 0.294, 0.004);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (to_draw) {
    glLoadIdentity();
    XWindowAttributes winattr;
    XGetWindowAttributes(dpy, win, &winattr);
    glViewport(0, 0, winattr.width, winattr.height);
    float x = (float)global_x / (float)width - 0.5f;
    double y = global_y;
    glColor3f(1.f, 0.f, 0.f);
    glBegin(GL_TRIANGLES);
    glVertex3f(x, 0.f, 0.f);
    glVertex3f(-1.f, -0.5f, 0.f);
    glVertex3f(1.f, -0.5f, 0.f);
    glEnd();
  }

  glXSwapBuffers(dpy, win);
}


void mouse_button(unsigned int bn, int state, int x, int y) {
  if (state == 1) {
    to_draw = true;
    redisp_pending = true;
    global_x = x;
    global_y = y;
  } else {
    to_draw = false;
    redisp_pending = true;
  }
}

void mouse_motion(int x, int y) {
  global_x = x;
  global_y = y;
}


Window create_window(int xsz, int ysz) {
  Window w, root;
  XVisualInfo *vis;
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

  if (!(vis = glXChooseVisual(dpy, scr, glxattr))) {
    printf("failed to find a suitable visual\n");
    return 0;
  }
  if (!(ctx = glXCreateContext(dpy, vis, nullptr, true)))  {
    XFree(vis);
    return 0;
  }

  xattr.background_pixel = xattr.border_pixel = BlackPixel(dpy, scr);
  xattr.colormap = XCreateColormap(dpy, root, vis->visual, AllocNone);
  xattr_mask = CWColormap | CWBackPixel | CWBorderPixel;

  if (!(w = XCreateWindow(dpy, root, 0, 0, xsz, ysz, 0, vis->depth, InputOutput,
                          vis->visual, xattr_mask, &xattr))) {
    printf("failed to create window\n");
    glXDestroyContext(dpy, ctx);
    XFree(vis);
    return 0;
  }
  XFree(vis);

  evmask = StructureNotifyMask | VisibilityChangeMask | KeyPressMask |
           ExposureMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask;
  XSelectInput(dpy, w, evmask);

  XSetWMProtocols(dpy, w, &xa_wm_del_win, 1);

  chint.res_name = chint.res_class = (char *) "glx-example";
  XSetClassHint(dpy, w, &chint);

  const char *title = "TRIANGLE";

  XTextProperty wm_name;
  XStringListToTextProperty((char **) &title, 1, &wm_name);
  XSetWMName(dpy, w, &wm_name);
  XSetWMIconName(dpy, w, &wm_name);
  XFree(wm_name.value);

  glXMakeCurrent(dpy, w, ctx);
  XMapWindow(dpy, w);
  return w;
}


bool handle_event(XEvent *ev) {
  bool quit = false;
  switch (ev->type) {
    case Expose: {
      redisp_pending = true;
      break;
    }
    case KeyPress: {
      KeySym sym = XLookupKeysym(&ev->xkey, 0);
      switch (sym & 0xff) {
        case 27:
        case 'q':
        case 'Q':
          return true;
        default:
          break;
      }
      break;
    }
    case ConfigureNotify: {
      if (ev->xconfigure.width != width || ev->xconfigure.height != height) {
        width = ev->xconfigure.width;
        height = ev->xconfigure.height;
        reshape(width, height);
        break;
      }
    }
    case ClientMessage: {
      if (ev->xclient.message_type == xa_wm_prot) {
        if (ev->xclient.data.l[0] == xa_wm_del_win) {
          quit = true;
          break;
        }
      }
      break;
    }
    case ButtonPress: {
      std::cout << "Press: " << ev->xbutton.x << ' ' << ev->xbutton.y << std::endl;
      mouse_button(ev->xbutton.button, 1, ev->xbutton.x, ev->xbutton.y);
      break;
    }
    case ButtonRelease: {
      std::cout << "Release: " << ev->xbutton.x << ' ' << ev->xbutton.y << std::endl;
      mouse_button(ev->xbutton.button - Button1, 0, ev->xbutton.x, ev->xbutton.y);
      break;
    }
    case MotionNotify: {
      mouse_motion(ev->xmotion.x, ev->xmotion.y);
      break;
    }
  }
  return quit;
}


int main() {
  if ((dpy = XOpenDisplay(nullptr)) == nullptr) {
    std::cerr << "failed in open display" << std::endl;
    return 1;
  }

  xa_wm_prot = XInternAtom(dpy, "WM_PROTOCOLS", False);
  xa_wm_del_win = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

  if (!(win = create_window(800, 600))) {
    std::cerr << "fail create window" << std::endl;
    return 2;
  }

  while (true) {
    XEvent ev;
    do {
      XNextEvent(dpy, &ev);
      if (handle_event(&ev)) {
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
        return 0;
      }
    } while (XPending(dpy));

    if (redisp_pending) {
      redisp_pending = false;
      redraw();
    }
  }
}

