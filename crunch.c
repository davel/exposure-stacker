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

    GeglBuffer *buffer;
    GeglNode *write_buffer = gegl_node_new_child(
        gegl,
        "operation", "gegl:buffer-sink",
        "buffer", &buffer,
        NULL);

    gegl_node_link_many(sigma, write_buffer, NULL);
    gegl_node_process(write_buffer);

    g_object_unref(gegl);

    GeglNode *gegl_save = gegl_node_new();
    GeglNode *src = gegl_node_new_child(
        gegl_save,
        "operation",
        "gegl:buffer-source",
        "buffer", buffer,
        NULL);
    GeglNode *sink = gegl_node_new_child(
        gegl_save,
        "operation", "gegl:save",
        "path", "output.tif",
        NULL
    );
    gegl_node_link_many(src, sink, NULL);
    gegl_node_process(sink);
    g_object_unref(gegl_save); 

    g_object_unref(buffer);

    gegl_exit();

    return 0;
}
