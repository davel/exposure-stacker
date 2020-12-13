/* vim:ts=4:shiftwidth=4:expandtab:smartindent
*/

#define _GNU_SOURCE
#include <gegl.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <glib/gprintf.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

const int exposures = 384;
const int output_frames = 50*50;
const int still_frames = 5*50;

gint main(gint argc, gchar *argv[]) {

    gegl_init (&argc, &argv);  /* initialize the GEGL library */


    GeglNode *gegl = gegl_node_new();


    GeglNode *source_image[exposures];

    for (int i=0; i<exposures; i++) {
        gchar *filename;
        asprintf(&filename, "bitmap/%04d.tif", i);
        assert(filename);
        source_image[i] = gegl_node_new_child(
            gegl,
            "operation", "gegl:load",
            "path", filename,
            NULL);
        free(filename);
    }

    double rate = sqrt(exposures-1) / (output_frames-still_frames);

    for (int frame = 0; frame< output_frames; frame++) {
        double exposures_in_frame = pow(((double) frame * rate), 2)+1;
        if (exposures_in_frame>exposures) {
            exposures_in_frame = exposures;
        }

        printf("frame %04d exposures %04.3f\n", frame, exposures_in_frame);

        GeglNode *sigma = source_image[frame % exposures];
        for (int i=1; (double) i<exposures_in_frame; i++) {
            GeglNode *scaled = source_image[(i+frame) % exposures];

            gdouble remaining_exposure = exposures_in_frame - (gdouble)i;

            if (remaining_exposure < 1) {
                printf("remaining exposure %f\n", remaining_exposure);
                GeglNode *s = gegl_node_new_child(
                    gegl,
                    "operation", "gegl:multiply",
                    "value", remaining_exposure,
                    NULL);
                gegl_node_link_many(scaled, s, NULL);
                scaled = s;
            }


            GeglNode *sum = gegl_node_new_child(
                gegl,
                "operation", "gegl:add",
                NULL);
            gegl_node_link_many(sigma, sum, NULL);
            gegl_node_connect_to(scaled, "output", sum, "aux");
            sigma = sum;
        }

        gdouble denumerator = exposures_in_frame;

        GeglNode *mean = gegl_node_new_child(
            gegl,
            "operation", "gegl:divide",
            "value", denumerator,
            NULL
        );


        gdouble exposure_correction = 9.0;
        gdouble black_level         = 0.00010;

        GeglNode *exposure = gegl_node_new_child(
            gegl,
            "operation", "gegl:exposure",
            "exposure", exposure_correction,
            "black-level", black_level,
            NULL
        );

        gchar *filename;
        asprintf(&filename, "output/%04d.png", frame);
        assert(filename);

        GeglNode *write_buffer = gegl_node_new_child(
            gegl,
            "operation", "gegl:save",
            "path", filename,
            NULL);


        gegl_node_link_many(sigma, mean, exposure, write_buffer, NULL);
        gegl_node_process(write_buffer);

        free(filename);
        g_object_unref(sigma);
        g_object_unref(mean);
        g_object_unref(exposure);
        g_object_unref(write_buffer);
    }

    g_object_unref(gegl);

    gegl_exit();

    return 0;
}
