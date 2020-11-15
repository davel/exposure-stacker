/* vim:ts=4:shiftwidth=4:expandtab:smartindent
*/

#define _GNU_SOURCE
#include <gegl.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <glib/gprintf.h>
#include <stdlib.h>

const int exposures = 87;

gint main(gint argc, gchar *argv[]) {

    gegl_init (&argc, &argv);  /* initialize the GEGL library */


    GeglNode *gegl = gegl_node_new();


    GeglNode *source_image[exposures];

    for (int i=0; i<exposures; i++) {
        gchar *filename;
        asprintf(&filename, "bitmap/%04d.tif", i);
        source_image[i] = gegl_node_new_child(
            gegl,
            "operation", "gegl:load",
            "path", filename,
            NULL);
        free(filename);
    }

    GeglNode *sigma = source_image[0];
    for (int i=1; i<exposures; i++) {
        GeglNode *sum = gegl_node_new_child(
            gegl,
            "operation", "gegl:add",
            NULL);
        gegl_node_link_many(sigma, sum, NULL);
        gegl_node_connect_to(source_image[i], "output", sum, "aux");
        sigma = sum;
    }

    gdouble denumerator = exposures;

    GeglNode *mean = gegl_node_new_child(
        gegl,
        "operation", "gegl:divide",
        "value", denumerator,
        NULL
    );


    GeglNode *write_buffer = gegl_node_new_child(
        gegl,
        "operation", "gegl:save",
        "path", "output.tif",
        NULL);

    gegl_node_link_many(sigma, mean, write_buffer, NULL);
    gegl_node_process(write_buffer);

    g_object_unref(gegl);

    gegl_exit();

    return 0;
}
