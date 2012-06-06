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

#include "es-util.h"

static void
print_matrix_floats (const char  *prefix,
                     const float  m[16])
{
  int i;

  for (i = 0; i < 4; i++)
    g_print ("%s\t%f %f %f %f\n", prefix, m[i], m[4+i], m[8+i], m[12+i] );
}

void
es_print_matrix (const char *prefix,
                 CoglMatrix *matrix)
{
  print_matrix_floats (prefix, (float *)matrix);
}

