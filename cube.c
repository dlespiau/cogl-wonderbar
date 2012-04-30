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
  CoglPrimitive *prim;
  CoglPipeline *pipeline;

} Cube;

typedef struct
{
  float x, y, z;        /* position       */
  float n_x, n_y, n_z;  /* normal         */
} Vertex;

/*
 *        f +--------+ e
 *         /        /|
 *        /        / |
 *    b  /      a /  |
 *      +--------+   |
 *      |  g     |   + h
 *      |        |  /
 *      |        | /
 *    c |        |/
 *      +--------+ d
 */

#define pos_a        1.0f,  1.0f, 1.0f
#define pos_b       -1.0f,  1.0f, 1.0f
#define pos_c       -1.0f, -1.0f, 1.0f
#define pos_d        1.0f, -1.0f, 1.0f

#define pos_e        1.0f,  1.0f, -1.0f
#define pos_f       -1.0f,  1.0f, -1.0f
#define pos_g       -1.0f, -1.0f, -1.0f
#define pos_h        1.0f, -1.0f, -1.0f

#define norm_front   0.0f,  0.0f,  1.0f
#define norm_right   1.0f,  0.0f,  0.0f
#define norm_back    0.0f,  0.0f, -1.0f
#define norm_left   -1.0f,  0.0f,  0.0f
#define norm_top     0.0f,  1.0f,  0.0f
#define norm_bottom  0.0f, -1.0f,  0.0f

static Vertex vertices[] =
{
  { pos_a, norm_front },
  { pos_b, norm_front },
  { pos_c, norm_front },
  { pos_c, norm_front },
  { pos_d, norm_front },
  { pos_a, norm_front },

  { pos_e, norm_right },
  { pos_a, norm_right },
  { pos_d, norm_right },
  { pos_d, norm_right },
  { pos_h, norm_right },
  { pos_e, norm_right },

  { pos_f, norm_back },
  { pos_e, norm_back },
  { pos_h, norm_back },
  { pos_h, norm_back },
  { pos_g, norm_back },
  { pos_f, norm_back },

  { pos_b, norm_left },
  { pos_f, norm_left },
  { pos_g, norm_left },
  { pos_g, norm_left },
  { pos_c, norm_left },
  { pos_b, norm_left },

  { pos_e, norm_top },
  { pos_f, norm_top },
  { pos_b, norm_top },
  { pos_b, norm_top },
  { pos_a, norm_top },
  { pos_e, norm_top },

  { pos_c, norm_bottom },
  { pos_g, norm_bottom },
  { pos_h, norm_bottom },
  { pos_h, norm_bottom },
  { pos_d, norm_bottom },
  { pos_c, norm_bottom }
};

static CoglPrimitive *
create_cube_primitive (Cube *cube)
{
  CoglAttributeBuffer *attribute_buffer;
  CoglAttribute *attributes[2];
  CoglPrimitive *primitive;

  attribute_buffer = cogl_attribute_buffer_new (cube->context,
                                                sizeof (vertices),
                                                vertices);
  attributes[0] = cogl_attribute_new (attribute_buffer,
                                      "cogl_position_in",
                                      sizeof (Vertex),
                                      offsetof (Vertex, x),
                                      3,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  attributes[1] = cogl_attribute_new (attribute_buffer,
                                      "cogl_normal_in",
                                      sizeof (Vertex),
                                      offsetof (Vertex, n_x),
                                      3,
                                      COGL_ATTRIBUTE_TYPE_FLOAT);
  cogl_object_unref (attribute_buffer);

  primitive = cogl_primitive_new_with_attributes (COGL_VERTICES_MODE_TRIANGLES,
                                                  G_N_ELEMENTS (vertices),
                                                  attributes, 2);
  cogl_object_unref (attributes[0]);
  cogl_object_unref (attributes[1]);

  return primitive;
}

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

  cube.prim = create_cube_primitive (&cube);

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
