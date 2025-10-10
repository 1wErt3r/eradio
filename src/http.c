#include <Ecore_Con.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "http.h"
#include "station_list.h"

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

static Eina_Bool _url_data_cb(void *data, int type, void *event_info);
static Eina_Bool _url_complete_cb(void *data, int type, void *event_info);

void
http_init(AppData *ad)
{
   ecore_con_init();
   ecore_event_handler_add(ECORE_CON_EVENT_URL_DATA, _url_data_cb, ad);
   ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_complete_cb, ad);
}

void
http_shutdown(void)
{
   ecore_con_shutdown();
}

void
http_search_stations(AppData *ad, const char *search_term, const char *search_type)
{
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

void
http_download_icon(AppData *ad, Elm_Object_Item *list_item, const char *url_str)
{
    Ecore_Con_Url *url = ecore_con_url_new(url_str);
    Icon_Download_Context *icon_ctx = calloc(1, sizeof(Icon_Download_Context));
    icon_ctx->base.type = DOWNLOAD_TYPE_ICON;
    icon_ctx->base.ad = ad;
    icon_ctx->list_item = list_item;
    ecore_con_url_data_set(url, icon_ctx);
    ecore_con_url_get(url);
}

void
_search_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   AppData *ad = data;
   const char *search_term = elm_object_text_get(ad->search_entry);
   const char *search_type = elm_object_text_get(ad->search_hoversel);
   http_search_stations(ad, search_term, search_type);
}

void
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

    eina_list_free(ad->stations);
    ad->stations = NULL;

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
    }

    station_list_populate(ad, ad->stations);

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
