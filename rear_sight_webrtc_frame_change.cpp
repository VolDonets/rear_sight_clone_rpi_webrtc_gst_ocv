//
// Created by biba_bo on 2020-08-12.
//

#include "rear_sight_processor/image_processing.h"
#include "rear_sight_processor/rear_sight_processor.h"

#include <locale.h>
#include <glib.h>
#include <gst/gst.h>
#include <gst/sdp/sdp.h>

#ifdef G_OS_UNIX
#include <glib-unix.h>
#endif

#define GST_USE_UNSTABLE_API
#include <gst/webrtc/webrtc.h>

#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include <string.h>

#include <opencv2/opencv.hpp>
#include <iostream>

/*This ia a payload server for possibility to establish client-server connection
 * I tried to use a local payload server, but I had difficulties with configuring it.*/
#define RTP_PAYLOAD_TYPE "96"
#define SOUP_HTTP_PORT 57778
#define STUN_SERVER "stun.l.google.com:19302"
//#define STUN_SERVER "127.0.0.1:3478"

//Struct which contains pipeline and other GstElements, for comfortable work with connection.
typedef struct _ReceiverEntry ReceiverEntry;

//Creating a structure and destroying it. Last one is used often, when we often change a connection.
ReceiverEntry *create_receiver_entry (SoupWebsocketConnection * connection);
void destroy_receiver_entry (gpointer receiver_entry_ptr);


GstPadProbeReturn payloader_caps_event_probe_cb (GstPad * pad,
                                                 GstPadProbeInfo * info, gpointer user_data);

void on_offer_created_cb (GstPromise * promise, gpointer user_data);
void on_negotiation_needed_cb (GstElement * webrtcbin, gpointer user_data);
void on_ice_candidate_cb (GstElement * webrtcbin, guint mline_index,
                          gchar * candidate, gpointer user_data);

void soup_websocket_message_cb (SoupWebsocketConnection * connection,
                                SoupWebsocketDataType data_type, GBytes * message, gpointer user_data);
void soup_websocket_closed_cb (SoupWebsocketConnection * connection,
                               gpointer user_data);

void soup_http_handler (SoupServer * soup_server, SoupMessage * message,
                        const char *path, GHashTable * query, SoupClientContext * client_context,
                        gpointer user_data);
void soup_websocket_handler (G_GNUC_UNUSED SoupServer * server,
                             SoupWebsocketConnection * connection, const char *path,
                             SoupClientContext * client_context, gpointer user_data);

static gchar *get_string_from_json_object (JsonObject * object);

struct _ReceiverEntry
{
    SoupWebsocketConnection *connection;

    GstElement *ocvvideosrc;
    GstElement *pipeline;
    GstElement *webrtcbin;
};

const gchar *html_source;
std::shared_ptr<FrameParameters> frame_param = std::make_shared<FrameParameters>();
std::shared_ptr<RearSightProcessor> rear_sight_processor = std::make_shared<RearSightProcessor>(frame_param);

/// a GstPad callback function, it is used for modification a pipeline stream
static GstPadProbeReturn cb_have_data (GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    GstMapInfo map;
    GstBuffer *buffer;

    buffer = GST_PAD_PROBE_INFO_BUFFER (info);

    buffer = gst_buffer_make_writable (buffer);

    /* Making a buffer writable can fail (for example if it
     * cannot be copied and is used more than once)
     */
    if (buffer == NULL)
        return GST_PAD_PROBE_OK;

    if (gst_buffer_map (buffer, &map, GST_MAP_WRITE)) {
        cv::Mat main_image, copy_main_image, done_main_image, done_mini_image, cropped_img;
        cv::Size frame_size(WIDTH, HEIGHT);

        main_image = cv::Mat(frame_size, CV_8UC4, (char*)(map.data), cv::Mat::AUTO_STEP);
        copy_main_image = main_image.clone();

        cv::Rect my_interest_region(frame_param->CROPPED_X, frame_param->CROPPED_Y,
                                    frame_param->CROPPED_WIDTH, frame_param->CROPPED_HEIGHT);
        cv::rectangle(copy_main_image, my_interest_region, cv::Scalar(0, 0,255), 2, 0, 0);

        cropped_img = main_image(my_interest_region);
        cv::resize(cropped_img, done_main_image, cv::Size(WIDTH, HEIGHT));
        cv::resize(copy_main_image, done_mini_image, cv::Size(RESIZE_WIDTH, RESIZE_HEIGHT));

        cv::Rect main_insertion_coord(0, 0, WIDTH, HEIGHT);
        cv::Rect mini_insertion_coord(RESIZE_X, RESIZE_Y, RESIZE_WIDTH, RESIZE_HEIGHT);

        done_main_image.copyTo(main_image(main_insertion_coord));
        done_mini_image.copyTo(main_image(mini_insertion_coord));

        cv::circle(main_image, cv::Point(DRAW_CIRCLE_X, DRAW_CIRCLE_Y), DRAW_CIRCLE_RADIUS, cv::Scalar(0, 0, 255), 2, cv::LINE_8, 0);
        cv::line(main_image, cv::Point(DRAW_LINE_1B_X, DRAW_LINE_1B_Y), cv::Point(DRAW_LINE_1E_X, DRAW_LINE_1E_Y), cv::Scalar(0, 0, 255), 2, cv::LINE_8);
        cv::line(main_image, cv::Point(DRAW_LINE_2B_X, DRAW_LINE_2B_Y), cv::Point(DRAW_LINE_2E_X, DRAW_LINE_2E_Y), cv::Scalar(0, 0, 255), 2, cv::LINE_8);
        cv::line(main_image, cv::Point(DRAW_LINE_3B_X, DRAW_LINE_3B_Y), cv::Point(DRAW_LINE_3E_X, DRAW_LINE_3E_Y), cv::Scalar(0, 0, 255), 2, cv::LINE_8);
        cv::line(main_image, cv::Point(DRAW_LINE_4B_X, DRAW_LINE_4B_Y), cv::Point(DRAW_LINE_4E_X, DRAW_LINE_4E_Y), cv::Scalar(0, 0, 255), 2, cv::LINE_8);

        gst_buffer_unmap (buffer, &map);
    }

    GST_PAD_PROBE_INFO_DATA (info) = buffer;

    return GST_PAD_PROBE_OK;
}

//a function fro filling a ReceiverEntry structure
//Here creates a pipeline, and addes a callback function for stream modifications
ReceiverEntry* create_receiver_entry (SoupWebsocketConnection * connection) {
    GError *error;
    ReceiverEntry *receiver_entry;
    GstWebRTCRTPTransceiver *trans;
    GArray *transceivers;

    receiver_entry = static_cast<ReceiverEntry *>(g_slice_alloc0(sizeof(ReceiverEntry)));
    receiver_entry->connection = connection;

    g_object_ref (G_OBJECT (connection));

    g_signal_connect (G_OBJECT (connection), "message",
                      G_CALLBACK (soup_websocket_message_cb), (gpointer) receiver_entry);

    error = NULL;
    /**/receiver_entry->pipeline =
                gst_parse_launch ("webrtcbin name=webrtcbin  stun-server=stun://" STUN_SERVER " "
                                  "v4l2src device=/dev/video0 "
                                  "! video/x-raw,width=" STR_WIDTH ",height=" STR_HEIGHT ",framerate=" STR_FRAMERATE " "
                                  "! videoconvert name=ocvvideosrc "
                                  "! video/x-raw,format=BGRA "
                                  "! videoconvert "
                                  "! queue max-size-buffers=1 "
                                  "! x264enc speed-preset=ultrafast tune=zerolatency key-int-max=15 "
                                  "! video/x-h264,profile=constrained-baseline "
                                  "! queue max-size-time=0 "
                                  "! h264parse "
                                  "! rtph264pay config-interval=-1 name=payloader "
                                  "! application/x-rtp,media=video,encoding-name=H264,payload=" RTP_PAYLOAD_TYPE " "
                                  "! webrtcbin. ", &error);



    if (error != NULL) {
        g_error ("Could not create WebRTC pipeline: %s\n", error->message);
        g_error_free (error);
        goto cleanup;
    }

    receiver_entry->ocvvideosrc = gst_bin_get_by_name(GST_BIN(receiver_entry->pipeline), "ocvvideosrc");
    GstPad *pad;
    pad = gst_element_get_static_pad(receiver_entry->ocvvideosrc, "src");
    gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback)cb_have_data, NULL, NULL);
    gst_object_unref(pad);

    receiver_entry->webrtcbin =
            gst_bin_get_by_name (GST_BIN (receiver_entry->pipeline), "webrtcbin");
    g_assert (receiver_entry->webrtcbin != NULL);

    g_signal_emit_by_name (receiver_entry->webrtcbin, "get-transceivers",
                           &transceivers);
    g_assert (transceivers != NULL && transceivers->len > 0);
    trans = g_array_index (transceivers, GstWebRTCRTPTransceiver *, 0);
    trans->direction = GST_WEBRTC_RTP_TRANSCEIVER_DIRECTION_SENDONLY;
    g_array_unref (transceivers);

    g_signal_connect (receiver_entry->webrtcbin, "on-negotiation-needed",
                      G_CALLBACK (on_negotiation_needed_cb), (gpointer) receiver_entry);

    g_signal_connect (receiver_entry->webrtcbin, "on-ice-candidate",
                      G_CALLBACK (on_ice_candidate_cb), (gpointer) receiver_entry);

    gst_element_set_state (receiver_entry->pipeline, GST_STATE_PLAYING);

    return receiver_entry;

    cleanup:
    destroy_receiver_entry ((gpointer) receiver_entry);
    return NULL;
}

void
destroy_receiver_entry (gpointer receiver_entry_ptr)
{
    ReceiverEntry *receiver_entry = (ReceiverEntry *) receiver_entry_ptr;

    g_assert (receiver_entry != NULL);

    if (receiver_entry->pipeline != NULL) {
        gst_element_set_state (GST_ELEMENT (receiver_entry->pipeline),
                               GST_STATE_NULL);

        gst_object_unref (GST_OBJECT (receiver_entry->webrtcbin));
        gst_object_unref (GST_OBJECT (receiver_entry->pipeline));
    }

    if (receiver_entry->connection != NULL)
        g_object_unref (G_OBJECT (receiver_entry->connection));
    if (receiver_entry->ocvvideosrc != NULL)
        g_object_unref(G_OBJECT((receiver_entry->ocvvideosrc)));

    g_slice_free1 (sizeof (ReceiverEntry), receiver_entry);
}


void
on_offer_created_cb (GstPromise * promise, gpointer user_data)
{
    gchar *sdp_string;
    gchar *json_string;
    JsonObject *sdp_json;
    JsonObject *sdp_data_json;
    GstStructure const *reply;
    GstPromise *local_desc_promise;
    GstWebRTCSessionDescription *offer = NULL;
    ReceiverEntry *receiver_entry = (ReceiverEntry *) user_data;

    reply = gst_promise_get_reply (promise);
    gst_structure_get (reply, "offer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION,
                       &offer, NULL);
    gst_promise_unref (promise);

    local_desc_promise = gst_promise_new ();
    g_signal_emit_by_name (receiver_entry->webrtcbin, "set-local-description",
                           offer, local_desc_promise);
    gst_promise_interrupt (local_desc_promise);
    gst_promise_unref (local_desc_promise);

    sdp_string = gst_sdp_message_as_text (offer->sdp);
    g_print ("Negotiation offer created:\n%s\n", sdp_string);

    sdp_json = json_object_new ();
    json_object_set_string_member (sdp_json, "type", "sdp");

    sdp_data_json = json_object_new ();
    json_object_set_string_member (sdp_data_json, "type", "offer");
    json_object_set_string_member (sdp_data_json, "sdp", sdp_string);
    json_object_set_object_member (sdp_json, "data", sdp_data_json);

    json_string = get_string_from_json_object (sdp_json);
    json_object_unref (sdp_json);

    soup_websocket_connection_send_text (receiver_entry->connection, json_string);
    g_free (json_string);
    g_free (sdp_string);

    gst_webrtc_session_description_free (offer);
}


void
on_negotiation_needed_cb (GstElement * webrtcbin, gpointer user_data)
{
    GstPromise *promise;
    ReceiverEntry *receiver_entry = (ReceiverEntry *) user_data;

    g_print ("Creating negotiation offer\n");

    promise = gst_promise_new_with_change_func (on_offer_created_cb,
                                                (gpointer) receiver_entry, NULL);
    g_signal_emit_by_name (G_OBJECT (webrtcbin), "create-offer", NULL, promise);
}


void
on_ice_candidate_cb (G_GNUC_UNUSED GstElement * webrtcbin, guint mline_index,
                     gchar * candidate, gpointer user_data)
{
    JsonObject *ice_json;
    JsonObject *ice_data_json;
    gchar *json_string;
    ReceiverEntry *receiver_entry = (ReceiverEntry *) user_data;

    ice_json = json_object_new ();
    json_object_set_string_member (ice_json, "type", "ice");

    ice_data_json = json_object_new ();
    json_object_set_int_member (ice_data_json, "sdpMLineIndex", mline_index);
    json_object_set_string_member (ice_data_json, "candidate", candidate);
    json_object_set_object_member (ice_json, "data", ice_data_json);

    json_string = get_string_from_json_object (ice_json);
    json_object_unref (ice_json);

    soup_websocket_connection_send_text (receiver_entry->connection, json_string);
    g_free (json_string);
}


void soup_websocket_message_cb (G_GNUC_UNUSED SoupWebsocketConnection * connection,
                           SoupWebsocketDataType data_type, GBytes * message, gpointer user_data) {

    gsize size;
    gchar *data;
    gchar *data_string;
    const gchar *type_string;
    JsonNode *root_json;
    JsonObject *root_json_object;
    JsonObject *data_json_object;
    JsonParser *json_parser = NULL;
    ReceiverEntry *receiver_entry = (ReceiverEntry *) user_data;

    switch (data_type) {
        case SOUP_WEBSOCKET_DATA_BINARY:
            g_error ("Received unknown binary message, ignoring\n");
            g_bytes_unref (message);
            return;

        case SOUP_WEBSOCKET_DATA_TEXT:
            data = static_cast<gchar *>(g_bytes_unref_to_data(message, &size));
            /* Convert to NULL-terminated string */
            data_string = g_strndup (data, size);
            g_free (data);
            if(strstr(data_string, "COMMAND")) {
                std::cout << "Client message: " << data_string << "\n";
                if (strstr(data_string, "COMMAND_MOVE_LEFT")) {
                    rear_sight_processor->on_move_left_processor();
                    rear_sight_processor->set_new_frame_param();
                } else if (strstr(data_string, "COMMAND_MOVE_RIGHT")) {
                    rear_sight_processor->on_move_right_processor();
                    rear_sight_processor->set_new_frame_param();
                } else if (strstr(data_string, "COMMAND_MOVE_UP")) {
                    rear_sight_processor->on_move_up_processor();
                    rear_sight_processor->set_new_frame_param();
                } else if (strstr(data_string, "COMMAND_MOVE_DOWN")) {
                    rear_sight_processor->on_move_down_processor();
                    rear_sight_processor->set_new_frame_param();
                } else if (strstr(data_string, "COMMAND_ZOOM_MINUS")) {
                    rear_sight_processor->on_zoom_minus_processor();
                    rear_sight_processor->set_new_frame_param();
                } else if (strstr(data_string, "COMMAND_ZOOM_PLUS")) {
                    rear_sight_processor->on_zoom_plus_processor();
                    rear_sight_processor->set_new_frame_param();
                    std::cout << "Old frame size: w:" << frame_param->CROPPED_WIDTH
                        << "  h:" << frame_param->CROPPED_HEIGHT << "\n >>>>>>>>>>>>>>>>>>>>>>\n\n";
                    std::cout
                        << "New frame size: w:" << frame_param->CROPPED_WIDTH << "  h:" << frame_param->CROPPED_HEIGHT << "\n >>>>>>>>>>>>>>>>>>>>>>\n\n";
                }
                return;
            }
            break;

        default:
            break;
    }
    std::cout << "hey\n";
    json_parser = json_parser_new ();
    if (!json_parser_load_from_data (json_parser, data_string, -1, NULL))
        goto unknown_message;

    std::cout << "hey 1\n";
    root_json = json_parser_get_root (json_parser);
    if (!JSON_NODE_HOLDS_OBJECT (root_json))
        goto unknown_message;

    root_json_object = json_node_get_object (root_json);
    std::cout << "hey 2\n";

    if (!json_object_has_member (root_json_object, "type")) {
        g_error ("Received message without type field\n");
        goto cleanup;
    }
    type_string = json_object_get_string_member (root_json_object, "type");
    std::cout << "hey 3\n";
    if (!json_object_has_member (root_json_object, "data")) {
        g_error ("Received message without data field\n");
        goto cleanup;
    }
    data_json_object = json_object_get_object_member (root_json_object, "data");

    if (g_strcmp0 (type_string, "sdp") == 0) {
        const gchar *sdp_type_string;
        const gchar *sdp_string;
        GstPromise *promise;
        GstSDPMessage *sdp;
        GstWebRTCSessionDescription *answer;
        int ret;

        if (!json_object_has_member (data_json_object, "type")) {
            g_error ("Received SDP message without type field\n");
            goto cleanup;
        }
        sdp_type_string = json_object_get_string_member (data_json_object, "type");

        if (g_strcmp0 (sdp_type_string, "answer") != 0) {
            g_error ("Expected SDP message type \"answer\", got \"%s\"\n",
                     sdp_type_string);
            goto cleanup;
        }

        if (!json_object_has_member (data_json_object, "sdp")) {
            g_error ("Received SDP message without SDP string\n");
            goto cleanup;
        }
        sdp_string = json_object_get_string_member (data_json_object, "sdp");

        g_print ("Received SDP:\n%s\n", sdp_string);

        ret = gst_sdp_message_new (&sdp);
        g_assert_cmphex (ret, ==, GST_SDP_OK);

        ret =
                gst_sdp_message_parse_buffer ((guint8 *) sdp_string,
                                              strlen (sdp_string), sdp);
        if (ret != GST_SDP_OK) {
            g_error ("Could not parse SDP string\n");
            goto cleanup;
        }

        answer = gst_webrtc_session_description_new (GST_WEBRTC_SDP_TYPE_ANSWER,
                                                     sdp);
        g_assert_nonnull (answer);

        promise = gst_promise_new ();
        g_signal_emit_by_name (receiver_entry->webrtcbin, "set-remote-description",
                               answer, promise);
        gst_promise_interrupt (promise);
        gst_promise_unref (promise);
        gst_webrtc_session_description_free (answer);
    } else if (g_strcmp0 (type_string, "ice") == 0) {
        guint mline_index;
        const gchar *candidate_string;

        if (!json_object_has_member (data_json_object, "sdpMLineIndex")) {
            g_error ("Received ICE message without mline index\n");
            goto cleanup;
        }
        mline_index =
                json_object_get_int_member (data_json_object, "sdpMLineIndex");

        if (!json_object_has_member (data_json_object, "candidate")) {
            g_error ("Received ICE message without ICE candidate string\n");
            goto cleanup;
        }
        candidate_string = json_object_get_string_member (data_json_object,
                                                          "candidate");

        g_print ("Received ICE candidate with mline index %u; candidate: %s\n",
                 mline_index, candidate_string);

        g_signal_emit_by_name (receiver_entry->webrtcbin, "add-ice-candidate",
                               mline_index, candidate_string);
    } else
        goto unknown_message;

    cleanup:
    if (json_parser != NULL)
        g_object_unref (G_OBJECT (json_parser));
    g_free (data_string);
    return;

    unknown_message:
    g_error ("Unknown message \"%s\", ignoring", data_string);
    goto cleanup;
}


void
soup_websocket_closed_cb (SoupWebsocketConnection * connection,
                          gpointer user_data)
{
    GHashTable *receiver_entry_table = (GHashTable *) user_data;
    g_hash_table_remove (receiver_entry_table, connection);
    g_print ("Closed websocket connection %p\n", (gpointer) connection);
}


void soup_http_handler (G_GNUC_UNUSED SoupServer * soup_server,
                   SoupMessage * message, const char *path, G_GNUC_UNUSED GHashTable * query,
                   G_GNUC_UNUSED SoupClientContext * client_context,
                   G_GNUC_UNUSED gpointer user_data) {
    SoupBuffer *soup_buffer;

    if ((g_strcmp0 (path, "/") != 0) && (g_strcmp0 (path, "/index.html") != 0)) {
        soup_message_set_status (message, SOUP_STATUS_NOT_FOUND);
        return;
    }

    soup_buffer =
            soup_buffer_new (SOUP_MEMORY_STATIC, html_source, strlen (html_source));

    soup_message_headers_set_content_type (message->response_headers, "text/html",
                                           NULL);
    soup_message_body_append_buffer (message->response_body, soup_buffer);
    soup_buffer_free (soup_buffer);

    soup_message_set_status (message, SOUP_STATUS_OK);
}


void soup_websocket_handler (G_GNUC_UNUSED SoupServer * server,
                        SoupWebsocketConnection * connection, G_GNUC_UNUSED const char *path,
                        G_GNUC_UNUSED SoupClientContext * client_context, gpointer user_data) {
    ReceiverEntry *receiver_entry;
    GHashTable *receiver_entry_table = (GHashTable *) user_data;

    g_print ("Processing new websocket connection %p", (gpointer) connection);

    g_signal_connect (G_OBJECT (connection), "closed",
                      G_CALLBACK (soup_websocket_closed_cb), (gpointer) receiver_entry_table);

    receiver_entry = create_receiver_entry (connection);
    g_hash_table_replace (receiver_entry_table, connection, receiver_entry);
}


static gchar *
get_string_from_json_object (JsonObject * object)
{
    JsonNode *root;
    JsonGenerator *generator;
    gchar *text;

    /* Make it the root node */
    root = json_node_init_object (json_node_alloc (), object);
    generator = json_generator_new ();
    json_generator_set_root (generator, root);
    text = json_generator_to_data (generator, NULL);

    /* Release everything */
    g_object_unref (generator);
    json_node_free (root);
    return text;
}

#ifdef G_OS_UNIX
gboolean
exit_sighandler (gpointer user_data)
{
    g_print ("Caught signal, stopping mainloop\n");
    GMainLoop *mainloop = (GMainLoop *) user_data;
    g_main_loop_quit (mainloop);
    return TRUE;
}
#endif

int main (int argc, char *argv[]) {
    std::ifstream html_file("web_src/index.html");
    std::string html_str((std::istreambuf_iterator<char>(html_file)),
                         std::istreambuf_iterator<char>());

    html_source = html_str.c_str();

    GMainLoop *mainloop;
    SoupServer *soup_server;
    GHashTable *receiver_entry_table;

    setlocale (LC_ALL, "");
    gst_init (&argc, &argv);

    receiver_entry_table =
            g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL,
                                   destroy_receiver_entry);

    mainloop = g_main_loop_new (NULL, FALSE);
    g_assert (mainloop != NULL);

#ifdef G_OS_UNIX
    g_unix_signal_add (SIGINT, exit_sighandler, mainloop);
    g_unix_signal_add (SIGTERM, exit_sighandler, mainloop);
#endif

    soup_server =
            soup_server_new (SOUP_SERVER_SERVER_HEADER, "webrtc-soup-server", NULL);
    soup_server_add_handler (soup_server, "/", soup_http_handler, NULL, NULL);
    soup_server_add_websocket_handler (soup_server, "/ws", NULL, NULL,
                                       soup_websocket_handler, (gpointer) receiver_entry_table, NULL);
    soup_server_listen_all (soup_server, SOUP_HTTP_PORT,
                            (SoupServerListenOptions) 0, NULL);


    g_print ("WebRTC page link: http://127.0.0.1:%d/\n", (gint) SOUP_HTTP_PORT);

    g_main_loop_run (mainloop);

    g_object_unref (G_OBJECT (soup_server));
    g_hash_table_destroy (receiver_entry_table);
    g_main_loop_unref (mainloop);

    gst_deinit();

    return 0;
}
