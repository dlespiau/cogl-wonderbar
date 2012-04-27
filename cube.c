#define COGL_ENABLE_EXPERIMENTAL_2_0_API
#define CLUTTER_ENABLE_EXPERIMENTAL_API

#include <string.h>
#include <stdlib.h>

#include <cogl/cogl.h>
#include <clutter/clutter.h>

typedef struct
{
  ClutterActor *stage;
  ClutterTimeline *timeline;

  CoglContext *context;
  CoglIndices *indices;
  CoglPrimitive *prim;
  CoglPipeline *pipeline;

} Cube;

/* A cube modelled using 4 vertices for each face.
 *
 * We use an index buffer when drawing the cube later so the GPU will
 * actually read each face as 2 separate triangles.
 */
static CoglVertexP3T2 vertices[] =
{
  /* Front face */
  { /* pos = */ -1.0f, -1.0f,  1.0f, /* tex coords = */ 0.0f, 1.0f},
  { /* pos = */  1.0f, -1.0f,  1.0f, /* tex coords = */ 1.0f, 1.0f},
  { /* pos = */  1.0f,  1.0f,  1.0f, /* tex coords = */ 1.0f, 0.0f},
  { /* pos = */ -1.0f,  1.0f,  1.0f, /* tex coords = */ 0.0f, 0.0f},

  /* Back face */
  { /* pos = */ -1.0f, -1.0f, -1.0f, /* tex coords = */ 1.0f, 0.0f},
  { /* pos = */ -1.0f,  1.0f, -1.0f, /* tex coords = */ 1.0f, 1.0f},
  { /* pos = */  1.0f,  1.0f, -1.0f, /* tex coords = */ 0.0f, 1.0f},
  { /* pos = */  1.0f, -1.0f, -1.0f, /* tex coords = */ 0.0f, 0.0f},

  /* Top face */
  { /* pos = */ -1.0f,  1.0f, -1.0f, /* tex coords = */ 0.0f, 1.0f},
  { /* pos = */ -1.0f,  1.0f,  1.0f, /* tex coords = */ 0.0f, 0.0f},
  { /* pos = */  1.0f,  1.0f,  1.0f, /* tex coords = */ 1.0f, 0.0f},
  { /* pos = */  1.0f,  1.0f, -1.0f, /* tex coords = */ 1.0f, 1.0f},

  /* Bottom face */
  { /* pos = */ -1.0f, -1.0f, -1.0f, /* tex coords = */ 1.0f, 1.0f},
  { /* pos = */  1.0f, -1.0f, -1.0f, /* tex coords = */ 0.0f, 1.0f},
  { /* pos = */  1.0f, -1.0f,  1.0f, /* tex coords = */ 0.0f, 0.0f},
  { /* pos = */ -1.0f, -1.0f,  1.0f, /* tex coords = */ 1.0f, 0.0f},

  /* Right face */
  { /* pos = */ 1.0f, -1.0f, -1.0f, /* tex coords = */ 1.0f, 0.0f},
  { /* pos = */ 1.0f,  1.0f, -1.0f, /* tex coords = */ 1.0f, 1.0f},
  { /* pos = */ 1.0f,  1.0f,  1.0f, /* tex coords = */ 0.0f, 1.0f},
  { /* pos = */ 1.0f, -1.0f,  1.0f, /* tex coords = */ 0.0f, 0.0f},

  /* Left face */
  { /* pos = */ -1.0f, -1.0f, -1.0f, /* tex coords = */ 0.0f, 0.0f},
  { /* pos = */ -1.0f, -1.0f,  1.0f, /* tex coords = */ 1.0f, 0.0f},
  { /* pos = */ -1.0f,  1.0f,  1.0f, /* tex coords = */ 1.0f, 1.0f},
  { /* pos = */ -1.0f,  1.0f, -1.0f, /* tex coords = */ 0.0f, 1.0f}
};

static void
paint (ClutterActor *stage,
       gpointer      data)
{
  Cube *cube = data;
  CoglFramebuffer *fb;
  int width;
  int height;
  double progress;

  fb = cogl_get_draw_framebuffer ();
  width = cogl_framebuffer_get_width (fb);
  height = cogl_framebuffer_get_height (fb);

  cogl_framebuffer_clear4f (fb,
                            COGL_BUFFER_BIT_COLOR|COGL_BUFFER_BIT_DEPTH,
                            0, 0, 0, 1);

  cogl_framebuffer_push_matrix (fb);
  cogl_framebuffer_translate (fb, width / 2, height / 2, 0);

  cogl_framebuffer_scale (fb, 75, 75, 75);


  progress = clutter_timeline_get_progress (cube->timeline);
  cogl_framebuffer_rotate (fb, progress * 360, 0, 0, 1);
  cogl_framebuffer_rotate (fb, progress * 360, 0, 1, 0);

  cogl_framebuffer_draw_primitive (fb, cube->pipeline, cube->prim);

  cogl_framebuffer_pop_matrix (fb);
}

static void
on_new_frame (ClutterTimeline *timeline,
              gint             msecs,
              gpointer         data)
{
  Cube *cube = data;

  clutter_actor_queue_redraw (cube->stage);
}

int
main (int argc, char **argv)
{
  ClutterBackend *backend;
  CoglContext *ctx;
  CoglDepthState depth_state;
  Cube cube;

  if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
    {
      g_print ("Could not initialize clutter");
      return EXIT_FAILURE;
    }

  memset (&cube, 0, sizeof(Cube));

  cube.stage = clutter_stage_new ();
  g_signal_connect_after (cube.stage, "paint", G_CALLBACK (paint), &cube);

  cube.timeline = clutter_timeline_new (5000);
  g_signal_connect (cube.timeline, "new-frame",
                    G_CALLBACK (on_new_frame), &cube);

  /*
   * Setup CoglObjects to render our cube
   */

  backend = clutter_get_default_backend ();
  cube.context = ctx = clutter_backend_get_cogl_context (backend);

  cube.indices = cogl_get_rectangle_indices (ctx, 6 /* n_rectangles */);
  cube.prim = cogl_primitive_new_p3t2 (ctx, COGL_VERTICES_MODE_TRIANGLES,
                                       G_N_ELEMENTS (vertices),
                                       vertices);
  cogl_primitive_set_indices (cube.prim,
                              cube.indices,
                              6 * 6);

  cube.pipeline = cogl_pipeline_new (ctx);
  //cogl_pipeline_set_layer_texture (cube.pipeline, 0, cube.texture);
  cogl_pipeline_set_color4f (cube.pipeline, 1.0f, 0.f, 0.f, 1.f);

  /* Since the box is made of multiple triangles that will overlap
   * when drawn and we don't control the order they are drawn in, we
   * enable depth testing to make sure that triangles that shouldn't
   * be visible get culled by the GPU. */
  cogl_depth_state_init (&depth_state);
  cogl_depth_state_set_test_enabled (&depth_state, TRUE);

  cogl_pipeline_set_depth_state (cube.pipeline, &depth_state, NULL);

  clutter_actor_show_all (cube.stage);
  clutter_timeline_start (cube.timeline);

  clutter_main ();

  return EXIT_SUCCESS;
}
