/*
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Ported to GLES2.
 * Kristian Høgsberg <krh@bitplanet.net>
 * May 3, 2010
 *
 * Improve GLES2 port:
 *   * Refactor gear drawing.
 *   * Use correct normals for surfaces.
 *   * Improve shader.
 *   * Use perspective projection transformation.
 *   * Add FPS count.
 *   * Add comments.
 * Alexandros Frantzis <alexandros.frantzis@linaro.org>
 * Jul 13, 2010
 */

/*
 * modified for bealeboard xm test
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include <epoxy/gl.h>
#include <epoxy/egl.h>

#define MYWINDOW 0

#define STRIPS_PER_TOOTH 7
#define VERTICES_PER_TOOTH 34
#define GEAR_VERTEX_STRIDE 6

/**
 * Struct describing the vertices in triangle strip
 */
struct vertex_strip_t
{
    /** The first vertex in the strip */
    GLint first;
    /** The number of consecutive vertices in the strip after the first */
    GLint count;
};

/* Each vertex consist of GEAR_VERTEX_STRIDE GLfloat attributes */
typedef GLfloat GearVertex[GEAR_VERTEX_STRIDE];

/**
 * Struct representing a gear.
 */
struct gear_t
{
    /** The array of vertices comprising the gear */
    GearVertex *vertices;
    /** The number of vertices comprising the gear */
    int nvertices;
    /** The array of triangle strips comprising the gear */
    struct vertex_strip_t *strips;
    /** The number of triangle strips comprising the gear */
    int nstrips;
    /** The Vertex Buffer Object holding the vertices in the graphics card */
    GLuint vbo;
};

struct point_t
{
    GLfloat x;
    GLfloat y;
};

static int g_start_time;

/** The view rotation [x, y, z] */
static GLfloat g_view_rot[3] = { 20.0, 30.0, 0.0 };
/** The gears */
static struct gear_t *g_gear1;
static struct gear_t *g_gear2;
static struct gear_t *g_gear3;
/** The current gear rotation angle */
static GLfloat g_angle = 0.0;
/** The location of the shader uniforms */
static GLuint g_ModelViewProjectionMatrix_location;
static GLuint g_NormalMatrix_location;
static GLuint g_LightSourcePosition_location;
static GLuint g_MaterialColor_location;
/** The projection matrix */
static GLfloat g_ProjectionMatrix[16];
/** The direction of the directional light for the scene */
static const GLfloat g_LightSourcePosition[4] = { 5.0, 5.0, 10.0, 1.0};

/**
 * Fills a gear vertex.
 *
 * @param v the vertex to fill
 * @param x the x coordinate
 * @param y the y coordinate
 * @param z the z coortinate
 * @param n pointer to the normal table
 *
 * @return the operation error code
 */
static GearVertex *
vert(GearVertex *v, GLfloat x, GLfloat y, GLfloat z, GLfloat n[3])
{
    v[0][0] = x;
    v[0][1] = y;
    v[0][2] = z;
    v[0][3] = n[0];
    v[0][4] = n[1];
    v[0][5] = n[2];
    return v + 1;
}

/* A set of macros for making the creation of the gears easier */
#define GEAR_POINT(i, r, da) do { \
    p[i].x = (r) * c[(da)]; \
    p[i].y = (r) * s[(da)]; \
} while (0)
#define SET_NORMAL(x, y, z) do { \
    normal[0] = (x); normal[1] = (y); normal[2] = (z); \
} while (0)

#define GEAR_VERT(v, point, sign) \
    vert((v), p[(point)].x, p[(point)].y, (sign) * width * 0.5, normal)

#define START_STRIP do { \
    gear->strips[cur_strip].first = v - gear->vertices; \
} while (0);

#define END_STRIP do { \
    int _tmp = (v - gear->vertices); \
    gear->strips[cur_strip].count = _tmp - gear->strips[cur_strip].first; \
    cur_strip++; \
} while (0)

#define QUAD_WITH_NORMAL(p1, p2) do { \
    SET_NORMAL((p[(p1)].y - p[(p2)].y), -(p[(p1)].x - p[(p2)].x), 0); \
    v = GEAR_VERT(v, (p1), -1); \
    v = GEAR_VERT(v, (p1), 1); \
    v = GEAR_VERT(v, (p2), -1); \
    v = GEAR_VERT(v, (p2), 1); \
} while (0)

/**
 *  Create a gear wheel.
 *
 *  @param inner_radius radius of hole at center
 *  @param outer_radius radius at center of teeth
 *  @param width width of gear
 *  @param teeth number of teeth
 *  @param tooth_depth depth of tooth
 *
 *  @return pointer to the constructed struct gear
 */
static struct gear_t *
create_gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
            GLint teeth, GLfloat tooth_depth)
{
    GLfloat r0;
    GLfloat r1;
    GLfloat r2;
    GLfloat da;
    GearVertex *v;
    struct gear_t *gear;
    double s[5];
    double c[5];
    GLfloat normal[3];
    int cur_strip = 0;
    int i;
    struct point_t p[7];

    /* Allocate memory for the gear */
    gear = malloc(sizeof(struct gear_t));

    /* Calculate the radii used in the gear */
    r0 = inner_radius;
    r1 = outer_radius - tooth_depth / 2.0;
    r2 = outer_radius + tooth_depth / 2.0;

    da = 2.0 * M_PI / teeth / 4.0;

    /* Allocate memory for the triangle strip information */
    gear->nstrips = STRIPS_PER_TOOTH * teeth;
    gear->strips = calloc(gear->nstrips, sizeof(*gear->strips));

    /* Allocate memory for the vertices */
    gear->vertices = calloc(VERTICES_PER_TOOTH * teeth,
                            sizeof(*gear->vertices));
    v = gear->vertices;

    for (i = 0; i < teeth; i++)
    {
        /* Calculate needed sin/cos for varius angles */
        sincos(i * 2.0 * M_PI / teeth, &s[0], &c[0]);
        sincos(i * 2.0 * M_PI / teeth + da, &s[1], &c[1]);
        sincos(i * 2.0 * M_PI / teeth + da * 2, &s[2], &c[2]);
        sincos(i * 2.0 * M_PI / teeth + da * 3, &s[3], &c[3]);
        sincos(i * 2.0 * M_PI / teeth + da * 4, &s[4], &c[4]);

        /* Create the 7 points (only x,y coords) used to draw a tooth */
        GEAR_POINT(0, r2, 1);
        GEAR_POINT(1, r2, 2);
        GEAR_POINT(2, r1, 0);
        GEAR_POINT(3, r1, 3);
        GEAR_POINT(4, r0, 0);
        GEAR_POINT(5, r1, 4);
        GEAR_POINT(6, r0, 4);

        /* Front face */
        START_STRIP;
        SET_NORMAL(0, 0, 1.0);
        v = GEAR_VERT(v, 0, +1);
        v = GEAR_VERT(v, 1, +1);
        v = GEAR_VERT(v, 2, +1);
        v = GEAR_VERT(v, 3, +1);
        v = GEAR_VERT(v, 4, +1);
        v = GEAR_VERT(v, 5, +1);
        v = GEAR_VERT(v, 6, +1);
        END_STRIP;

        /* Inner face */
        START_STRIP;
        QUAD_WITH_NORMAL(4, 6);
        END_STRIP;

        /* Back face */
        START_STRIP;
        SET_NORMAL(0, 0, -1.0);
        v = GEAR_VERT(v, 6, -1);
        v = GEAR_VERT(v, 5, -1);
        v = GEAR_VERT(v, 4, -1);
        v = GEAR_VERT(v, 3, -1);
        v = GEAR_VERT(v, 2, -1);
        v = GEAR_VERT(v, 1, -1);
        v = GEAR_VERT(v, 0, -1);
        END_STRIP;

        /* Outer face */
        START_STRIP;
        QUAD_WITH_NORMAL(0, 2);
        END_STRIP;

        START_STRIP;
        QUAD_WITH_NORMAL(1, 0);
        END_STRIP;

        START_STRIP;
        QUAD_WITH_NORMAL(3, 1);
        END_STRIP;

        START_STRIP;
        QUAD_WITH_NORMAL(5, 3);
        END_STRIP;
    }

    gear->nvertices = (v - gear->vertices);

    /* Store the vertices in a vertex buffer object (VBO) */
    glGenBuffers(1, &gear->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);
    glBufferData(GL_ARRAY_BUFFER, gear->nvertices * sizeof(GearVertex),
                 gear->vertices, GL_STATIC_DRAW);
    return gear;
}

/**
 * Multiplies two 4x4 matrices.
 *
 * The result is stored in matrix m.
 *
 * @param m the first matrix to multiply
 * @param n the second matrix to multiply
 */
static void
multiply(GLfloat *m, const GLfloat *n)
{
    GLfloat tmp[16];
    const GLfloat *row;
    const GLfloat *column;
    div_t d;
    int i;
    int j;

    for (i = 0; i < 16; i++)
    {
        tmp[i] = 0;
        d = div(i, 4);
        row = n + d.quot * 4;
        column = m + d.rem;
        for (j = 0; j < 4; j++)
        {
            tmp[i] += row[j] * column[j * 4];
        }
    }
    memcpy(m, &tmp, sizeof(tmp));
}

/**
 * Rotates a 4x4 matrix.
 *
 * @param[in,out] m the matrix to rotate
 * @param angle the angle to rotate
 * @param x the x component of the direction to rotate to
 * @param y the y component of the direction to rotate to
 * @param z the z component of the direction to rotate to
 */
static void
rotate(GLfloat *m, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    double s;
    double c;
    GLfloat r[16];

    sincos(angle, &s, &c);
    r[0] = x * x * (1 - c) + c;
    r[1] = y * x * (1 - c) + z * s;
    r[2] = x * z * (1 - c) - y * s;
    r[3] = 0;
    r[4] = x * y * (1 - c) - z * s;
    r[5] = y * y * (1 - c) + c;
    r[6] = y * z * (1 - c) + x * s;
    r[7] = 0;
    r[8] = x * z * (1 - c) + y * s;
    r[9] = y * z * (1 - c) - x * s;
    r[10] = z * z * (1 - c) + c;
    r[11] = 0;
    r[12] = 0;
    r[13] = 0;
    r[14] = 0;
    r[15] = 1;
    multiply(m, r);
}

/**
 * Creates an identity 4x4 matrix.
 *
 * @param m the matrix make an identity matrix
 */
static void
identity(GLfloat *m)
{
    GLfloat t[16] =
    {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    };
    memcpy(m, t, sizeof(t));
}

/**
 * Translates a 4x4 matrix.
 *
 * @param[in,out] m the matrix to translate
 * @param x the x component of the direction to translate to
 * @param y the y component of the direction to translate to
 * @param z the z component of the direction to translate to
 */
static void
translate(GLfloat *m, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat t[16];

    identity(t);
    t[12] = x;
    t[13] = y;
    t[14] = z;
    multiply(m, t);
}

/**
 * Transposes a 4x4 matrix.
 *
 * @param m the matrix to transpose
 */
static void
transpose(GLfloat *m)
{
    GLfloat t[16];

    t[0] = m[0];
    t[1] = m[4];
    t[2] = m[8];
    t[3] = m[12];
    t[4] = m[1];
    t[5] = m[5];
    t[6] = m[9];
    t[7] = m[13];
    t[8] = m[2];
    t[9] = m[6];
    t[10] = m[10];
    t[11] = m[14];
    t[12] = m[3];
    t[13] = m[7];
    t[14] = m[11];
    t[15] = m[15];
    memcpy(m, t, sizeof(t));
}

/**
 * Inverts a 4x4 matrix.
 *
 * This function can currently handle only pure translation-rotation matrices.
 * Read http://www.gamedev.net/community/forums/topic.asp?topic_id=425118
 * for an explanation.
 */
static void
invert(GLfloat *m)
{
    GLfloat t[16];

    identity(t);
    /* Extract and invert the translation part 't'. The inverse of a
       translation matrix can be calculated by negating the translation
       coordinates. */
    t[12] = -m[12];
    t[13] = -m[13];
    t[14] = -m[14];
    /* Invert the rotation part 'r'. The inverse of a rotation matrix is
       equal to its transpose. */
    m[12] = m[13] = m[14] = 0;
    transpose(m);
    /* inv(m) = inv(r) * inv(t) */
    multiply(m, t);
}

/**
 * Calculate a perspective projection transformation.
 *
 * @param m the matrix to save the transformation in
 * @param fovy the field of view in the y direction
 * @param aspect the view aspect ratio
 * @param zNear the near clipping plane
 * @param zFar the far clipping plane
 */
static void
perspective(GLfloat *m, GLfloat fovy, GLfloat aspect, GLfloat zNear,
            GLfloat zFar)
{
    GLfloat tmp[16];
    double sine;
    double cosine;
    double cotangent;
    double deltaZ;
    GLfloat radians;

    identity(tmp);
    radians = fovy / 2 * M_PI / 180;
    deltaZ = zFar - zNear;
    sincos(radians, &sine, &cosine);
    if ((deltaZ == 0) || (sine == 0) || (aspect == 0))
    {
        return;
    }
    cotangent = cosine / sine;
    tmp[0] = cotangent / aspect;
    tmp[5] = cotangent;
    tmp[10] = -(zFar + zNear) / deltaZ;
    tmp[11] = -1;
    tmp[14] = -2 * zNear * zFar / deltaZ;
    tmp[15] = 0;
    memcpy(m, tmp, sizeof(tmp));
}

/**
 * Draws a gear.
 *
 * @param gear the gear to draw
 * @param transform the current transformation matrix
 * @param x the x position to draw the gear at
 * @param y the y position to draw the gear at
 * @param angle the rotation angle of the gear
 * @param color the color of the gear
 */
static void
draw_gear(struct gear_t *gear, GLfloat *transform,
          GLfloat x, GLfloat y, GLfloat angle, const GLfloat color[4])
{
    GLfloat model_view[16];
    GLfloat normal_matrix[16];
    GLfloat model_view_projection[16];
    int n;

    /* Translate and rotate the gear */
    memcpy(model_view, transform, sizeof(model_view));
    translate(model_view, x, y, 0);
    rotate(model_view, 2 * M_PI * angle / 360.0, 0, 0, 1);

    /* Create and set the ModelViewProjectionMatrix */
    memcpy(model_view_projection, g_ProjectionMatrix,
           sizeof(model_view_projection));
    multiply(model_view_projection, model_view);

    glUniformMatrix4fv(g_ModelViewProjectionMatrix_location, 1, GL_FALSE,
                       model_view_projection);

    /*
     * Create and set the NormalMatrix. It's the inverse transpose of the
     * ModelView matrix.
     */
    memcpy(normal_matrix, model_view, sizeof (normal_matrix));
    invert(normal_matrix);
    transpose(normal_matrix);
    glUniformMatrix4fv(g_NormalMatrix_location, 1, GL_FALSE, normal_matrix);

    /* Set the gear color */
    glUniform4fv(g_MaterialColor_location, 1, color);

    /* Set the vertex buffer object to use */
    glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);

    /* Set up the position of the attributes in the vertex buffer object */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat), NULL);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          6 * sizeof(GLfloat), (GLfloat *) 0 + 3);

    /* Enable the attributes */
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    /* Draw the triangle strips that comprise the gear */
    for (n = 0; n < gear->nstrips; n++)
    {
        glDrawArrays(GL_TRIANGLE_STRIP, gear->strips[n].first,
                     gear->strips[n].count);
    }

    /* Disable the attributes */
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

/**
 * Draws the gears.
 */
static void
gears_draw(void)
{
    const static GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
    const static GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
    const static GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };
    GLfloat transform[16];

    identity(transform);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /* Translate and rotate the view */
    translate(transform, 0, 0, -20);
    rotate(transform, 2 * M_PI * g_view_rot[0] / 360.0, 1, 0, 0);
    rotate(transform, 2 * M_PI * g_view_rot[1] / 360.0, 0, 1, 0);
    rotate(transform, 2 * M_PI * g_view_rot[2] / 360.0, 0, 0, 1);
    /* Draw the gears */
    draw_gear(g_gear1, transform, -3.0, -2.0, g_angle, red);
    draw_gear(g_gear2, transform, 3.1, -2.0, -2 * g_angle - 9.0, green);
    draw_gear(g_gear3, transform, -3.1, 4.2, -2 * g_angle - 25.0, blue);
}

static int
now(void)
{
    struct timeval tv;
    struct timezone tz;

    (void) gettimeofday(&tv, &tz);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/**
 * Handles a new window size or exposure.
 *
 * @param width the window width
 * @param height the window height
 */
static void
gears_reshape(int width, int height)
{
    /* Update the projection matrix */
    perspective(g_ProjectionMatrix, 60.0, width / (float)height, 1.0, 1024.0);
    /* Set the viewport */
    glViewport(0, 0, (GLint) width, (GLint) height);
}

static void
gears_idle(void)
{
    static int frames = 0;
    static double tRot0 = -1.0;
    static double tRate0 = -1.0;
    double dt;
    double t;
    GLfloat seconds;
    GLfloat fps;

    t = (now() - g_start_time)  / 1000.0;
    if (tRot0 < 0.0)
    {
        tRot0 = t;
    }
    dt = t - tRot0;
    tRot0 = t;
    /* advance rotation for next frame */
    g_angle += 70.0 * dt; /* 70 degrees per second */
    if (g_angle > 3600.0)
    {
        g_angle -= 3600.0;
    }
    frames++;
    if (tRate0 < 0.0)
    {
        tRate0 = t;
    }
    if (t - tRate0 >= 5.0)
    {
        seconds = t - tRate0;
        fps = frames / seconds;
        printf("%d frames in %3.1f seconds = %6.3f FPS\n",
               frames, seconds, fps);
        tRate0 = t;
        frames = 0;
    }
}

static const char vertex_shader[] =
"attribute vec3 position;\n"
"attribute vec3 normal;\n"
"\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"uniform mat4 NormalMatrix;\n"
"uniform vec4 LightSourcePosition;\n"
"uniform vec4 MaterialColor;\n"
"\n"
"varying vec4 Color;\n"
"\n"
"void main(void)\n"
"{\n"
"    /* Transform the normal to eye coordinates */\n"
"    vec3 N = normalize(vec3(NormalMatrix * vec4(normal, 1.0)));\n"
"\n"
"    /* The LightSourcePosition is actually its direction for directional\n"
"       light */\n"
"    vec3 L = normalize(LightSourcePosition.xyz);\n"
"\n"
"    /* Multiply the diffuse value by the vertex color (which is fixed in \n"
"       this case)\n"
"       to get the actual color that we will use to draw this vertex with */\n"
"    float diffuse = max(dot(N, L), 0.0);\n"
"    Color = diffuse * MaterialColor;\n"
"\n"
"    /* Transform the position to clip coordinates */\n"
"    gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);\n"
"}";

static const char fragment_shader[] =
"precision mediump float;\n"
"varying vec4 Color;\n"
"\n"
"void main(void)\n"
"{\n"
"    gl_FragColor = Color;\n"
"}";

static void
gears_init(void)
{
    GLuint v;
    GLuint f;
    GLuint program;
    GLint linked;
    GLint compiled;
    const char *p;

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    /* Compile the vertex shader */
    p = vertex_shader;
    v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &p, NULL);
    glCompileShader(v);
    glGetShaderiv(v, GL_COMPILE_STATUS, &compiled);
    printf("vertex shader compiled %d\n", compiled);

    /* Compile the fragment shader */
    p = fragment_shader;
    f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &p, NULL);
    glCompileShader(f);
    glGetShaderiv(f, GL_COMPILE_STATUS, &compiled);
    printf("fragment shader compiled %d\n", compiled);

    /* Create and link the shader program */
    program = glCreateProgram();
    glAttachShader(program, v);
    glAttachShader(program, f);
    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "normal");

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    printf("program linked %d\n", linked);

    /* Enable the shaders */
    glUseProgram(program);

    /* Get the locations of the uniforms so we can access them */
    g_ModelViewProjectionMatrix_location =
        glGetUniformLocation(program, "ModelViewProjectionMatrix");
    g_NormalMatrix_location =
        glGetUniformLocation(program, "NormalMatrix");
    g_LightSourcePosition_location =
        glGetUniformLocation(program, "LightSourcePosition");
    g_MaterialColor_location =
        glGetUniformLocation(program, "MaterialColor");

    /* Set the LightSourcePosition uniform which is constant throught
     * the program */
    glUniform4fv(g_LightSourcePosition_location, 1, g_LightSourcePosition);

    /* make the gears */
    g_gear1 = create_gear(1.0, 4.0, 1.0, 20, 0.7);
    g_gear2 = create_gear(0.5, 2.0, 2.0, 10, 0.7);
    g_gear3 = create_gear(1.3, 2.0, 0.5, 10, 0.7);
}

int
main(int argc, char** argv)
{
    EGLint config_attribs[] = {
        EGL_BUFFER_SIZE,                EGL_DONT_CARE,
        EGL_RED_SIZE,                   8,
        EGL_GREEN_SIZE,                 8,
        EGL_BLUE_SIZE,                  8,
        EGL_DEPTH_SIZE,                 8,
        EGL_SURFACE_TYPE,               EGL_PIXMAP_BIT | EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE,            EGL_OPENGL_ES2_BIT,
        EGL_NONE };
    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION,     2,
        EGL_NONE };
    EGLConfig cfg[4];
    EGLint count;
    int major;
    int minor;
    EGLSurface surface;
    EGLContext ctx;
    EGLDisplay* edpy;
    EGLint width;
    EGLint height;

    if (!eglBindAPI(EGL_OPENGL_ES_API))
    {
        printf("error\n");
        return 1;
    }
    edpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (edpy == EGL_NO_DISPLAY)
    {
        printf("Failed to get an EGLDisplay\n");
        return 1;
    }
    if (eglInitialize(edpy, &major, &minor) == EGL_FALSE)
    {
        printf("eglInitialize failed\n");
        return 1;
    }
    printf("elgInitialize ok major %d minor %d\n", major, minor);
    memset(cfg, 0, sizeof(cfg));
    if (eglChooseConfig(edpy, config_attribs, cfg, 4, &count) == EGL_FALSE)
    {
        printf("Couldn't get an EGLConfig\n");
        return 1;
    }
    if (count < 1)
    {
        printf("Couldn't get an EGLConfig\n");
        return 1;
    }
    printf("eglChooseConfig ok count %d\n", count);
    ctx = eglCreateContext(edpy, cfg[0], EGL_NO_CONTEXT, context_attribs);
    if (ctx == EGL_NO_CONTEXT)
    {
        printf("Couldn't create a GL context\n");
        return 1;
    }
    printf("eglCreateContext ok\n");
    surface = eglCreateWindowSurface(edpy, cfg[0], MYWINDOW, NULL);
    if (surface == EGL_NO_SURFACE)
    {
        printf("eglCreateWindowSurface failed\n");
        return 1;
    }
    printf("eglCreateWindowSurface ok\n");
    if (eglMakeCurrent(edpy, surface, surface, ctx) == EGL_FALSE)
    {
        printf("eglMakeCurrent failed\n");
        return 1;
    }
    printf("eglMakeCurrent ok\n");
    printf("GL_VENDOR     - %s\n"
           "GL_VERSION    - %s\n"
           "GL_EXTENSIONS - %s\n", glGetString(GL_VENDOR),
           glGetString(GL_VERSION), glGetString(GL_EXTENSIONS));
    printf("epoxy gl verstion %d\n", epoxy_gl_version());

    eglQuerySurface(edpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(edpy, surface, EGL_HEIGHT, &height);
    printf("width %d height %d\n", width, height);

    gears_init();
    gears_reshape(width, height);
    g_start_time = now();
    for (;;)
    {
        gears_draw();
        eglSwapBuffers(edpy, surface);
        gears_idle();
#if 0
        view_rot[0] = fmod(view_rot[0] + 0.5, 360.0);
        view_rot[1] = fmod(view_rot[1] + 0.7, 360.0);
        view_rot[2] = fmod(view_rot[2] + 0.9, 360.0);
#endif
    }
    eglTerminate(edpy);
    return 0;
}
