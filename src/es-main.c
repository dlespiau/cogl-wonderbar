/*
 * Eclectic Sheep
 *
 * Copyright (C) 2012  Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdlib.h>

#include <cogl/cogl.h>
#include <SDL.h>

#include "es-entity.h"
#include "es-components.h"

#ifndef COGL_VERSION_CHECK
#define COGL_VERSION_CHECK(a,b,c) (FALSE)
#endif

typedef struct
{
  CoglFramebuffer *fb;
  gboolean quit;

  Entity entities[3];
  Entity *selected_entity;
  Entity *light;

  /* shadow mapping */
  CoglOffscreen *shadow_fb;
  CoglTexture2D *shadow_color;
  CoglTexture   *shadow_map;

  CoglPipeline *shadow_color_tex;
  CoglPipeline *shadow_map_tex;

  /* debug */
  CoglTexture *uv_debug;
} Cube;

static Cube cube;
static CoglContext *context;
static GTimer *timer;

CoglContext *
es_get_cogl_context (void)
{
  return context;
}

CoglPipeline *
es_get_root_pipeline (void)
{
  return es_entity_get_pipeline (&cube.entities[1]);
}

/* in micro seconds  */
int64_t
es_get_current_time (void)
{
  return (int64_t) (g_timer_elapsed (timer, NULL) * 1e6);
}

static CoglPipeline *
create_texture_pipeline (CoglTexture *texture)
{
  static CoglPipeline *template = NULL, *new_pipeline;

  if (G_UNLIKELY (template == NULL))
    {
      template = cogl_pipeline_new (context);
      cogl_pipeline_set_layer_null_texture (template, 0, COGL_TEXTURE_TYPE_2D);
    }

  new_pipeline = cogl_pipeline_copy (template);
  cogl_pipeline_set_layer_texture (new_pipeline, 0, texture);

  return new_pipeline;
}


static void
compute_light_shadow_matrix (CoglMatrix *light_matrix,
                             CoglMatrix *light_projection,
                             CoglMatrix *light_view)
{
  /* Move the unit cube from [-1,1] to [0,1] */
  float bias[16] = {
    .5f, .0f, .0f, .0f,
    .0f, .5f, .0f, .0f,
    .0f, .0f, .5f, .0f,
    .5f, .5f, .5f, 1.f
  };

  cogl_matrix_init_from_array (light_matrix, bias);
  cogl_matrix_multiply (light_matrix, light_matrix, light_projection);
  cogl_matrix_multiply (light_matrix, light_matrix, light_view);
}

static void
draw_entities (Cube            *cube,
               CoglFramebuffer *fb,
               gboolean         shadow_pass)
{
  int i;

  for (i = 1; i < 3; i++)
    {
      const CoglMatrix *transform;
      Entity *entity;

      entity = &cube->entities[i];

      if (shadow_pass && !entity_cast_shadow (entity))
        continue;

      cogl_framebuffer_push_matrix (fb);

      transform = es_entity_get_transform (entity);
      cogl_framebuffer_transform (fb, transform);

      es_entity_draw (entity, fb);

      cogl_framebuffer_pop_matrix (fb);
    }
}

static void
draw (Cube *cube)
{
  int i;
  int64_t time; /* micro seconds */
  CoglMatrix shadow_transform;
  CoglFramebuffer *shadow_fb;

  /*
   * render the shadow map
   */

  shadow_fb = COGL_FRAMEBUFFER (cube->shadow_fb);

  cogl_push_framebuffer (shadow_fb);

  cogl_framebuffer_clear4f (shadow_fb,
                            COGL_BUFFER_BIT_COLOR | COGL_BUFFER_BIT_DEPTH,
                            0, 1, 0, 1);

  /* the light position is hardcoded for now */
  cogl_matrix_init_identity (&shadow_transform);
  cogl_matrix_look_at (&shadow_transform,
                       /* light position */
                       es_entity_get_x (cube->light),
                       es_entity_get_y (cube->light),
                       es_entity_get_z (cube->light),
                       .0f, 0.f, 0.f,     /* direction to look at */
                       .0f, 1.f, 0.f);    /* world up */

  cogl_framebuffer_set_modelview_matrix (shadow_fb, &shadow_transform);

  /* update the light matrix uniform */
  {
    CoglMatrix light_shadow_matrix, light_projection, light_view;
    CoglPipeline *pipeline;

    int location;

    cogl_framebuffer_get_projection_matrix (shadow_fb, &light_projection);
    cogl_framebuffer_get_modelview_matrix (shadow_fb, &light_view);
    compute_light_shadow_matrix (&light_shadow_matrix,
                                 &light_projection,
                                 &light_view);

    pipeline = es_entity_get_pipeline (&cube->entities[1]);
    location = cogl_pipeline_get_uniform_location (pipeline,
                                                   "light_shadow_matrix");
    cogl_pipeline_set_uniform_matrix (pipeline,
                                      location,
                                      4, 1,
                                      FALSE,
                                      cogl_matrix_get_array (&light_shadow_matrix));

    pipeline = es_entity_get_pipeline (&cube->entities[2]);
    location = cogl_pipeline_get_uniform_location (pipeline,
                                                   "light_shadow_matrix");
    cogl_pipeline_set_uniform_matrix (pipeline,
                                      location,
                                      4, 1,
                                      FALSE,
                                      cogl_matrix_get_array (&light_shadow_matrix));
  }

  draw_entities (cube, shadow_fb, TRUE /* shadow pass */);

  cogl_pop_framebuffer ();

  /*
   * render the scene
   */

  /* setup the base tranformation matrix, we use clutter so the transformation
   * is relative to the default one in clutter */

  cogl_framebuffer_clear4f (cube->fb,
                            COGL_BUFFER_BIT_COLOR|COGL_BUFFER_BIT_DEPTH,
                            0, 0, 0, 1);

  cogl_framebuffer_push_matrix (cube->fb);

  cogl_framebuffer_translate (cube->fb, 0.f, 0.f, -10.f);

  /* update entities */
  time = es_get_current_time ();

  for (i = 0; i < 3; i++)
    {
      Entity *entity = &cube->entities[i];

      es_entity_update (entity, time);
    }

  /* draw entities */
  draw_entities (cube, cube->fb, FALSE /* shadow pass */);

  /* draw the color and depth buffers of the shadow FBO to debug them */
  cogl_framebuffer_draw_rectangle (cube->fb, cube->shadow_color_tex,
                                   -4, 3, -2, 1);
  cogl_framebuffer_draw_rectangle (cube->fb, cube->shadow_map_tex,
                                   -4, 1, -2, -1);

  cogl_framebuffer_pop_matrix (cube->fb);

  cogl_onscreen_swap_buffers (COGL_ONSCREEN (cube->fb));
}

CoglPipeline *
create_diffuse_specular_material (void)
{
  CoglPipeline *pipeline;
  CoglSnippet *snippet;
  CoglDepthState depth_state;

  pipeline = cogl_pipeline_new (context);
  cogl_pipeline_set_color4f (pipeline, 1.0f, 0.2f, 0.2f, 1.f);

  /* enable depth testing */
  cogl_depth_state_init (&depth_state);
  cogl_depth_state_set_test_enabled (&depth_state, TRUE);
  cogl_pipeline_set_depth_state (pipeline, &depth_state, NULL);

  /* set up our vertex shader */
#if 0
  snippet = cogl_snippet_new (cogl_snippet_hook_vertex,

                              /* definitions */
                              "varying vec4 color;\n"

                              /* per vertex diffuse, directional light */
                              "vec3 normal_direction =\n"
                              "  normalize(gl_NormalMatrix * cogl_normal_in);\n"
                              "vec3 light_direction =\n"
                              "  normalize(vec3(light0.position));\n"

                              "vec3 diffuse = vec3(cogl_color_in) *\n"
                              "               vec3(light0.diffuse) *\n"
                              "               max(0.0, dot(normal_direction,\n"
                              "                            light_direction));\n"

                              "color = vec4(diffuse, 1.0);\n"
                              );
#endif
  snippet = cogl_snippet_new (COGL_SNIPPET_HOOK_VERTEX,

      /* definitions */
      "uniform mat4 light_shadow_matrix;\n"
      "varying vec3 normal_direction, eye_direction;\n"
      "varying vec4 shadow_coords;\n",

      "normal_direction = normalize(gl_NormalMatrix * cogl_normal_in);\n"
      "eye_direction    = -vec3(cogl_modelview_matrix * cogl_position_in);\n"

      "shadow_coords = light_shadow_matrix * cogl_position_in;\n"
  );

  cogl_pipeline_add_snippet (pipeline, snippet);
  cogl_object_unref (snippet);

  /* and fragment shader */
#if 0
  snippet = cogl_snippet_new (COGL_SNIPPET_HOOK_FRAGMENT,
                              /* definitions */
                              "varying vec4 color;\n",
                              /* post */
                              NULL);
  /* per vertex lighting, just forward the color */
  cogl_snippet_set_replace (snippet, "cogl_color_out = color;\n");
#endif
  snippet = cogl_snippet_new (COGL_SNIPPET_HOOK_FRAGMENT,
      /* definitions */
      "uniform vec4 light0_ambient, light0_diffuse, light0_specular;\n"
      "uniform vec3 light0_direction_norm;\n"
      "varying vec3 normal_direction, eye_direction;\n",

      /* post */
      NULL);

  cogl_snippet_set_replace (snippet,
      "vec4 final_color = light0_ambient * cogl_color_in;\n"

      " vec3 L = light0_direction_norm;\n"
      " vec3 N = normalize(normal_direction);\n"

      "float lambert = dot(N, L);\n"

      "if (lambert > 0.0)\n"
      "{\n"
      "  final_color += cogl_color_in * light0_diffuse * lambert;\n"

      "  vec3 E = normalize(eye_direction);\n"
      "  vec3 R = reflect (-L, N);\n"
      "  float specular = pow (max(dot(R, E), 0.0),\n"
      "                        2.);\n"
      "  final_color += light0_specular * vec4(.6, .6, .6, 1.0) * specular;\n"
      "}\n"

      "shadow_coords_d = shadow_coords / shadow_coords.w;\n"
      "cogl_texel7 =  cogl_texture_lookup7 (cogl_sampler7, cogl_tex_coord_in[0]);\n"
      "float distance_from_light = cogl_texel7.z;\n"
      "float shadow = 1.0;\n"
      "if (shadow_coords.w > 0.0 && distance_from_light < shadow_coords_d.z)\n"
      "    shadow = 0.5;\n"

      "cogl_color_out = shadow * final_color;\n"
      //"cogl_color_out = cogl_texel7;\n"
  );

  cogl_pipeline_add_snippet (pipeline, snippet);
  cogl_object_unref (snippet);

  return pipeline;
}

static void
handle_event (Cube *cube, SDL_Event *event)
{
  switch (event->type)
    {
    case SDL_VIDEOEXPOSE:
      draw (cube);
      break;

    case SDL_MOUSEMOTION:
      {
        g_message ("motion %dx%d", event->motion.x, event->motion.y);
      }
      break;

    case SDL_KEYDOWN:
      switch (event->key.keysym.sym)
        {
        case SDLK_o:
          g_message ("Object selected");
          cube->selected_entity = &cube->entities[2];
          break;

        case SDLK_l:
          g_message ("Light selected");
          cube->selected_entity = cube->light;
          break;

        case SDLK_RIGHT:
          {
            float x;
            Entity *entity;

            entity = cube->selected_entity;
            x = es_entity_get_x (entity);
            es_entity_set_x (entity, x + 0.1);

            draw (cube);
          }
          break;

        case SDLK_LEFT:
          {
            float x;
            Entity *entity;

            entity = cube->selected_entity;
            x = es_entity_get_x (entity);
            es_entity_set_x (entity, x - 0.1);

            draw (cube);
          }
          break;

        case SDLK_UP:
          {
            float z;
            Entity *entity;

            entity = cube->selected_entity;
            z = es_entity_get_z (entity);
            es_entity_set_z (entity, z - 0.1);

            draw (cube);
          }
          break;

        case SDLK_DOWN:
          {
            float z;
            Entity *entity;

            entity = cube->selected_entity;
            z = es_entity_get_z (entity);
            es_entity_set_z (entity, z + 0.1);

            draw (cube);
          }
          break;

        default:
          break;
        }
      break;

    case SDL_QUIT:
      cube->quit = TRUE;
      break;
    }
}

static Uint32
timer_handler (Uint32 interval, void *user_data)
{
  static const SDL_UserEvent dummy_event =
    {
      SDL_USEREVENT
    };

  /* Post an event to wake up from SDL_WaitEvent */
  SDL_PushEvent ((SDL_Event *) &dummy_event);

  return 0;
}

static gboolean
wait_event_with_timeout (Cube *cube, SDL_Event *event, gint64 timeout)
{
  if (timeout == -1)
    {
      if (SDL_WaitEvent (event))
        return TRUE;
      else
        {
          cube->quit = TRUE;
          return FALSE;
        }
    }
  else if (timeout == 0)
    return SDL_PollEvent (event);
  else
    {
      gboolean ret;
      /* Add a timer so that we can wake up the event loop */
      SDL_TimerID timer_id =
        SDL_AddTimer (timeout / 1000, timer_handler, cube);

      if (SDL_WaitEvent (event))
        ret = TRUE;
      else
        {
          cube->quit = TRUE;
          ret = FALSE;
        }

      SDL_RemoveTimer (timer_id);

      return ret;
    }
}

int
main (int argc, char **argv)
{
  CoglOnscreen *onscreen;
  GError *error = NULL;
  Component *component;
  CoglPipeline *pipeline1, *pipeline2;
  CoglSnippet *snippet;
  CoglColor color;
  float vector3[3];
  SDL_Event event;

  memset (&cube, 0, sizeof(Cube));

  /*
   * Setup SDL/Cogl
   */

  /* force the SDL winsys */
#if COGL_VERSION_CHECK (1, 99, 0)
  context = cogl_sdl_context_new (SDL_USEREVENT, &error);
#else
    {
    CoglRenderer *renderer;
    CoglDisplay *display;


    renderer = cogl_renderer_new ();
    cogl_renderer_set_winsys_id (renderer, COGL_WINSYS_ID_SDL);
    display = cogl_display_new (renderer, NULL);
    context = cogl_context_new (display, &error);
  }
#endif
  if (!context)
    {
      fprintf (stderr, "Failed to create context: %s\n", error->message);
      return 1;
    }

  SDL_InitSubSystem (SDL_INIT_TIMER);

  onscreen = cogl_onscreen_new (context, 800, 600);
  cube.fb = COGL_FRAMEBUFFER (onscreen);
  cogl_framebuffer_perspective (COGL_FRAMEBUFFER (cube.fb),
                                60.f, /* fov */
                                (float) 800 / 600,  /* aspect ratio */
                                1.1f,  /* near */
                                100);  /* far */

  cogl_onscreen_show (onscreen);

  /* timer for the world time */
  timer = g_timer_new ();

  /* load the debug uv grid */
  cube.uv_debug = cogl_texture_new_from_file ("uvgrid.jpg",
                                              COGL_TEXTURE_NO_ATLAS |
                                              COGL_TEXTURE_NO_SLICING,
                                              COGL_PIXEL_FORMAT_ANY,
                                              &error);
  if (error)
    g_warning ("Could not load uv debug texture: %s", error->message);

  /*
   * Setup shadow mapping
   */
  {
    CoglTexture2D *color_buffer;
    GError *error = NULL;

    color_buffer = cogl_texture_2d_new_with_size (context,
                                                  512, 512,
                                                  COGL_PIXEL_FORMAT_ANY,
                                                  &error);
    if (error)
      g_critical ("could not create texture: %s", error->message);

    cube.shadow_color = color_buffer;

    /* XXX: Right now there's no way to disable rendering to the the color
     * buffer. */
    cube.shadow_fb =
        cogl_offscreen_new_to_texture (COGL_TEXTURE (color_buffer));

    /* directional light -> orthographic perspective */
#if 0
    cogl_framebuffer_orthographic (COGL_FRAMEBUFFER (cube.shadow_fb),
                                   10.f, 10.f, -10.f, -10.f, -15.f, 15.f);

#endif
    cogl_framebuffer_perspective (COGL_FRAMEBUFFER (cube.shadow_fb),
                                  60.f, /* fov */
                                  1.0f,  /* aspect ratio */
                                  1.1f,  /* near */
                                  10.f); /* far */

    /* retrieve the depth texture */
    cogl_framebuffer_enable_depth_texture (COGL_FRAMEBUFFER (cube.shadow_fb),
                                           TRUE);
    cube.shadow_map =
      cogl_framebuffer_get_depth_texture (COGL_FRAMEBUFFER (cube.shadow_fb));

    if (cube.shadow_fb == NULL)
      g_critical ("could not create offscreen buffer");
  }

  /* Hook the shadow sampling */
  pipeline1 = create_diffuse_specular_material ();
  cogl_pipeline_set_layer_texture (pipeline1, 7, cube.shadow_map);
  /* cogl_pipeline_set_layer_texture (pipeline1, 7, cube.uv_debug); */

  cogl_pipeline_set_layer_wrap_mode_s (pipeline1,
                                       7,
                                       COGL_PIPELINE_WRAP_MODE_CLAMP_TO_EDGE);
  cogl_pipeline_set_layer_wrap_mode_t (pipeline1,
                                       7,
                                       COGL_PIPELINE_WRAP_MODE_CLAMP_TO_EDGE);

  snippet = cogl_snippet_new (COGL_SNIPPET_HOOK_TEXTURE_LOOKUP,
                              /* declarations */
                              "varying vec4 shadow_coords;\n"
                              "vec4 shadow_coords_d;\n",
                              /* post */
                              "");

  cogl_snippet_set_replace (snippet,
                            "cogl_texel = texture2D(cogl_sampler7, shadow_coords_d.st);\n");

  cogl_pipeline_add_layer_snippet (pipeline1, 7, snippet);
  cogl_object_unref (snippet);

  /*
   * Setup CoglObjects to render our plane and cube
   */

  /* light */
  cube.light = &cube.entities[0];
  es_entity_init (cube.light);

  vector3[0] = 0.f;
  vector3[1] = 3.0f;
  vector3[2] = -0.5f;
  es_entity_set_position (cube.light, vector3);

  component = es_light_new();
  cogl_color_init_from_4f (&color, .2f, .2f, .2f, 1.f);
  es_light_set_ambient (ES_LIGHT (component), &color);
  cogl_color_init_from_4f (&color, .6f, .6f, .6f, 1.f);
  es_light_set_diffuse (ES_LIGHT (component), &color);
  cogl_color_init_from_4f (&color, .4f, .4f, .4f, 1.f);
  es_light_set_specular (ES_LIGHT (component), &color);

  es_entity_add_component (cube.light, component);

  /* plane */
  es_entity_init (&cube.entities[1]);
  es_entity_set_cast_shadow (&cube.entities[1], FALSE);

  component = es_mesh_renderer_new_from_template ("plane", pipeline1);

  es_entity_add_component (&cube.entities[1], component);

  /* a second, more interesting, entity */
  es_entity_init (&cube.entities[2]);
  es_entity_set_cast_shadow (&cube.entities[2], TRUE);

  pipeline2 = cogl_pipeline_copy (pipeline1);
  cogl_pipeline_set_color4f (pipeline2, 0.0f, 0.1f, 5.0f, 1.0f);

  component = es_mesh_renderer_new_from_file ("cone.ply", pipeline2);

  es_entity_add_component (&cube.entities[2], component);

  /* animate the x property of the second entity */
#if 0
  component = es_animation_clip_new (2000);
  es_animation_clip_add_float (ES_ANIMATION_CLIP (component),
                               &cube.entities[2],
                               FLOAT_GETTER (es_entity_get_x),
                               FLOAT_SETTER (es_entity_set_x),
                               5.0f);
  es_animation_clip_start (ES_ANIMATION_CLIP (component));

  es_entity_add_component (&cube.entities[2], component);
#endif

  /* animate the rotation of the second entity */
#if 0
  {
    CoglEuler end_angles;
    CoglQuaternion end_rotation;

    cogl_euler_init (&end_angles, 90, -90, 0);
    cogl_quaternion_init_from_euler (&end_rotation, &end_angles);

    component = es_animation_clip_new (5000);
    es_animation_clip_add_quaternion (ES_ANIMATION_CLIP (component),
                                      &cube.entities[2],
                                      QUATERNION_GETTER (es_entity_get_rotation),
                                      QUATERNION_SETTER (es_entity_set_rotation),
                                      &end_rotation);

    es_animation_clip_start (ES_ANIMATION_CLIP (component));

    es_entity_add_component (&cube.entities[2], component);
  }
#endif

  /* default to selecting the interesting object */
  cube.selected_entity = &cube.entities[2];

  /* create the pipelines to display the shadow color and depth textures */
  cube.shadow_color_tex =
      create_texture_pipeline (COGL_TEXTURE (cube.shadow_color));
  cube.shadow_map_tex =
      create_texture_pipeline (COGL_TEXTURE (cube.shadow_map));

  cogl_object_unref (pipeline1);
  cogl_object_unref (pipeline2);

  g_timer_start (timer);

  /*
   * Main loop
   */

  while (!cube.quit)
    {
      CoglPollFD *poll_fds;
      int n_poll_fds;
      gint64 timeout;

      cogl_poll_get_info (context, &poll_fds, &n_poll_fds, &timeout);

      /* It's difficult to wait for file descriptors using the SDL
         event mechanism, but it the SDL winsys is documented that it
         will never require this so we can assert that there are no
         fds */
      g_assert (n_poll_fds == 0);

      if (wait_event_with_timeout (&cube, &event, timeout))
        do
          handle_event (&cube, &event);
        while (SDL_PollEvent (&event));

      cogl_poll_dispatch (context, poll_fds, n_poll_fds);
    }

  return EXIT_SUCCESS;
}
