#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "favorites.h"

typedef struct {
    char *key;   // uuid or url
    char *uuid;
    char *url;
    char *name;
    char *favicon;
} FavEntry;

static void _fav_entry_free(void *data)
{
    FavEntry *e = data;
    if (!e) return;
    free(e->key);
    free(e->uuid);
    free(e->url);
    free(e->name);
    free(e->favicon);
    free(e);
}

static char *
_favorites_dir_path(void)
{
    const char *home = getenv("HOME");
    if (!home) return NULL;
    size_t len = strlen(home) + strlen("/.config/eradio") + 1;
    char *p = malloc(len);
    if (!p) return NULL;
    snprintf(p, len, "%s/.config/eradio", home);
    return p;
}

static char *
_favorites_file_path(void)
{
    const char *home = getenv("HOME");
    if (!home) return NULL;
    size_t len = strlen(home) + strlen("/.config/eradio/favorites.xml") + 1;
    char *p = malloc(len);
    if (!p) return NULL;
    snprintf(p, len, "%s/.config/eradio/favorites.xml", home);
    return p;
}

static Eina_Bool
_ensure_dir_exists(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0)
        return S_ISDIR(st.st_mode) ? EINA_TRUE : EINA_FALSE;

    if (errno == ENOENT)
    {
        if (mkdir(path, 0700) == 0)
            return EINA_TRUE;
    }
    return EINA_FALSE;
}

void
favorites_init(AppData *ad)
{
    if (!ad->favorites)
        ad->favorites = eina_hash_string_superfast_new(_fav_entry_free);
}

static void
_favorites_hash_add_entry(AppData *ad, const char *key, const char *uuid, const char *url, const char *name, const char *favicon)
{
    if (!key || !key[0]) return;
    FavEntry *e = calloc(1, sizeof(FavEntry));
    if (!e) return;

    e->key = key ? strdup(key) : NULL;
    e->uuid = uuid ? strdup(uuid) : NULL;
    e->url = url ? strdup(url) : NULL;
    e->name = name ? strdup(name) : NULL;
    e->favicon = favicon ? strdup(favicon) : NULL;

    // Add to hash - if this fails, we need to clean up manually
    if (!eina_hash_add(ad->favorites, e->key, e))
    {
        // Manual cleanup since hash add failed
        free(e->key);
        free(e->uuid);
        free(e->url);
        free(e->name);
        free(e->favicon);
        free(e);
    }
}

void
favorites_shutdown(AppData *ad)
{
    if (!ad->favorites) return;
    eina_hash_free(ad->favorites);
    ad->favorites = NULL;
}

void
favorites_load(AppData *ad)
{
    char *dir = _favorites_dir_path();
    char *path = _favorites_file_path();
    if (!dir || !path)
        goto end;

    _ensure_dir_exists(dir);

    xmlDocPtr doc = xmlParseFile(path);
    if (!doc)
        goto end;

    xmlNodePtr root = xmlDocGetRootElement(doc);
    for (xmlNodePtr cur = root ? root->children : NULL; cur; cur = cur->next)
    {
        if (cur->type != XML_ELEMENT_NODE) continue;
        if (xmlStrcmp(cur->name, (xmlChar *)"station") != 0) continue;

        xmlChar *uuid = xmlGetProp(cur, (xmlChar *)"uuid");
        xmlChar *url = xmlGetProp(cur, (xmlChar *)"url");
        xmlChar *name = xmlGetProp(cur, (xmlChar *)"name");
        xmlChar *favicon = xmlGetProp(cur, (xmlChar *)"favicon");
        const char *key = NULL;
        if (uuid && uuid[0]) key = (const char *)uuid;
        else if (url && url[0]) key = (const char *)url;
        _favorites_hash_add_entry(ad, key, (const char *)uuid, (const char *)url, (const char *)name, (const char *)favicon);
        if (uuid) xmlFree(uuid);
        if (url) xmlFree(url);
        if (name) xmlFree(name);
        if (favicon) xmlFree(favicon);
    }

    xmlFreeDoc(doc);

end:
    if (dir) free(dir);
    if (path) free(path);
}

void
favorites_apply_to_stations(AppData *ad)
{
    Eina_List *l;
    Station *st;
    EINA_LIST_FOREACH(ad->stations, l, st)
    {
        const char *key = st->stationuuid ? st->stationuuid : st->url;
        st->favorite = EINA_FALSE;
        if (key && eina_hash_find(ad->favorites, key))
            st->favorite = EINA_TRUE;
    }
}

static Eina_Bool _favorites_save_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
    xmlNodePtr root = fdata;
    FavEntry *e = data;
    if (!e) return EINA_TRUE;
    if ((!e->uuid || !e->uuid[0]) && (!e->url || !e->url[0]))
        return EINA_TRUE;
    xmlNodePtr sn = xmlNewChild(root, NULL, (xmlChar *)"station", NULL);
    if (e->uuid && e->uuid[0]) xmlNewProp(sn, (xmlChar *)"uuid", (xmlChar *)e->uuid);
    if (e->name && e->name[0]) xmlNewProp(sn, (xmlChar *)"name", (xmlChar *)e->name);
    if (e->url && e->url[0]) xmlNewProp(sn, (xmlChar *)"url", (xmlChar *)e->url);
    if (e->favicon && e->favicon[0]) xmlNewProp(sn, (xmlChar *)"favicon", (xmlChar *)e->favicon);
    return EINA_TRUE;
}

void favorites_save(AppData *ad)
{
    char *dir = _favorites_dir_path();
    char *path = _favorites_file_path();
    if (!dir || !path)
        goto end;

    if (!_ensure_dir_exists(dir))
        goto end;

    size_t tmplen = strlen(path) + 5;
    char *tmp = malloc(tmplen);
    if (!tmp) goto end;
    snprintf(tmp, tmplen, "%s.tmp", path);

    xmlDocPtr doc = xmlNewDoc((xmlChar *)"1.0");
    xmlNodePtr root = xmlNewNode(NULL, (xmlChar *)"favorites");
    xmlNewProp(root, (xmlChar *)"version", (xmlChar *)"1");
    xmlDocSetRootElement(doc, root);

    eina_hash_foreach(ad->favorites, _favorites_save_cb, root);

    int xml_save_result = xmlSaveFormatFileEnc(tmp, doc, "UTF-8", 1);
    xmlFreeDoc(doc);

    if (xml_save_result == -1) {
        // XML save failed - could be disk space, permissions, etc.
        if (ad->statusbar) {
            elm_object_text_set(ad->statusbar, "Error: Could not save favorites (disk full?)");
        }
        unlink(tmp); // Clean up the temp file
        free(tmp);
        goto end;
    }

    if (rename(tmp, path) == -1) {
        // Rename failed - could be filesystem issues
        if (ad->statusbar) {
            elm_object_text_set(ad->statusbar, "Error: Could not save favorites (filesystem error)");
        }
        unlink(tmp); // Clean up the temp file
        free(tmp);
        goto end;
    }

    free(tmp);

    // Success - provide feedback to user
    if (ad->statusbar) {
        elm_object_text_set(ad->statusbar, "Favorites saved successfully");
    }

end:
    if (dir) free(dir);
    if (path) free(path);
}

void favorites_set(AppData *ad, Station *st, Eina_Bool on)
{
    if (!ad || !st) return;
    const char *key = st->stationuuid ? st->stationuuid : st->url;
    if (!key || !key[0]) return;
    if (on)
    {
        FavEntry *existing = eina_hash_find(ad->favorites, key);
        if (existing)
        {
            // Update metadata
            if (st->name) {
                free(existing->name);
                existing->name = strdup(st->name);
            }
            if (st->url) {
                free(existing->url);
                existing->url = strdup(st->url);
            }
            if (st->stationuuid) {
                free(existing->uuid);
                existing->uuid = strdup(st->stationuuid);
            }
            if (st->favicon) {
                free(existing->favicon);
                existing->favicon = strdup(st->favicon);
            }
        }
        else
        {
            _favorites_hash_add_entry(ad, key, st->stationuuid, st->url, st->name, st->favicon);
        }
    }
    else
    {
        FavEntry *existing = eina_hash_find(ad->favorites, key);
        if (existing)
            eina_hash_del(ad->favorites, key, existing);
    }
}

static Eina_Bool _favorites_rebuild_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
    AppData *ad = fdata;
    FavEntry *e = data;
    if (!ad || !e) return EINA_TRUE;
    Station *st = calloc(1, sizeof(Station));
    if (!st) return EINA_TRUE;
    if (e->name) st->name = eina_stringshare_add(e->name);
    if (e->url) st->url = eina_stringshare_add(e->url);
    if (e->uuid) st->stationuuid = eina_stringshare_add(e->uuid);
    if (e->favicon) st->favicon = eina_stringshare_add(e->favicon);
    st->favorite = EINA_TRUE;

    // Add to list - check for failure to avoid memory leak
    Eina_List *new_list = eina_list_append(ad->favorites_stations, st);
    if (!new_list)
    {
        // List append failed, clean up the station
        eina_stringshare_del(st->name);
        eina_stringshare_del(st->url);
        eina_stringshare_del(st->stationuuid);
        eina_stringshare_del(st->favicon);
        free(st);
        return EINA_TRUE;
    }
    ad->favorites_stations = new_list;
    return EINA_TRUE;
}

void favorites_rebuild_station_list(AppData *ad)
{
    Station *st;
    EINA_LIST_FREE(ad->favorites_stations, st)
    {
        eina_stringshare_del(st->name);
        eina_stringshare_del(st->url);
        eina_stringshare_del(st->favicon);
        eina_stringshare_del(st->stationuuid);
        free(st);
    }
    ad->favorites_stations = NULL;

    eina_hash_foreach(ad->favorites, _favorites_rebuild_cb, ad);
}