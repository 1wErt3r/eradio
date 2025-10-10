#include <Elementary.h>
#include <Emotion.h>
#include <Ecore_Con.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

typedef struct _Station
{
   const char *name;
   const char *url;
   const char *favicon;
} Station;

typedef struct _AppData
{
   Evas_Object *win;
   Evas_Object *list;
   Evas_Object *emotion;
   Evas_Object *search_entry;
   Evas_Object *search_hoversel;
   Eina_List *stations;
} AppData;

static void _win_del_cb(void *data, Evas_Object *obj, void *event_info);
static void _list_item_selected_cb(void *data, Evas_Object *obj, void *event_info);
static void _play_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _pause_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _stop_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _search_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);

static void
_win_del_cb(void *data, Evas_Object *obj, void *event_info)
{
   elm_exit();
}

static void
_list_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   Elm_Object_Item *it = event_info;
   Station *st = elm_object_item_data_get(it);

   emotion_object_file_set(ad->emotion, st->url);
   emotion_object_play_set(ad->emotion, EINA_TRUE);
}



static void
_play_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   emotion_object_play_set(ad->emotion, EINA_TRUE);
}

static void
_pause_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   emotion_object_play_set(ad->emotion, EINA_FALSE);
}

static Eina_Bool _url_progress_cb(void *data, int type, void *event_info);
static Eina_Bool _url_complete_cb(void *data, int type, void *event_info);

typedef struct _Download_Context
{
   AppData *ad;
   xmlParserCtxtPtr ctxt;
} Download_Context;

static void
_search_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   const char *search_term = elm_object_text_get(ad->search_entry);
   const char *search_type = elm_object_text_get(ad->search_hoversel);
   char url_str[1024];
   Ecore_Con_Url *url;
   Download_Context *d_ctx;

   if (!search_term || !search_term[0]) return;

   d_ctx = calloc(1, sizeof(Download_Context));
   d_ctx->ad = ad;

   snprintf(url_str, sizeof(url_str), "http://de2.api.radio-browser.info/xml/stations/search?%s=%s",
            search_type, search_term);

   url = ecore_con_url_new(url_str);
   ecore_con_url_data_set(url, d_ctx);
   ecore_con_url_get(url);
}

static Eina_Bool
_url_progress_cb(void *data, int type, void *event_info)
{
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_url_data_cb(void *data, int type, void *event_info)
{
    Ecore_Con_Event_Url_Data *url_data = event_info;
    Download_Context *d_ctx = ecore_con_url_data_get(url_data->url_con);

    if (!d_ctx->ctxt)
      {
         d_ctx->ctxt = xmlCreatePushParserCtxt(NULL, NULL,
                                               (const char *)url_data->data, url_data->size,
                                               "noname.xml");
      }
    else
      xmlParseChunk(d_ctx->ctxt, (const char *)url_data->data, url_data->size, 0);

    return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_url_complete_cb(void *data, int type, void *event_info)
{
    Ecore_Con_Event_Url_Complete *ev = event_info;
    Download_Context *d_ctx = ecore_con_url_data_get(ev->url_con);
    AppData *ad;
    xmlDocPtr doc;
    xmlXPathContextPtr xpathCtx;
    xmlXPathObjectPtr xpathObj;

    if (!d_ctx) return ECORE_CALLBACK_PASS_ON;

    ad = d_ctx->ad;

    if (!d_ctx->ctxt)
      {
         // In case of empty reply, the data callback is not called
         free(d_ctx);
         ecore_con_url_free(ev->url_con);
         return ECORE_CALLBACK_PASS_ON;
      }

    xmlParseChunk(d_ctx->ctxt, "", 0, 1);
    doc = d_ctx->ctxt->myDoc;
    xmlFreeParserCtxt(d_ctx->ctxt);
    free(d_ctx);

    if (doc == NULL)
    {
        printf("Error: could not parse XML\n");
        ecore_con_url_free(ev->url_con);
        return ECORE_CALLBACK_PASS_ON;
    }


    xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL)
    {
        printf("Error: could not create new XPath context\n");
        xmlFreeDoc(doc);
        ecore_con_url_free(ev->url_con);
        return ECORE_CALLBACK_PASS_ON;
    }

    xpathObj = xmlXPathEvalExpression((xmlChar *)"//station", xpathCtx);
    if (xpathObj == NULL)
    {
        printf("Error: could not evaluate xpath expression\n");
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        ecore_con_url_free(ev->url_con);
        return ECORE_CALLBACK_PASS_ON;
    }

    elm_list_clear(ad->list);

    for (int i = 0; i < xpathObj->nodesetval->nodeNr; i++)
    {
        xmlNodePtr cur = xpathObj->nodesetval->nodeTab[i];
        Station *st = calloc(1, sizeof(Station));
        st->name = eina_stringshare_add((const char *)xmlGetProp(cur, (xmlChar *)"name"));
        st->url = eina_stringshare_add((const char *)xmlGetProp(cur, (xmlChar *)"url_resolved"));
        st->favicon = eina_stringshare_add((const char *)xmlGetProp(cur, (xmlChar *)"favicon"));
        ad->stations = eina_list_append(ad->stations, st);

        Evas_Object *ic = elm_icon_add(ad->win);
        elm_icon_standard_set(ic, st->favicon);
        elm_list_item_append(ad->list, st->name, ic, NULL, NULL, st);
    }
    elm_list_go(ad->list);

    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
    ecore_con_url_free(ev->url_con);

    return ECORE_CALLBACK_PASS_ON;
}

static void
_stop_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   emotion_object_play_set(ad->emotion, EINA_FALSE);
   emotion_object_position_set(ad->emotion, 0.0);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   AppData ad = {0};

   ecore_con_init();

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   ad.win = elm_win_add(NULL, "efl-internet-radio", ELM_WIN_BASIC);
   elm_win_title_set(ad.win, "EFL Internet Radio");
   elm_win_autodel_set(ad.win, EINA_TRUE);
   evas_object_smart_callback_add(ad.win, "delete,request", _win_del_cb, &ad);

   ecore_event_handler_add(ECORE_CON_EVENT_URL_DATA, _url_data_cb, &ad);
   ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_complete_cb, &ad);

   Evas_Object *box = elm_box_add(ad.win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(ad.win, box);
   evas_object_show(box);

   Evas_Object *search_hbox = elm_box_add(ad.win);
   elm_box_horizontal_set(search_hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(search_hbox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(search_hbox, EVAS_HINT_FILL, 0);
   elm_box_pack_end(box, search_hbox);
   evas_object_show(search_hbox);

   ad.search_entry = elm_entry_add(ad.win);
   elm_entry_single_line_set(ad.search_entry, EINA_TRUE);
   elm_entry_scrollable_set(ad.search_entry, EINA_TRUE);
   evas_object_size_hint_weight_set(ad.search_entry, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(ad.search_entry, EVAS_HINT_FILL, 0);
   elm_object_part_text_set(ad.search_entry, "guide", "Search for stations...");
   elm_box_pack_end(search_hbox, ad.search_entry);
   evas_object_show(ad.search_entry);

   ad.search_hoversel = elm_hoversel_add(ad.win);
   elm_hoversel_hover_parent_set(ad.search_hoversel, ad.win);
   elm_object_text_set(ad.search_hoversel, "name");
   elm_hoversel_item_add(ad.search_hoversel, "name", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(ad.search_hoversel, "country", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(ad.search_hoversel, "language", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(ad.search_hoversel, "tag", NULL, ELM_ICON_NONE, NULL, NULL);
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
   evas_object_size_hint_weight_set(ad.emotion, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ad.emotion, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, ad.emotion);
   evas_object_show(ad.emotion);

   Evas_Object *controls_hbox = elm_box_add(ad.win);
   elm_box_horizontal_set(controls_hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(controls_hbox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(controls_hbox, EVAS_HINT_FILL, 0);
   elm_box_pack_end(box, controls_hbox);
   evas_object_show(controls_hbox);

   Evas_Object *play_btn = elm_button_add(ad.win);
   elm_object_text_set(play_btn, "Play");
   elm_box_pack_end(controls_hbox, play_btn);
   evas_object_show(play_btn);

   Evas_Object *pause_btn = elm_button_add(ad.win);
   elm_object_text_set(pause_btn, "Pause");
   elm_box_pack_end(controls_hbox, pause_btn);
   evas_object_show(pause_btn);

   Evas_Object *stop_btn = elm_button_add(ad.win);
   elm_object_text_set(stop_btn, "Stop");
   elm_box_pack_end(controls_hbox, stop_btn);
   evas_object_show(stop_btn);

   evas_object_smart_callback_add(ad.list, "selected", _list_item_selected_cb, &ad);
   evas_object_smart_callback_add(play_btn, "clicked", _play_btn_clicked_cb, &ad);
   evas_object_smart_callback_add(pause_btn, "clicked", _pause_btn_clicked_cb, &ad);
   evas_object_smart_callback_add(stop_btn, "clicked", _stop_btn_clicked_cb, &ad);
   evas_object_smart_callback_add(search_btn, "clicked", _search_btn_clicked_cb, &ad);

   evas_object_show(ad.win);

   elm_run();
   ecore_con_shutdown();
   return 0;
}
ELM_MAIN()
