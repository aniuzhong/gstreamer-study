#include <gst/gst.h>

int main(int argc, char *argv[])
{
    GstElement *pipeline, *bin, *filter, *convert, *sink;
    GstPad *pad, *ghost_pad;
    GstBus *bus;
    GstMessage *msg;

    gst_init(&argc, &argv);

    pipeline = gst_parse_launch("playbin uri=uri=https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm", NULL);

    /* Create the elements inside the sink bin */
    filter = gst_element_factory_make("solarize", "filter");
    convert = gst_element_factory_make("videoconvert", "convert");
    sink = gst_element_factory_make("autovideosink", "video_sink");
    if (!filter || !convert || !sink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    /* Create the sink bin, add the elements and link them */
    bin = gst_bin_new("video_sink_bin");
    gst_bin_add_many(GST_BIN(bin), filter, convert, sink, NULL);
    gst_element_link_many(filter, convert, sink, NULL);
    pad = gst_element_get_static_pad(filter, "sink");
    ghost_pad = gst_ghost_pad_new("sink", pad);
    gst_pad_set_active(ghost_pad, TRUE);
    gst_element_add_pad(bin, ghost_pad);
    gst_object_unref(pad);

    /* Set playbin's video sink to be our sink bin */
    g_object_set(GST_OBJECT(pipeline), "video-sink", bin, NULL);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    if (msg != NULL)
        gst_message_unref(msg);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}
