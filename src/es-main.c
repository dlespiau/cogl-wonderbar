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

#define USER_ENTITY 2
#define N_ENTITIES  4

typedef struct
{
  CoglFramebuffer *fb;
  gboolean quit;

  Entity entities[N_ENTITIES];
  Entity *selected_entity;
  Entity *main_camera;
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
  return es_entity_get_pipeline (&cube.entities[USER_ENTITY]);
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
  /* Move the unit cube from [-1,1] to [0,1], column major order */
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
               Entity          *camera,
               gboolean         shadow_pass)
{
  CoglMatrix *transform, inverse;
  int i;

  transform = es_entity_get_transform (camera);
  cogl_matrix_get_inverse (transform, &inverse);
  if (shadow_pass)
    {
      cogl_framebuffer_identity_matrix (fb);
      cogl_framebuffer_scale (fb, 1, -1, 1);
      cogl_framebuffer_transform (fb, &inverse);
    }
  else
    {
      cogl_framebuffer_set_modelview_matrix (fb, &inverse);
    }
  es_entity_draw (camera, fb);

  for (i = 1; i < N_ENTITIES; i++)
    {
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
  CoglFramebuffer *shadow_fb;

  /*
   * update entities
   */

  time = es_get_current_time ();

  for (i = 0; i < N_ENTITIES; i++)
    {
      Entity *entity = &cube->entities[i];

      es_entity_update (entity, time);
    }

  /*
   * render the shadow map
   */

  shadow_fb = COGL_FRAMEBUFFER (cube->shadow_fb);

  /* update the light matrix uniform */
  {
    CoglMatrix light_shadow_matrix, light_projection, *light_transform,
               light_view;
    CoglPipeline *pipeline;

    int location;

    cogl_framebuffer_get_projection_matrix (shadow_fb, &light_projection);
    light_transform = es_entity_get_transform (cube->light);
    cogl_matrix_get_inverse (light_transform, &light_view);
    compute_light_shadow_matrix (&light_shadow_matrix,
                                 &light_projection,
                                 &light_view);

    pipeline = es_entity_get_pipeline (&cube->entities[USER_ENTITY]);
    location = cogl_pipeline_get_uniform_location (pipeline,
                                                   "light_shadow_matrix");
    cogl_pipeline_set_uniform_matrix (pipeline,
                                      location,
                                      4, 1,
                                      FALSE,
                                      cogl_matrix_get_array (&light_shadow_matrix));

    pipeline = es_entity_get_pipeline (&cube->entities[USER_ENTITY + 1]);
    location = cogl_pipeline_get_uniform_location (pipeline,
                                                   "light_shadow_matrix");
    cogl_pipeline_set_uniform_matrix (pipeline,
                                      location,
                                      4, 1,
                                      FALSE,
                                      cogl_matrix_get_array (&light_shadow_matrix));
  }

  draw_entities (cube, shadow_fb, cube->light, TRUE /* shadow pass */);

  /*
   * render the scene
   */

  cogl_framebuffer_push_matrix (cube->fb);

  /* draw entities */
  draw_entities (cube, cube->fb, cube->main_camera, FALSE /* shadow pass */);

  /* draw the color and depth buffers of the shadow FBO to debug them */
  cogl_framebuffer_draw_rectangle (cube->fb, cube->shadow_color_tex,
                                   -2, 1, -4, 3);
  cogl_framebuffer_draw_rectangle (cube->fb, cube->shadow_map_tex,
                                   -2, -1, -4, 1);

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
      "float distance_from_light = cogl_texel7.z + 0.005;\n"
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

    case SDL_KEYDOWN:
      switch (event->key.keysym.sym)
        {
        case SDLK_o:
          g_message ("Object selected");
          cube->selected_entity = &cube->entities[USER_ENTITY + 1];
          break;

        case SDLK_l:
          g_message ("Light selected");
          cube->selected_entity = cube->light;
          break;

        case SDLK_c:
          g_message ("Camera selected");
          cube->selected_entity = cube->main_camera;
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
  CoglPipeline *root_pipeline, *pipeline;
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

    /* retrieve the depth texture */
    cogl_framebuffer_enable_depth_texture (COGL_FRAMEBUFFER (cube.shadow_fb),
                                           TRUE);
    cube.shadow_map =
      cogl_framebuffer_get_depth_texture (COGL_FRAMEBUFFER (cube.shadow_fb));

    if (cube.shadow_fb == NULL)
      g_critical ("could not create offscreen buffer");
  }

  /* Hook the shadow sampling */
  root_pipeline = create_diffuse_specular_material ();
  cogl_pipeline_set_layer_texture (root_pipeline, 7, cube.shadow_map);
  /* cogl_pipeline_set_layer_texture (root_pipeline, 7, cube.uv_debug); */

  cogl_pipeline_set_layer_wrap_mode_s (root_pipeline,
                                       7,
                                       COGL_PIPELINE_WRAP_MODE_CLAMP_TO_EDGE);
  cogl_pipeline_set_layer_wrap_mode_t (root_pipeline,
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

  cogl_pipeline_add_layer_snippet (root_pipeline, 7, snippet);
  cogl_object_unref (snippet);

  /*
   * Setup CoglObjects to render our plane and cube
   */

  /* camera */
  cube.main_camera = &cube.entities[0];
  es_entity_init (cube.main_camera);

  vector3[0] = 0.f;
  vector3[1] = 2.f;
  vector3[2] = 10.f;
  es_entity_set_position (cube.main_camera, vector3);

  component = es_camera_new ();

  es_camera_set_framebuffer (ES_CAMERA (component), cube.fb);
  es_camera_set_field_of_view (ES_CAMERA (component), 60.f);
  es_camera_set_near_plane (ES_CAMERA (component), 1.1f);
  es_camera_set_far_plane (ES_CAMERA (component), 100.f);

  es_entity_add_component (cube.main_camera, component);

  /* light */
  cube.light = &cube.entities[1];
  es_entity_init (cube.light);

  vector3[0] = 1.0f;
  vector3[1] = 8.0f;
  vector3[2] = -2.0f;
  es_entity_set_position (cube.light, vector3);

  es_entity_rotate_x_axis (cube.light, -120);
  es_entity_rotate_y_axis (cube.light, 10);

  component = es_light_new ();
  cogl_color_init_from_4f (&color, .2f, .2f, .2f, 1.f);
  es_light_set_ambient (ES_LIGHT (component), &color);
  cogl_color_init_from_4f (&color, .6f, .6f, .6f, 1.f);
  es_light_set_diffuse (ES_LIGHT (component), &color);
  cogl_color_init_from_4f (&color, .4f, .4f, .4f, 1.f);
  es_light_set_specular (ES_LIGHT (component), &color);

  es_entity_add_component (cube.light, component);

  component = es_camera_new ();

  cogl_color_init_from_4f (&color, 0.f, .3f, 0.f, 1.f);
  es_camera_set_background_color (ES_CAMERA (component), &color);
  es_camera_set_framebuffer (ES_CAMERA (component),
                             COGL_FRAMEBUFFER (cube.shadow_fb));
  es_camera_set_projection (ES_CAMERA (component), ES_PROJECTION_ORTHOGRAPHIC);
  es_camera_set_size_of_view (ES_CAMERA (component), 3);
  es_camera_set_near_plane (ES_CAMERA (component), 1.1f);
  es_camera_set_far_plane (ES_CAMERA (component), 20.f);

  es_entity_add_component (cube.light, component);


  /* plane */
  es_entity_init (&cube.entities[USER_ENTITY]);
  es_entity_set_cast_shadow (&cube.entities[USER_ENTITY], FALSE);

  component = es_mesh_renderer_new_from_template ("plane", root_pipeline);

  es_entity_add_component (&cube.entities[USER_ENTITY], component);

  /* a second, more interesting, entity */
  es_entity_init (&cube.entities[USER_ENTITY + 1]);
  es_entity_set_cast_shadow (&cube.entities[USER_ENTITY + 1], TRUE);

  pipeline = cogl_pipeline_copy (root_pipeline);
  cogl_pipeline_set_color4f (pipeline, 0.0f, 0.1f, 5.0f, 1.0f);

  component = es_mesh_renderer_new_from_template ("cube", pipeline);
  cogl_object_unref (pipeline);

  es_entity_add_component (&cube.entities[USER_ENTITY + 1], component);

  /* animate the x property of the second entity */
#if 0
  component = es_animation_clip_new (2000);
  es_animation_clip_add_float (ES_ANIMATION_CLIP (component),
                               &cube.entities[USER_ENTITY + 1],
                               FLOAT_GETTER (es_entity_get_x),
                               FLOAT_SETTER (es_entity_set_x),
                               5.0f);
  es_animation_clip_start (ES_ANIMATION_CLIP (component));

  es_entity_add_component (&cube.entities[USER_ENTITY + 1], component);
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
                                      &cube.entities[USER_ENTITY + 1],
                                      QUATERNION_GETTER (es_entity_get_rotation),
                                      QUATERNION_SETTER (es_entity_set_rotation),
                                      &end_rotation);

    es_animation_clip_start (ES_ANIMATION_CLIP (component));

    es_entity_add_component (&cube.entities[USER_ENTITY + 1], component);
  }
#endif

  /* default to selecting the interesting object */
  cube.selected_entity = &cube.entities[USER_ENTITY + 1];

  /* create the pipelines to display the shadow color and depth textures */
  cube.shadow_color_tex =
      create_texture_pipeline (COGL_TEXTURE (cube.shadow_color));
  cube.shadow_map_tex =
      create_texture_pipeline (COGL_TEXTURE (cube.shadow_map));

  cogl_object_unref (root_pipeline);

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
