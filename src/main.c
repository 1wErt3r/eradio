#include <Elementary.h>
#include <Ecore_Evas.h>
#include <Emotion.h>
#include <Ecore_Con.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

typedef struct _Station
{
   const char *name;
   const char *url;
   const char *favicon;
   const char *stationuuid;
} Station;

typedef struct _AppData
{
   Evas_Object *win;
   Evas_Object *list;
   Evas_Object *emotion;
   Evas_Object *search_entry;
   Evas_Object *search_hoversel;
   Evas_Object *play_pause_btn;
   Eina_List *stations;
   Eina_Bool playing;
} AppData;

typedef enum _Download_Type
{
   DOWNLOAD_TYPE_STATIONS,
   DOWNLOAD_TYPE_ICON
} Download_Type;

typedef struct _Download_Context
{
   Download_Type type;
   AppData *ad;
} Download_Context;

typedef struct _Station_Download_Context
{
   Download_Context base;
   xmlParserCtxtPtr ctxt;
} Station_Download_Context;

typedef struct _Icon_Download_Context
{
   Download_Context base;
   Elm_Object_Item *list_item;
   Eina_Binbuf *image_data;
} Icon_Download_Context;


static void _win_del_cb(void *data, Evas_Object *obj, void *event_info);
static void _list_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
static void _play_pause_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _stop_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _hoversel_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
static void _search_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _search_entry_activated_cb(void *data, Evas_Object *obj, void *event_info);
static Eina_Bool _url_data_cb(void *data, int type, void *event_info);
static Eina_Bool _url_complete_cb(void *data, int type, void *event_info);


static void
_win_del_cb(void *data, Evas_Object *obj, void *event_info)
{
   elm_exit();
}

static void
_hoversel_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Object_Item *it = event_info;
    const char *label = elm_object_item_text_get(it);
    elm_object_text_set(obj, label);
}

static void
_station_click_counter_request(Station *st)
{
   char url_str[1024];
   Ecore_Con_Url *url;

   if (!st || !st->stationuuid) return;

   snprintf(url_str, sizeof(url_str), "http://de2.api.radio-browser.info/xml/url/%s",
            st->stationuuid);

   url = ecore_con_url_new(url_str);
   ecore_con_url_additional_header_add(url, "User-Agent", "eradio/1.0");
   ecore_con_url_get(url);
}

static void
_list_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   Elm_Object_Item *it = event_info;
   Station *st = elm_object_item_data_get(it);
   Evas_Object *ic;

   if (!st) return;

   _station_click_counter_request(st);

   if (st->url && st->url[0])
     {
        emotion_object_file_set(ad->emotion, st->url);
        emotion_object_play_set(ad->emotion, EINA_TRUE);
        ad->playing = EINA_TRUE;
        ic = elm_object_part_content_get(ad->play_pause_btn, "icon");
        elm_icon_standard_set(ic, "media-playback-pause");
     }
}

static void
_play_pause_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   Evas_Object *ic;

   ad->playing = !ad->playing;
   emotion_object_play_set(ad->emotion, ad->playing);

   ic = elm_object_part_content_get(ad->play_pause_btn, "icon");
   if (ad->playing)
     elm_icon_standard_set(ic, "media-playback-pause");
   else
     elm_icon_standard_set(ic, "media-playback-start");
}

static void
_search_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   const char *search_term = elm_object_text_get(ad->search_entry);
   const char *search_type = elm_object_text_get(ad->search_hoversel);
   char url_str[1024];
   Ecore_Con_Url *url;
   Station_Download_Context *d_ctx;

   if (!search_term || !search_term[0]) return;

   d_ctx = calloc(1, sizeof(Station_Download_Context));
   d_ctx->base.type = DOWNLOAD_TYPE_STATIONS;
   d_ctx->base.ad = ad;

   snprintf(url_str, sizeof(url_str), "http://de2.api.radio-browser.info/xml/stations/search?%s=%s",
            search_type, search_term);

   url = ecore_con_url_new(url_str);
   ecore_con_url_additional_header_add(url, "User-Agent", "eradio/1.0");
   ecore_con_url_data_set(url, d_ctx);
   ecore_con_url_get(url);
}

static void
_search_entry_activated_cb(void *data, Evas_Object *obj, void *event_info)
{
   _search_btn_clicked_cb(data, obj, event_info);
}

static void
_handle_station_list_data(Ecore_Con_Event_Url_Data *url_data)
{
    Station_Download_Context *d_ctx = ecore_con_url_data_get(url_data->url_con);

    if (!d_ctx) return;

    if (!d_ctx->ctxt)
      {
         d_ctx->ctxt = xmlCreatePushParserCtxt(NULL, NULL,
                                               (const char *)url_data->data, url_data->size,
                                               "noname.xml");
      }
    else
      xmlParseChunk(d_ctx->ctxt, (const char *)url_data->data, url_data->size, 0);
}

static void
_handle_icon_data(Ecore_Con_Event_Url_Data *url_data)
{
    Icon_Download_Context *icon_ctx = ecore_con_url_data_get(url_data->url_con);

    if (!icon_ctx) return;

    if (!icon_ctx->image_data)
      icon_ctx->image_data = eina_binbuf_new();

    eina_binbuf_append_length(icon_ctx->image_data, (const unsigned char *)url_data->data, url_data->size);
}

static void
_handle_icon_complete(Ecore_Con_Event_Url_Complete *ev)
{
    Icon_Download_Context *icon_ctx = ecore_con_url_data_get(ev->url_con);

    if (icon_ctx && icon_ctx->image_data)
    {
        Elm_Object_Item *it = icon_ctx->list_item;
        Evas_Object *icon = elm_object_item_part_content_get(it, "start");
        const char *ext = strrchr(ecore_con_url_url_get(ev->url_con), '.');
        if (ext) ext++;

        elm_image_memfile_set(icon, eina_binbuf_string_get(icon_ctx->image_data), eina_binbuf_length_get(icon_ctx->image_data), (char *)ext, NULL);
    }

    if (icon_ctx)
    {
       if (icon_ctx->image_data) eina_binbuf_free(icon_ctx->image_data);
       free(icon_ctx);
    }
}

static void
_handle_station_list_complete(Ecore_Con_Event_Url_Complete *ev)
{
    Station_Download_Context *d_ctx = ecore_con_url_data_get(ev->url_con);
    AppData *ad;
    xmlDocPtr doc;
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;

    if (!d_ctx) return;

    ad = d_ctx->base.ad;

    if (!d_ctx->ctxt)
      {
         free(d_ctx);
         return;
      }

    xmlParseChunk(d_ctx->ctxt, "", 0, 1);
    doc = d_ctx->ctxt->myDoc;
    xmlFreeParserCtxt(d_ctx->ctxt);
    free(d_ctx);

    if (doc == NULL)
    {
        printf("Error: could not parse XML\n");
        return;
    }

    xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL)
    {
        printf("Error: could not create new XPath context\n");
        xmlFreeDoc(doc);
        return;
    }

    xpathObj = xmlXPathEvalExpression((xmlChar *)"//station", xpathCtx);
    if (xpathObj == NULL)
    {
        printf("Error: could not evaluate xpath expression\n");
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        return;
    }

    elm_list_clear(ad->list);

    for (int i = 0; i < xpathObj->nodesetval->nodeNr; i++)
    {
        xmlNodePtr cur = xpathObj->nodesetval->nodeTab[i];
        Station *st = calloc(1, sizeof(Station));
        xmlChar *prop;

        prop = xmlGetProp(cur, (xmlChar *)"name");
        if (prop)
          {
             st->name = eina_stringshare_add((const char *)prop);
             xmlFree(prop);
          }

        prop = xmlGetProp(cur, (xmlChar *)"url_resolved");
        if (prop)
          {
             st->url = eina_stringshare_add((const char *)prop);
             xmlFree(prop);
          }

        prop = xmlGetProp(cur, (xmlChar *)"favicon");
        if (prop)
          {
             st->favicon = eina_stringshare_add((const char *)prop);
             xmlFree(prop);
          }

        prop = xmlGetProp(cur, (xmlChar *)"stationuuid");
        if (prop)
          {
             st->stationuuid = eina_stringshare_add((const char *)prop);
             xmlFree(prop);
          }
        ad->stations = eina_list_append(ad->stations, st);

        Evas_Object *icon = elm_icon_add(ad->win);
        elm_icon_standard_set(icon, "radio");
        Elm_Object_Item *li = elm_list_item_append(ad->list, st->name, icon, NULL, _list_item_selected_cb, ad);
        elm_object_item_data_set(li, st);

        if (st->favicon && st->favicon[0])
          {
             Ecore_Con_Url *url = ecore_con_url_new(st->favicon);
             Icon_Download_Context *icon_ctx = calloc(1, sizeof(Icon_Download_Context));
             icon_ctx->base.type = DOWNLOAD_TYPE_ICON;
             icon_ctx->base.ad = ad;
             icon_ctx->list_item = li;
             ecore_con_url_data_set(url, icon_ctx);
             ecore_con_url_get(url);
          }
    }
    elm_list_go(ad->list);

    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
}

static Eina_Bool
_url_data_cb(void *data, int type, void *event_info)
{
    Ecore_Con_Event_Url_Data *url_data = event_info;
    Download_Context *ctx = ecore_con_url_data_get(url_data->url_con);

    if (!ctx) return ECORE_CALLBACK_PASS_ON;

    if (ctx->type == DOWNLOAD_TYPE_STATIONS)
      _handle_station_list_data(url_data);
    else if (ctx->type == DOWNLOAD_TYPE_ICON)
      _handle_icon_data(url_data);

    return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_url_complete_cb(void *data, int type, void *event_info)
{
    Ecore_Con_Event_Url_Complete *ev = event_info;
    Download_Context *ctx = ecore_con_url_data_get(ev->url_con);

    if (!ctx)
      {
         ecore_con_url_free(ev->url_con);
         return ECORE_CALLBACK_PASS_ON;
      }

    if (ctx->type == DOWNLOAD_TYPE_STATIONS)
      _handle_station_list_complete(ev);
    else if (ctx->type == DOWNLOAD_TYPE_ICON)
      _handle_icon_complete(ev);

    ecore_con_url_free(ev->url_con);
    return ECORE_CALLBACK_PASS_ON;
}

static void
_stop_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   Evas_Object *ic;

   emotion_object_play_set(ad->emotion, EINA_FALSE);
   emotion_object_position_set(ad->emotion, 0.0);
   ad->playing = EINA_FALSE;
   ic = elm_object_part_content_get(ad->play_pause_btn, "icon");
   elm_icon_standard_set(ic, "media-playback-start");
}

static void
_app_exit_cb(void *data, Evas_Object *obj, void *event_info)
{
    Ecore_Evas *ee = data;
    if (ee) ecore_evas_free(ee);
}

#include <stdlib.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   AppData ad = {0};
   Ecore_Evas *ee = ecore_evas_new(NULL, 0, 0, 0, 0, NULL);
   if (ee)
     {
        int dpi = 0;
        ecore_evas_screen_dpi_get(ee, NULL, &dpi);
        if (dpi >= 192)
          elm_config_scale_set(4.0);
     }


   ecore_con_init();

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   ad.win = elm_win_add(NULL, "eradio", ELM_WIN_BASIC);
   elm_win_title_set(ad.win, "eradio");
   elm_win_autodel_set(ad.win, EINA_TRUE);
   evas_object_smart_callback_add(ad.win, "delete,request", _win_del_cb, &ad);
   if (ee) evas_object_smart_callback_add(ad.win, "delete,request", _app_exit_cb, ee);

   ecore_event_handler_add(ECORE_CON_EVENT_URL_DATA, _url_data_cb, &ad);
   ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_complete_cb, &ad);

   Evas_Object *box = elm_box_add(ad.win);
   elm_box_padding_set(box, 0, 10);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(ad.win, box);
   evas_object_show(box);

   Evas_Object *search_hbox = elm_box_add(ad.win);
   elm_box_horizontal_set(search_hbox, EINA_TRUE);
   elm_box_padding_set(search_hbox, 10, 0);
   evas_object_size_hint_weight_set(search_hbox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(search_hbox, EVAS_HINT_FILL, 0);
   elm_box_pack_end(box, search_hbox);
   evas_object_show(search_hbox);

   ad.search_entry = elm_entry_add(ad.win);
   elm_entry_single_line_set(ad.search_entry, EINA_TRUE);
   elm_entry_scrollable_set(ad.search_entry, EINA_TRUE);
   evas_object_size_hint_weight_set(ad.search_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ad.search_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_part_text_set(ad.search_entry, "guide", "Search for stations...");
   elm_box_pack_end(search_hbox, ad.search_entry);
   evas_object_show(ad.search_entry);
   elm_object_focus_set(ad.search_entry, EINA_TRUE);

   ad.search_hoversel = elm_hoversel_add(ad.win);
   elm_hoversel_hover_parent_set(ad.search_hoversel, ad.win);
   elm_object_text_set(ad.search_hoversel, "name");
   elm_hoversel_item_add(ad.search_hoversel, "name", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad.search_hoversel, "country", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad.search_hoversel, "language", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_hoversel_item_add(ad.search_hoversel, "tag", NULL, ELM_ICON_NONE, _hoversel_item_selected_cb, NULL);
   elm_box_pack_end(search_hbox, ad.search_hoversel);
   evas_object_show(ad.search_hoversel);

   Evas_Object *search_btn = elm_button_add(ad.win);
   elm_object_text_set(search_btn, "Search");
   elm_box_pack_end(search_hbox, search_btn);
   evas_object_show(search_btn);

   ad.list = elm_list_add(ad.win);
   evas_object_size_hint_weight_set(ad.list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ad.list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, ad.list);
   evas_object_show(ad.list);

   ad.emotion = emotion_object_add(ad.win);

   Evas_Object *controls_hbox = elm_box_add(ad.win);
   elm_box_horizontal_set(controls_hbox, EINA_TRUE);
   elm_box_padding_set(controls_hbox, 10, 0);
   elm_box_align_set(controls_hbox, 0.5, 1.0);
   evas_object_size_hint_weight_set(controls_hbox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(controls_hbox, EVAS_HINT_FILL, 0);
   elm_box_pack_end(box, controls_hbox);
   evas_object_show(controls_hbox);

   Evas_Object *ic;

   ad.play_pause_btn = elm_button_add(ad.win);
   ic = elm_icon_add(ad.play_pause_btn);
   elm_icon_standard_set(ic, "media-playback-start");
   evas_object_size_hint_min_set(ic, 40, 40);
   elm_object_part_content_set(ad.play_pause_btn, "icon", ic);
   elm_box_pack_end(controls_hbox, ad.play_pause_btn);
   evas_object_show(ad.play_pause_btn);

   Evas_Object *stop_btn = elm_button_add(ad.win);
   ic = elm_icon_add(stop_btn);
   elm_icon_standard_set(ic, "media-playback-stop");
   evas_object_size_hint_min_set(ic, 40, 40);
   elm_object_part_content_set(stop_btn, "icon", ic);
   elm_box_pack_end(controls_hbox, stop_btn);
   evas_object_show(stop_btn);

   evas_object_smart_callback_add(ad.play_pause_btn, "clicked", _play_pause_btn_clicked_cb, &ad);
   evas_object_smart_callback_add(stop_btn, "clicked", _stop_btn_clicked_cb, &ad);
   evas_object_smart_callback_add(search_btn, "clicked", _search_btn_clicked_cb, &ad);
   evas_object_smart_callback_add(ad.search_entry, "activated", _search_entry_activated_cb, &ad);
   evas_object_smart_callback_add(ad.list, "selected", _list_item_selected_cb, &ad);

   evas_object_resize(ad.win, 400, 600);
   evas_object_show(ad.win);

   elm_run();
   ecore_con_shutdown();
   return 0;
}
ELM_MAIN()
