#include <Ecore_Con.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <Ecore_File.h>

#include "http.h"
#include "station_list.h"
#include "favorites.h"
#include "ui.h" // Include ui.h for ui_set_load_more_button_visibility

typedef enum _Download_Type
{
   DOWNLOAD_TYPE_STATIONS,
   DOWNLOAD_TYPE_ICON,
   DOWNLOAD_TYPE_COUNTER
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
   Eina_List *servers;     // list of const char* hostnames
   Eina_List *current;     // current server node
   char search_type[64];
   char search_term[512];
   char order[64];
   Eina_Bool reverse;
   int offset;
   int limit;
   Eina_Bool new_search;
} Station_Download_Context;

typedef struct _Icon_Download_Context
{
   Download_Context base;
   Elm_Object_Item *list_item;
   Eina_Binbuf *image_data;
} Icon_Download_Context;

typedef struct _Counter_Download_Context
{
   Download_Context base;
   Eina_List *servers;     // list of const char* hostnames
   Eina_List *current;     // current server node
   char stationuuid[128];
} Counter_Download_Context;

static Eina_Bool _url_data_cb(void *data, int type, void *event_info);
static Eina_Bool _url_complete_cb(void *data, int type, void *event_info);

static void _refresh_api_servers(AppData *ad);
static void _randomize_servers(AppData *ad);
static const char *_primary_server(AppData *ad);
static void _populate_station_request(Station_Download_Context *d_ctx, AppData *ad, const char *search_type, const char *search_term, const char *order, Eina_Bool reverse);
static void _issue_station_request(Ecore_Con_Url **url_out, Station_Download_Context *d_ctx);
static void _retry_next_server_station(Ecore_Con_Url *old_url, Station_Download_Context *d_ctx);
static void _populate_counter_request(Counter_Download_Context *c_ctx, AppData *ad, const char *uuid);
static void _issue_counter_request(Ecore_Con_Url **url_out, Counter_Download_Context *c_ctx);
static void _retry_next_server_counter(Ecore_Con_Url *old_url, Counter_Download_Context *c_ctx);

void
http_init(AppData *ad)
{
   ecore_con_init();
   ecore_event_handler_add(ECORE_CON_EVENT_URL_DATA, _url_data_cb, ad);
   ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_complete_cb, ad);

   _refresh_api_servers(ad);
   _randomize_servers(ad);
   ad->api_selected = _primary_server(ad);
}

void
http_shutdown(void)
{
   ecore_con_shutdown();
}

void
http_search_stations(AppData *ad, const char *search_term, const char *search_type, const char *order, Eina_Bool reverse, int offset, int limit, Eina_Bool new_search)
{
   Ecore_Con_Url *url;
   Station_Download_Context *d_ctx;

   if (!search_term || !search_term[0]) return;

   d_ctx = calloc(1, sizeof(Station_Download_Context));
   d_ctx->base.type = DOWNLOAD_TYPE_STATIONS;
   d_ctx->base.ad = ad;
   d_ctx->offset = offset;
   d_ctx->limit = limit;
   d_ctx->new_search = new_search;
   _populate_station_request(d_ctx, ad, search_type, search_term, order, reverse);
   _issue_station_request(&url, d_ctx);
   ecore_con_url_additional_header_add(url, "User-Agent", "eradio/1.0");
   ecore_con_url_data_set(url, d_ctx);
   ecore_con_url_get(url);
}

void
http_station_click_counter(AppData *ad, const char *uuid)
{
   if (!uuid || !uuid[0]) return;

   Counter_Download_Context *c_ctx = calloc(1, sizeof(Counter_Download_Context));
   c_ctx->base.type = DOWNLOAD_TYPE_COUNTER;
   c_ctx->base.ad = ad;
   _populate_counter_request(c_ctx, ad, uuid);

   Ecore_Con_Url *url;
   _issue_counter_request(&url, c_ctx);
   ecore_con_url_additional_header_add(url, "User-Agent", "eradio/1.0");
   ecore_con_url_data_set(url, c_ctx);
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
_search_btn_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   AppData *ad = data;
   const char *search_term = elm_object_text_get(ad->search_entry);
   const char *search_type = elm_object_text_get(ad->search_hoversel);
   const char *order = elm_object_text_get(ad->sort_hoversel);
   Eina_Bool reverse = elm_check_state_get(ad->reverse_check);
   ad->search_offset = 0; // Reset offset for a new search
   http_search_stations(ad, search_term, search_type, order, reverse, ad->search_offset, 100, EINA_TRUE);
}

void
_search_entry_activated_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
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
        Station *st = elm_object_item_data_get(it);
        Evas_Object *icon = elm_object_item_part_content_get(it, "start");
        const char *ext = strrchr(ecore_con_url_url_get(ev->url_con), '.');
        if (ext) ext++;

        if (st && st->stationuuid)
        {
            char cache_dir[PATH_MAX];
            const char *home = getenv("HOME");
            snprintf(cache_dir, sizeof(cache_dir), "%s/.cache/eradio/favicons", home);
            ecore_file_mkpath(cache_dir);

            char cache_path[PATH_MAX];
            snprintf(cache_path, sizeof(cache_path), "%s/%s", cache_dir, st->stationuuid);

            FILE *f = fopen(cache_path, "wb");
            if (f)
            {
                fwrite(eina_binbuf_string_get(icon_ctx->image_data), 1, eina_binbuf_length_get(icon_ctx->image_data), f);
                fclose(f);
            }
        }

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

    if (ev->status != 200)
      {
         printf("HTTP error %d on %s, trying fallback...\n", ev->status, ecore_con_url_url_get(ev->url_con));
         _retry_next_server_station(ev->url_con, d_ctx);
         return;
      }

    if (!d_ctx->ctxt)
      {
         printf("Error: no parser context; retrying next server...\n");
         _retry_next_server_station(ev->url_con, d_ctx);
         return;
      }

    xmlParseChunk(d_ctx->ctxt, "", 0, 1);
    doc = d_ctx->ctxt->myDoc;
    xmlFreeParserCtxt(d_ctx->ctxt);

    if (doc == NULL)
    {
        printf("Error: could not parse XML; trying fallback...\n");
        _retry_next_server_station(ev->url_con, ecore_con_url_data_get(ev->url_con));
        return;
    }

    xpathCtx = xmlXPathNewContext(doc);
    if (xpathCtx == NULL)
    {
        printf("Error: could not create new XPath context\n");
        xmlFreeDoc(doc);
        free(d_ctx);
        return;
    }

    xpathObj = xmlXPathEvalExpression((xmlChar *)"//station", xpathCtx);
    if (xpathObj == NULL)
    {
        printf("Error: could not evaluate xpath expression\n");
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);
        free(d_ctx);
        return;
    }

    if (d_ctx->new_search)
    {
        Station *st;
        EINA_LIST_FREE(ad->stations, st)
        {
            eina_stringshare_del(st->name);
            eina_stringshare_del(st->url);
            eina_stringshare_del(st->favicon);
            eina_stringshare_del(st->stationuuid);
            free(st);
        }
        ad->stations = NULL;
    }

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

    favorites_apply_to_stations(ad);
    if (ad->view_mode == VIEW_SEARCH)
      station_list_populate(ad, ad->stations, d_ctx->new_search);

    // Determine if "Load More" button should be visible
    if (d_ctx->new_search)
    {
        if (xpathObj->nodesetval->nodeNr > 0)
            ui_set_load_more_button_visibility(ad, EINA_TRUE);
        else
            ui_set_load_more_button_visibility(ad, EINA_FALSE);
    }
    else
    {
        if (xpathObj->nodesetval->nodeNr == 0)
            ui_set_load_more_button_visibility(ad, EINA_FALSE);
    }

    free(d_ctx);

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
    else if (ctx->type == DOWNLOAD_TYPE_COUNTER)
      {
         if (ev->status != 200)
           {
              printf("HTTP error %d on %s, trying fallback counter...\n", ev->status, ecore_con_url_url_get(ev->url_con));
              _retry_next_server_counter(ev->url_con, (Counter_Download_Context *)ctx);
              return ECORE_CALLBACK_PASS_ON;
           }
         // No further action needed; just free context
         free(ctx);
      }

    ecore_con_url_free(ev->url_con);
    return ECORE_CALLBACK_PASS_ON;
}

// -------- Helper functions for API server discovery & selection ---------

static void _add_unique_server(AppData *ad, const char *hostname)
{
   if (!hostname || !hostname[0]) return;
   // ensure uniqueness
   Eina_List *l;
   const char *h;
   EINA_LIST_FOREACH(ad->api_servers, l, h)
   {
      if (strcmp(h, hostname) == 0) return;
   }
   ad->api_servers = eina_list_append(ad->api_servers, eina_stringshare_add(hostname));
}

static void _refresh_api_servers(AppData *ad)
{
   // Resolve all.api.radio-browser.info to get all server IPs
   struct addrinfo hints = {0}, *res = NULL, *p;
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   int ret = getaddrinfo("all.api.radio-browser.info", NULL, &hints, &res);
   if (ret != 0)
   {
      printf("DNS lookup failed: %s\n", gai_strerror(ret));
      return;
   }

   for (p = res; p != NULL; p = p->ai_next)
   {
      char host[NI_MAXHOST];
      // Reverse DNS to get human-readable server names
      int r = getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NAMEREQD);
      if (r == 0)
      {
         _add_unique_server(ad, host);
      }
      else
      {
         // Fallback: if reverse lookup fails, use the numeric address
         char addrstr[INET6_ADDRSTRLEN] = {0};
         void *addr = NULL;
         if (p->ai_family == AF_INET)
           addr = &((struct sockaddr_in *)p->ai_addr)->sin_addr;
         else if (p->ai_family == AF_INET6)
           addr = &((struct sockaddr_in6 *)p->ai_addr)->sin6_addr;
         if (addr)
         {
            inet_ntop(p->ai_family, addr, addrstr, sizeof(addrstr));
            _add_unique_server(ad, addrstr);
         }
      }
   }

   freeaddrinfo(res);
}

static void _randomize_servers(AppData *ad)
{
   // Fisher-Yates shuffle on an array copy of the list
   int n = eina_list_count(ad->api_servers);
   if (n <= 1) return;
   char **arr = calloc(n, sizeof(char *));
   Eina_List *l; const char *h; int i = 0;
   EINA_LIST_FOREACH(ad->api_servers, l, h) arr[i++] = (char *)h;
   srand((unsigned int)time(NULL));
   for (int j = n - 1; j > 0; j--)
   {
      int k = rand() % (j + 1);
      char *tmp = arr[j]; arr[j] = arr[k]; arr[k] = tmp;
   }
   // rebuild list in randomized order
   eina_list_free(ad->api_servers);
   ad->api_servers = NULL;
   for (int j = 0; j < n; j++) ad->api_servers = eina_list_append(ad->api_servers, arr[j]);
   free(arr);
}

static const char *_primary_server(AppData *ad)
{
   if (!ad->api_servers) return NULL;
   return eina_list_data_get(ad->api_servers);
}

static void _prepend_selected_as_primary(Eina_List **list, const char *selected)
{
   if (!selected || !list) return;
   Eina_List *l; const char *h; Eina_List *node = NULL;
   EINA_LIST_FOREACH(*list, l, h)
   {
      if ((h == selected) || (h && selected && strcmp(h, selected) == 0))
      {
         node = l;
         break;
      }
   }
   if (node)
   {
      const char *data = node->data;
      *list = eina_list_remove_list(*list, node);
      *list = eina_list_prepend(*list, data);
   }
}

static void _populate_station_request(Station_Download_Context *d_ctx, AppData *ad, const char *search_type, const char *search_term, const char *order, Eina_Bool reverse)
{
   strncpy(d_ctx->search_type, search_type ? search_type : "name", sizeof(d_ctx->search_type) - 1);
   strncpy(d_ctx->search_term, search_term ? search_term : "", sizeof(d_ctx->search_term) - 1);
   strncpy(d_ctx->order, order ? order : "name", sizeof(d_ctx->order) - 1);
   d_ctx->reverse = reverse;
   d_ctx->servers = eina_list_clone(ad->api_servers);
   _prepend_selected_as_primary(&d_ctx->servers, ad->api_selected);
   d_ctx->current = d_ctx->servers; // start at primary
}

static void _issue_station_request(Ecore_Con_Url **url_out, Station_Download_Context *d_ctx)
{
   const char *server = d_ctx->current ? (const char *)d_ctx->current->data : NULL;
   char url_str[2048];
   char query_params[1024] = {0};

   // Start with the base search params
   snprintf(query_params, sizeof(query_params), "%s=%s", d_ctx->search_type, d_ctx->search_term);

   // Add sorting and pagination params
   char other_params[512];
   snprintf(other_params, sizeof(other_params), "&offset=%d&limit=%d&order=%s&reverse=%s",
            d_ctx->offset, d_ctx->limit, d_ctx->order, d_ctx->reverse ? "true" : "false");

   strncat(query_params, other_params, sizeof(query_params) - strlen(query_params) - 1);

   if (server)
      snprintf(url_str, sizeof(url_str), "http://%s/xml/stations/search?%s", server, query_params);
   else
      snprintf(url_str, sizeof(url_str), "http://de2.api.radio-browser.info/xml/stations/search?%s", query_params);

   printf("Request URL: %s\n", url_str);
   *url_out = ecore_con_url_new(url_str);
}

static void _retry_next_server_station(Ecore_Con_Url *old_url, Station_Download_Context *d_ctx)
{
   if (d_ctx->current && d_ctx->current->next)
   {
      d_ctx->current = d_ctx->current->next;
      Ecore_Con_Url *new_url;
      _issue_station_request(&new_url, d_ctx);
      ecore_con_url_additional_header_add(new_url, "User-Agent", "eradio/1.0");
      ecore_con_url_data_set(new_url, d_ctx);
      ecore_con_url_get(new_url);
      ecore_con_url_free(old_url);
   }
   else
   {
      printf("All servers exhausted; failing request.\n");
      ecore_con_url_free(old_url);
      // free context and notify UI of failure
      if (d_ctx->ctxt)
      {
         xmlFreeParserCtxt(d_ctx->ctxt);
         d_ctx->ctxt = NULL;
      }
      free(d_ctx);
   }
}

static void _populate_counter_request(Counter_Download_Context *c_ctx, AppData *ad, const char *uuid)
{
   strncpy(c_ctx->stationuuid, uuid, sizeof(c_ctx->stationuuid) - 1);
   c_ctx->servers = eina_list_clone(ad->api_servers);
   _prepend_selected_as_primary(&c_ctx->servers, ad->api_selected);
   c_ctx->current = c_ctx->servers;
}

static void _issue_counter_request(Ecore_Con_Url **url_out, Counter_Download_Context *c_ctx)
{
   const char *server = c_ctx->current ? (const char *)c_ctx->current->data : NULL;
   char url_str[1024];
   if (server)
      snprintf(url_str, sizeof(url_str), "http://%s/xml/url/%s", server, c_ctx->stationuuid);
   else
      snprintf(url_str, sizeof(url_str), "http://de2.api.radio-browser.info/xml/url/%s", c_ctx->stationuuid);

   printf("Counter Request URL: %s\n", url_str);
   *url_out = ecore_con_url_new(url_str);
}

static void _retry_next_server_counter(Ecore_Con_Url *old_url, Counter_Download_Context *c_ctx)
{
   if (c_ctx->current && c_ctx->current->next)
   {
      c_ctx->current = c_ctx->current->next;
      Ecore_Con_Url *new_url;
      _issue_counter_request(&new_url, c_ctx);
      ecore_con_url_additional_header_add(new_url, "User-Agent", "eradio/1.0");
      ecore_con_url_data_set(new_url, c_ctx);
      ecore_con_url_get(new_url);
      ecore_con_url_free(old_url);
   }
   else
   {
      printf("All servers exhausted for counter; giving up.\n");
      ecore_con_url_free(old_url);
      free(c_ctx);
   }
}
