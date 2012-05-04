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
#include <clutter/clutter.h>

#include "entity.h"
#include "es-components.h"

typedef struct
{
  ClutterActor *stage;
  ClutterTimeline *timeline;

  Entity entities[3];
} Cube;

static CoglContext *context;

CoglContext *
es_get_cogl_context (void)
{
  return context;
}

static void
paint (ClutterActor *stage,
       gpointer      data)
{
  Cube *cube = data;
  CoglFramebuffer *fb;
  int width;
  int height, i;
  //double progress;

  fb = cogl_get_draw_framebuffer ();
  width = cogl_framebuffer_get_width (fb);
  height = cogl_framebuffer_get_height (fb);
  //progress = clutter_timeline_get_progress (cube->timeline);

  cogl_framebuffer_clear4f (fb,
                            COGL_BUFFER_BIT_COLOR|COGL_BUFFER_BIT_DEPTH,
                            0, 0, 0, 1);

  cogl_framebuffer_push_matrix (fb);

  cogl_framebuffer_translate (fb, width / 2, height / 2, 0);
  cogl_framebuffer_scale (fb, 75, -75, 75);

  /* draw entities */
  for (i = 0; i < 2; i++)
    {
      const CoglMatrix *transform;
      Entity *entity;

      cogl_framebuffer_push_matrix (fb);

      entity = &cube->entities[i];
      transform = es_entity_get_transform (entity);

      cogl_framebuffer_transform (fb, transform);
      es_entity_draw (entity);

      cogl_framebuffer_pop_matrix (fb);
    }

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
      "varying vec3 normal_direction, light_direction, eye_direction;\n"

      "struct light\n"
      "{\n"
      "  vec4 position;\n"
      "  vec4 diffuse;\n"
      "};\n"

      "light light0 = light(\n"
      "  vec4(1.0, 1.0, 1.0, 0.0),\n"
      "  vec4(1.0, 0.8, 0.8, 1.0)\n"
      ");\n",

      "normal_direction = normalize(gl_NormalMatrix * cogl_normal_in);\n"
      "light_direction  = normalize(vec3(light0.position));\n"
      "eye_direction    = -vec3(cogl_modelview_matrix * cogl_position_in);\n"
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
      "varying vec3 normal_direction, light_direction, eye_direction;\n"

      "struct light\n"
      "{\n"
      "  vec4 position;\n"
      "  vec4 ambient;\n"
      "  vec4 diffuse;\n"
      "  vec4 specular;\n"
      "};\n"

      "light light0 = light(\n"
      "  vec4(1.0, 1.0, 1.0, 0.0),\n"
      "  vec4(0.2, 0.2, 0.2, 1.0),\n"
      "  vec4(1.0, 0.8, 0.8, 1.0),\n"
      "  vec4(0.6, 0.6, 0.6, 1.0)\n"
      ");\n",

      /* post */
      NULL);
  cogl_snippet_set_replace (snippet,
      "vec4 final_color = light0.ambient * cogl_color_in;\n"

      " vec3 L = normalize(light_direction);\n"
      " vec3 N = normalize(normal_direction);\n"

      "float lambert = dot(N, L);\n"

      "if (lambert > 0.0)\n"
      "{\n"
      "  final_color += cogl_color_in * light0.diffuse * lambert;\n"

      "  vec3 E = normalize(eye_direction);\n"
      "  vec3 R = reflect (-L, N);\n"
      "  float specular = pow (max(dot(R, E), 0.0),\n"
      "                        2.);\n"
      "  final_color += light0.specular * vec4(.6, .6, .6, 1.0) * specular;\n"
      "}\n"

      "cogl_color_out = final_color;\n"
  );

  cogl_pipeline_add_snippet (pipeline, snippet);
  cogl_object_unref (snippet);

  return pipeline;
}

int
main (int argc, char **argv)
{
  Cube cube;
  ClutterBackend *backend;
  Component *component;
  CoglPipeline *pipeline1, *pipeline2;

  /*
   * Setup Clutter
   */
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
   * Setup CoglObjects to render our plane and cube
   */
  backend = clutter_get_default_backend ();
  context = clutter_backend_get_cogl_context (backend);


  /* plane */
  es_entity_init (&cube.entities[0]);

  pipeline1 = create_diffuse_specular_material ();
  component = es_mesh_renderer_new_from_template ("plane", pipeline1);

  es_entity_add_component (&cube.entities[0], component);


  /* sphere */
  es_entity_init (&cube.entities[1]);

  pipeline2 = cogl_pipeline_copy (pipeline1);
  cogl_pipeline_set_color4f (pipeline2, 0.0f, 0.1f, 5.0f, 1.0f);
  component = es_mesh_renderer_new_from_file ("sphere.ply", pipeline2);

  es_entity_add_component (&cube.entities[1], component);

  cogl_object_unref (pipeline1);
  cogl_object_unref (pipeline2);

  clutter_actor_show_all (cube.stage);
  clutter_timeline_start (cube.timeline);

  clutter_main ();

  return EXIT_SUCCESS;
}
