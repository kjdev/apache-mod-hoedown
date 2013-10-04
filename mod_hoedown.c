/*
**  mod_hoedown.c -- Apache hoedown module
**
**  Then activate it in Apache's httpd.conf file:
**
**    # httpd.conf
**    LoadModule hoedown_module modules/mod_hoedown.so
**
**    HoedownDefaultPage    /var/www/html/README.md
**    HoedownDirectoryIndex index.md
**    # Style file
**    HoedownStylePath      /var/www/html/style
**    HoedownStyleDefault   default
**    HoedownStyleExtension .html
**    # Class attribute
**    HoedownClassUl   ul-list
**    HoedownClassOl   ol-list
**    HoedownClassTask task-list
**    # Toc options
**    HoedownTocHeader   '<div class="toc">'
**    HoedownTocFooter   '</div>'
**    HoedownTocStarting 2
**    HoedownTocNesting  6
**    # Raw options
**    HoedownRaw On
**    # Extension options
**    HoedownExtNoIntraEmphasis     Off
**    HoedownExtTables              Off
**    HoedownExtFencedCode          Off
**    HoedownExtAutolink            Off
**    HoedownExtStrikethrough       Off
**    HoedownExtUnderline           Off
**    HoedownExtSpaceHeaders        Off
**    HoedownExtSuperscript         Off
**    HoedownExtLaxSpacing          Off
**    HoedownExtDisableIndentedCode Off
**    HoedownExtHighlight           Off
**    HoedownExtFootnotes           Off
**    HoedownExtQuote               Off
**    HoedownExtSpecialAttribute    Off
**    # Html Render options
**    HoedownRenderSkipHtml      Off
**    HoedownRenderSkipStyle     Off
**    HoedownRenderSkipImages    Off
**    HoedownRenderSkipLinks     Off
**    HoedownRenderExpandTabs    Off
**    HoedownRenderSafelink      Off
**    HoedownRenderToc           Off
**    HoedownRenderHardWrap      Off
**    HoedownRenderUseXhtml      Off
**    HoedownRenderEscape        Off
**    HoedownRenderPrettify      Off
**    HoedownRenderUseTaskList   Off
**    HoedownRenderSkipEol       Off
**    HoedownRenderTocSkipEscape Off
**
**    <Location /hoedown>
**      # AddHandler hoedown .md
**      SetHandler hoedown
**    </Location>
*/

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

/* httpd */
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_main.h"
#include "http_log.h"
#include "util_script.h"
#include "ap_config.h"
#include "apr_fnmatch.h"
#include "apr_strings.h"
#include "apr_hash.h"

/* apreq2 */
#include "apreq2/apreq_module_apache2.h"

#ifdef HOEDOWN_URL_SUPPORT
/* libcurl */
#include "curl/curl.h"
#endif

/* hoedown */
#include "hoedown/src/markdown.h"
#include "hoedown/src/html.h"
#include "hoedown/src/buffer.h"

#define HOEDOWN_READ_UNIT       1024
#define HOEDOWN_OUTPUT_UNIT     64
#define HOEDOWN_CURL_TIMEOUT    30
#define HOEDOWN_TITLE_DEFAULT   "Markdown"
#define HOEDOWN_CONTENT_TYPE    "text/html"
#define HOEDOWN_TAG             "<body*>"
#define HOEDOWN_STYLE_EXT       ".html"
#define HOEDOWN_DIRECTORY_INDEX "index.md"
#define HOEDOWN_TOC_STARTING     2
#define HOEDOWN_TOC_NESTING      6

typedef struct {
    char *default_page;
    char *directory_index;
    struct {
        char *path;
        char *name;
        char *ext;
    } style;
    struct {
        char *ul;
        char *ol;
        char *task;
    } class;
    struct {
        int starting;
        int nesting;
        char *header;
        char *footer;
    } toc;
    int raw;
    unsigned int extensions;
    unsigned int html;
} hoedown_config_rec;

module AP_MODULE_DECLARE_DATA hoedown_module;


static int
output_style_header(request_rec *r, apr_file_t *fp)
{
    char buf[HUGE_STRING_LEN];
    char *lower = NULL;

    while (apr_file_gets(buf, HUGE_STRING_LEN, fp) == APR_SUCCESS) {
        ap_rputs(buf, r);

        lower = apr_pstrdup(r->pool, buf);
        ap_str_tolower(lower);
        if (apr_fnmatch("*"HOEDOWN_TAG"*", lower, APR_FNM_CASE_BLIND) == 0) {
            return 1;
        }
    }

    return 0;
}

static apr_file_t *
style_header(request_rec *r, hoedown_config_rec *cfg, char *filename)
{
    apr_status_t rc = -1;
    apr_file_t *fp = NULL;
    char *style_filepath = NULL;

    if (filename == NULL && cfg->style.name != NULL) {
        filename = cfg->style.name;
    }

    if (filename != NULL) {
        if (cfg->style.path == NULL) {
            ap_add_common_vars(r);
            cfg->style.path = (char *)apr_table_get(r->subprocess_env,
                                                    "DOCUMENT_ROOT");
        }

        style_filepath = apr_psprintf(r->pool, "%s/%s%s",
                                      cfg->style.path, filename, cfg->style.ext);

        rc = apr_file_open(&fp, style_filepath,
                           APR_READ | APR_BINARY | APR_XTHREAD,
                           APR_OS_DEFAULT, r->pool);
        if (rc == APR_SUCCESS) {
            if (output_style_header(r, fp) != 1) {
                apr_file_close(fp);
                fp = NULL;
            }
        } else {
            style_filepath = apr_psprintf(r->pool, "%s/%s%s",
                                          cfg->style.path,
                                          cfg->style.name,
                                          cfg->style.ext);

            rc = apr_file_open(&fp, style_filepath,
                               APR_READ | APR_BINARY | APR_XTHREAD,
                               APR_OS_DEFAULT, r->pool);
            if (rc == APR_SUCCESS) {
                if (output_style_header(r, fp) != 1) {
                    apr_file_close(fp);
                    fp = NULL;
                }
            }
        }
    }

    if (rc != APR_SUCCESS) {
        ap_rputs("<!DOCTYPE html>\n<html>\n", r);
        ap_rputs("<head><title>"HOEDOWN_TITLE_DEFAULT"</title></head>\n", r);
        ap_rputs("<body>\n", r);
    }

    return fp;
}

static int
style_footer(request_rec *r, apr_file_t *fp) {
    char buf[HUGE_STRING_LEN];

    if (fp != NULL) {
        while (apr_file_gets(buf, HUGE_STRING_LEN, fp) == APR_SUCCESS) {
            ap_rputs(buf, r);
        }
        apr_file_close(fp);
    } else {
        ap_rputs("</body>\n</html>\n", r);
    }

    return 0;
}

static void
append_data(hoedown_buffer *ib, void *buffer, size_t size)
{
    size_t offset = 0;

    if (!ib || !buffer || size == 0) {
        return;
    }

    while (offset < size) {
        size_t bufsize = ib->asize - ib->size;
        if (size >= (bufsize + offset)) {
            memcpy(ib->data + ib->size, buffer + offset, bufsize);
            ib->size += bufsize;
            hoedown_buffer_grow(ib, ib->size + HOEDOWN_READ_UNIT);
            offset += bufsize;
        } else {
            bufsize = size - offset;
            if (bufsize > 0) {
                memcpy(ib->data + ib->size, buffer + offset, bufsize);
                ib->size += bufsize;
            }
            break;
        }
    }
}

#ifdef HOEDOWN_URL_SUPPORT
static size_t
append_url_data(void *buffer, size_t size, size_t nmemb, void *user)
{
    size_t segsize = size * nmemb;
    append_data((hoedown_buffer *)user, buffer, segsize);
    return segsize;
}
#endif

static int
append_page_data(request_rec *r, hoedown_config_rec *cfg,
                 hoedown_buffer *ib, char *name, int directory)
{
    apr_status_t rc = -1;
    apr_file_t *fp = NULL;
    apr_size_t read;
    char *filename = NULL;

    if (name == NULL) {
        if (!cfg->default_page) {
            return HTTP_NOT_FOUND;
        }
        filename = cfg->default_page;
    } else if (strlen(name) <= 0 ||
               memcmp(name + strlen(name) - 1, "/", 1) == 0) {
        if (!cfg->directory_index || !directory) {
            return HTTP_FORBIDDEN;
        }
        filename = apr_psprintf(r->pool, "%s%s", name, cfg->directory_index);
    } else {
        filename = name;
    }

    rc = apr_file_open(&fp, filename,
                       APR_READ | APR_BINARY | APR_XTHREAD, APR_OS_DEFAULT,
                       r->pool);
    if (rc != APR_SUCCESS || !fp) {
        switch (errno) {
            case ENOENT:
                return HTTP_NOT_FOUND;
            case EACCES:
                return HTTP_FORBIDDEN;
            default:
                break;
        }
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    do {
        rc = apr_file_read_full(fp, ib->data + ib->size, ib->asize - ib->size,
                                &read);
        if (read > 0) {
            ib->size += read;
            hoedown_buffer_grow(ib, ib->size + HOEDOWN_READ_UNIT);
        }
    } while (rc != APR_EOF);

    apr_file_close(fp);

    return APR_SUCCESS;
}

/* content handler */
static int
hoedown_handler(request_rec *r)
{
    int ret = -1;
    int directory = 1;
    apr_file_t *fp = NULL;
    char *style = NULL;
    char *url = NULL;
    char *text = NULL;
    char *raw = NULL;
    char *toc = NULL;
    int toc_starting = HOEDOWN_TOC_STARTING, toc_nesting = HOEDOWN_TOC_NESTING;
    apreq_handle_t *apreq;
    apr_table_t *params;

    hoedown_config_rec *cfg;

    /* hoedown: markdown */
    hoedown_buffer *ib, *ob;
    hoedown_callbacks callbacks;
    hoedown_html_renderopt options;
    struct hoedown_markdown *markdown;

    if (strcmp(r->handler, "hoedown")) {
        return DECLINED;
    }

    if (r->header_only) {
        return OK;
    }

    /* config */
    cfg = ap_get_module_config(r->per_dir_config, &hoedown_module);

    /* set contest type */
    r->content_type = HOEDOWN_CONTENT_TYPE;

    /* get parameter */
    apreq = apreq_handle_apache2(r);
    params = apreq_params(apreq, r->pool);
    if (params) {
        style = (char *)apreq_params_as_string(r->pool, params,
                                               "style", APREQ_JOIN_AS_IS);
#ifdef HOEDOWN_URL_SUPPORT
        url = (char *)apreq_params_as_string(r->pool, params,
                                             "url", APREQ_JOIN_AS_IS);
#endif
        if (cfg->raw != 0) {
            raw = (char *)apr_table_get(params, "raw");
        }
        if (cfg->html & HOEDOWN_HTML_TOC) {
            toc = (char *)apr_table_get(params, "toc");
        }
        if (r->method_number == M_POST) {
            text = (char *)apreq_params_as_string(r->pool, params,
                                                  "markdown", APREQ_JOIN_AS_IS);
        }
    }

    /* reading everything */
    ib = hoedown_buffer_new(HOEDOWN_READ_UNIT);
    hoedown_buffer_grow(ib, HOEDOWN_READ_UNIT);

    /* page */
    if (url || text) {
        directory = 0;
    }
    append_page_data(r, cfg, ib, r->filename, directory);

    /* text */
    if (text && strlen(text) > 0) {
        append_data(ib, text, strlen(text));
    }

#ifdef HOEDOWN_URL_SUPPORT
    /* url */
    if (url && strlen(url) > 0) {
        CURL *curl;

        curl = curl_easy_init();
        if (!curl) {
            return HTTP_INTERNAL_SERVER_ERROR;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);

        /* curl */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)ib);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append_url_data);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, HOEDOWN_CURL_TIMEOUT);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

        ret = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        /*
        if (ret != 0) {
            hoedown_buffer_free(ib);
            return HTTP_INTERNAL_SERVER_ERROR;
        }
        */
    }
#endif

    /* default page */
    if (ib->size == 0) {
        ret = append_page_data(r, cfg, ib, NULL, 0);
        if (ret != APR_SUCCESS) {
            hoedown_buffer_free(ib);
            return ret;
        }
    }

    /* default toc level */
    toc_starting = cfg->toc.starting;
    toc_nesting = cfg->toc.nesting;

    if (ib->size > 0) {
        if (cfg->raw != 0 && raw != NULL) {
            r->content_type = "text/plain";
            ap_rwrite(ib->data, ib->size, r);
            hoedown_buffer_free(ib);
            return OK;
        }

        /* output style header */
        fp = style_header(r, cfg, style);

        if (cfg->html & HOEDOWN_HTML_TOC) {
            /* toc */
            if (toc) {
                size_t len = strlen(toc);
                int n;

                if (len > 0) {
                    char *delim, *toc_b = NULL, *toc_e = NULL;
                    delim = strstr(toc, ":");
                    if (delim) {
                        int i = delim - toc;
                        toc_b = apr_pstrndup(r->pool, toc, i++);
                        n = atoi(toc_b);
                        if (n) {
                            toc_starting = n;
                        }

                        toc_e = apr_pstrndup(r->pool, toc + i, len - i);
                        n = atoi(toc_e);
                        if (n) {
                            toc_nesting = n;
                        }
                    } else {
                        n = atoi(toc);
                        if (n) {
                            toc_starting = n;
                        }
                    }
                }
            }

            ob = hoedown_buffer_new(HOEDOWN_OUTPUT_UNIT);

            hoedown_html_toc_renderer(&callbacks, &options, 0);

            options.flags = cfg->html;
            options.toc_data.starting_level = toc_starting;
            options.toc_data.nesting_level = toc_nesting;
            options.toc_data.header = cfg->toc.header;
            options.toc_data.footer = cfg->toc.footer;

            markdown = hoedown_markdown_new(cfg->extensions, 16,
                                            &callbacks, &options);

            hoedown_markdown_render(ob, ib->data, ib->size, markdown);
            hoedown_markdown_free(markdown);

            ap_rwrite(ob->data, ob->size, r);

            hoedown_buffer_free(ob);
        }

        /* performing markdown parsing */
        ob = hoedown_buffer_new(HOEDOWN_OUTPUT_UNIT);

        /* markdown render */
        hoedown_html_renderer(&callbacks, &options, 0, 0);

        options.flags = cfg->html;

        options.toc_data.starting_level = toc_starting;
        options.toc_data.nesting_level = toc_nesting;

        if ((options.flags & HOEDOWN_HTML_USE_TASK_LIST) && cfg->class.task) {
            options.class_attributes.task = cfg->class.task;
        }
        if (cfg->class.ol) {
            options.class_attributes.ol = cfg->class.ol;
        }
        if (cfg->class.ul) {
            options.class_attributes.ul = cfg->class.ul;
        }

        markdown = hoedown_markdown_new(cfg->extensions, 16,
                                        &callbacks, &options);

        hoedown_markdown_render(ob, ib->data, ib->size, markdown);
        hoedown_markdown_free(markdown);

        /* writing the result */
        ap_rwrite(ob->data, ob->size, r);

        /* cleanup */
        hoedown_buffer_free(ob);
    } else {
        /* output style header */
        fp = style_header(r, cfg, style);
    }

    /* cleanup */
    hoedown_buffer_free(ib);

    /* output style footer */
    style_footer(r, fp);

    return OK;
}

static void *
hoedown_create_dir_config(apr_pool_t *p, char *dir)
{
    hoedown_config_rec *cfg;

    cfg = apr_pcalloc(p, sizeof(hoedown_config_rec));

    memset(cfg, 0, sizeof(hoedown_config_rec));

    cfg->default_page = NULL;
    cfg->directory_index = HOEDOWN_DIRECTORY_INDEX;
    cfg->style.path = NULL;
    cfg->style.name = NULL;
    cfg->style.ext = HOEDOWN_STYLE_EXT;
    cfg->class.ul = NULL;
    cfg->class.ol = NULL;
    cfg->class.task = NULL;
    cfg->toc.starting = HOEDOWN_TOC_STARTING;
    cfg->toc.nesting = HOEDOWN_TOC_NESTING;
    cfg->toc.header = NULL;
    cfg->toc.footer = NULL;
    cfg->raw = 0;
    cfg->extensions = 0;
    cfg->html = 0;

    return (void *)cfg;
}

static void *
hoedown_merge_dir_config(apr_pool_t *p, void *base_conf, void *override_conf)
{
    hoedown_config_rec *cfg = apr_pcalloc(p, sizeof(hoedown_config_rec));
    hoedown_config_rec *base = (hoedown_config_rec *)base_conf;
    hoedown_config_rec *override = (hoedown_config_rec *)override_conf;

    if (override->default_page && strlen(override->default_page) > 0) {
        cfg->default_page = override->default_page;
    } else {
        cfg->default_page = base->default_page;
    }
    if (strcmp(override->directory_index, HOEDOWN_DIRECTORY_INDEX) != 0) {
        cfg->directory_index = override->directory_index;
    } else {
        cfg->directory_index = base->directory_index;
    }

    if (override->style.path && strlen(override->style.path) > 0) {
        cfg->style.path = override->style.path;
    } else {
        cfg->style.path = base->style.path;
    }
    if (override->style.name && strlen(override->style.name) > 0) {
        cfg->style.name = override->style.name;
    } else {
        cfg->style.name = base->style.name;
    }
    if (strcmp(override->style.ext, HOEDOWN_STYLE_EXT) != 0) {
        cfg->style.ext = override->style.ext;
    } else {
        cfg->style.ext = base->style.ext;
    }

    if (override->class.ul && strlen(override->class.ul) > 0) {
        cfg->class.ul = override->class.ul;
    } else {
        cfg->class.ul = base->class.ul;
    }
    if (override->class.ol && strlen(override->class.ol) > 0) {
        cfg->class.ol = override->class.ol;
    } else {
        cfg->class.ol = base->class.ol;
    }
    if (override->class.task && strlen(override->class.task) > 0) {
        cfg->class.task = override->class.task;
    } else {
        cfg->class.task = base->class.task;
    }

    if (override->toc.starting != HOEDOWN_TOC_STARTING) {
        cfg->toc.starting = override->toc.starting;
    } else {
        cfg->toc.starting = base->toc.starting;
    }
    if (override->toc.nesting != HOEDOWN_TOC_NESTING) {
        cfg->toc.nesting = override->toc.nesting;
    } else {
        cfg->toc.nesting = base->toc.nesting;
    }
    if (override->toc.header && strlen(override->toc.header) > 0) {
        cfg->toc.header = override->toc.header;
    } else {
        cfg->toc.header = base->toc.header;
    }
    if (override->toc.footer && strlen(override->toc.footer) > 0) {
        cfg->toc.footer = override->toc.footer;
    } else {
        cfg->toc.footer = base->toc.footer;
    }

    if (override->raw != 0) {
        cfg->raw = 1;
    } else {
        cfg->raw = base->raw;
    }

    if (override->extensions > 0) {
        cfg->extensions = override->extensions;
    } else {
        cfg->extensions = base->extensions;
    }
    if (override->html > 0) {
        cfg->html = override->html;
    } else {
        cfg->html = base->html;
    }

    return (void *)cfg;
}

#define HOEDOWN_SET_EXTENSIONS(_name, _ext)                                 \
static const char *                                                         \
hoedown_set_extensions_ ## _name(cmd_parms *parms, void *mconfig, int bool) \
{                                                                           \
    hoedown_config_rec *cfg = (hoedown_config_rec *)mconfig;                \
    if (bool != 0) {                                                        \
        cfg->extensions |= _ext;                                            \
    }                                                                       \
    return NULL;                                                            \
}

HOEDOWN_SET_EXTENSIONS(nointraemphasis, HOEDOWN_EXT_NO_INTRA_EMPHASIS);
HOEDOWN_SET_EXTENSIONS(tables, HOEDOWN_EXT_TABLES);
HOEDOWN_SET_EXTENSIONS(fencedcode, HOEDOWN_EXT_FENCED_CODE);
HOEDOWN_SET_EXTENSIONS(autolink, HOEDOWN_EXT_AUTOLINK);
HOEDOWN_SET_EXTENSIONS(strikethrough, HOEDOWN_EXT_STRIKETHROUGH);
HOEDOWN_SET_EXTENSIONS(underline, HOEDOWN_EXT_UNDERLINE);
HOEDOWN_SET_EXTENSIONS(spaceheaders, HOEDOWN_EXT_SPACE_HEADERS);
HOEDOWN_SET_EXTENSIONS(superscript, HOEDOWN_EXT_SUPERSCRIPT);
HOEDOWN_SET_EXTENSIONS(laxspacing, HOEDOWN_EXT_LAX_SPACING);
HOEDOWN_SET_EXTENSIONS(disableindentedcode, HOEDOWN_EXT_DISABLE_INDENTED_CODE);
HOEDOWN_SET_EXTENSIONS(highlight, HOEDOWN_EXT_HIGHLIGHT);
HOEDOWN_SET_EXTENSIONS(footnotes, HOEDOWN_EXT_FOOTNOTES);
HOEDOWN_SET_EXTENSIONS(quote, HOEDOWN_EXT_QUOTE);
HOEDOWN_SET_EXTENSIONS(specialattribute, HOEDOWN_EXT_SPECIAL_ATTRIBUTE);

#define HOEDOWN_SET_RENDER(_name, _ext)                                 \
static const char *                                                     \
hoedown_set_render_ ## _name(cmd_parms *parms, void *mconfig, int bool) \
{                                                                       \
    hoedown_config_rec *cfg = (hoedown_config_rec *)mconfig;            \
    if (bool != 0) {                                                    \
        cfg->html |= _ext;                                              \
    }                                                                   \
    return NULL;                                                        \
}

HOEDOWN_SET_RENDER(skiphtml, HOEDOWN_HTML_SKIP_HTML);
HOEDOWN_SET_RENDER(skipstyle, HOEDOWN_HTML_SKIP_STYLE);
HOEDOWN_SET_RENDER(skipimages, HOEDOWN_HTML_SKIP_IMAGES);
HOEDOWN_SET_RENDER(skiplinks, HOEDOWN_HTML_SKIP_LINKS);
HOEDOWN_SET_RENDER(expandtabs, HOEDOWN_HTML_EXPAND_TABS);
HOEDOWN_SET_RENDER(safelink, HOEDOWN_HTML_SAFELINK);
HOEDOWN_SET_RENDER(toc, HOEDOWN_HTML_TOC);
HOEDOWN_SET_RENDER(hardwrap, HOEDOWN_HTML_HARD_WRAP);
HOEDOWN_SET_RENDER(usexhtml, HOEDOWN_HTML_USE_XHTML);
HOEDOWN_SET_RENDER(escape, HOEDOWN_HTML_ESCAPE);
HOEDOWN_SET_RENDER(prettify, HOEDOWN_HTML_PRETTIFY);
HOEDOWN_SET_RENDER(usetasklist, HOEDOWN_HTML_USE_TASK_LIST);
HOEDOWN_SET_RENDER(skipeol, HOEDOWN_HTML_SKIP_EOL);
HOEDOWN_SET_RENDER(tocskipescape, HOEDOWN_HTML_SKIP_TOC_ESCAPE);

static const command_rec
hoedown_cmds[] = {
    AP_INIT_TAKE1("HoedownDefaultPage", ap_set_string_slot,
                  (void *)APR_OFFSETOF(hoedown_config_rec, default_page),
                  OR_ALL, "hoedown default page file"),
    AP_INIT_TAKE1("HoedownDirectoryIndex", ap_set_string_slot,
                  (void *)APR_OFFSETOF(hoedown_config_rec, directory_index),
                  OR_ALL, "hoedown directory index page"),
    /* Style file */
    AP_INIT_TAKE1("HoedownStylePath", ap_set_string_slot,
                  (void *)APR_OFFSETOF(hoedown_config_rec, style.path),
                  OR_ALL, "hoedown style path"),
    AP_INIT_TAKE1("HoedownStyleDefault", ap_set_string_slot,
                  (void *)APR_OFFSETOF(hoedown_config_rec, style.name),
                  OR_ALL, "hoedown default style file name"),
    AP_INIT_TAKE1("HoedownStyleExtension", ap_set_string_slot,
                  (void *)APR_OFFSETOF(hoedown_config_rec, style.ext),
                  OR_ALL, "hoedown default style file extension"),
    /* Class name */
    AP_INIT_TAKE1("HoedownClassUl", ap_set_string_slot,
                  (void *)APR_OFFSETOF(hoedown_config_rec, class.ul),
                  OR_ALL, "hoedown ul class attributes"),
    AP_INIT_TAKE1("HoedownClassOl", ap_set_string_slot,
                  (void *)APR_OFFSETOF(hoedown_config_rec, class.ol),
                  OR_ALL, "hoedown ol class attributes"),
    AP_INIT_TAKE1("HoedownClassTask", ap_set_string_slot,
                  (void *)APR_OFFSETOF(hoedown_config_rec, class.task),
                  OR_ALL, "hoedown task list class attributes"),
    /* Toc options */
    AP_INIT_TAKE1("HoedownTocHeader", ap_set_string_slot,
                  (void *)APR_OFFSETOF(hoedown_config_rec, toc.header),
                  OR_ALL, "hoedown toc header"),
    AP_INIT_TAKE1("HoedownTocFooter", ap_set_string_slot,
                  (void *)APR_OFFSETOF(hoedown_config_rec, toc.footer),
                  OR_ALL, "hoedown toc footer"),
    AP_INIT_TAKE1("HoedownTocStarting", ap_set_int_slot,
                  (void *)APR_OFFSETOF(hoedown_config_rec, toc.starting),
                  OR_ALL, "hoedown toc starting level"),
    AP_INIT_TAKE1("HoedownTocNesting", ap_set_int_slot,
                  (void *)APR_OFFSETOF(hoedown_config_rec, toc.nesting),
                  OR_ALL, "hoedown toc nesting level"),
    /* Raw options */
    AP_INIT_FLAG("HoedownRaw", ap_set_flag_slot,
                 (void *)APR_OFFSETOF(hoedown_config_rec, raw),
                 OR_ALL, "Enable hoedown raw support"),
    /* Markdown extension options */
    AP_INIT_FLAG("HoedownExtNoIntraEmphasis",
                 hoedown_set_extensions_nointraemphasis,
                 NULL, OR_ALL, "Enable hoedown extension NoIntraEmphasis"),
    AP_INIT_FLAG("HoedownExtTables",
                 hoedown_set_extensions_tables,
                 NULL, OR_ALL, "Enable hoedown extension Tables"),
    AP_INIT_FLAG("HoedownExtFencedCode",
                 hoedown_set_extensions_fencedcode,
                 NULL, OR_ALL, "Enable hoedown extension Fenced Code"),
    AP_INIT_FLAG("HoedownExtAutolink",
                 hoedown_set_extensions_autolink,
                 NULL, OR_ALL, "Enable hoedown extension Autolink"),
    AP_INIT_FLAG("HoedownExtStrikethrough",
                 hoedown_set_extensions_strikethrough,
                 NULL, OR_ALL, "Enable hoedown extension Strikethrough"),
    AP_INIT_FLAG("HoedownExtUnderline",
                 hoedown_set_extensions_underline,
                 NULL, OR_ALL, "Enable hoedown extension Underline"),
    AP_INIT_FLAG("HoedownExtSpaceHeaders",
                 hoedown_set_extensions_spaceheaders,
                 NULL, OR_ALL, "Enable hoedown extension Space Headers"),
    AP_INIT_FLAG("HoedownExtSuperscript",
                 hoedown_set_extensions_superscript,
                 NULL, OR_ALL, "Enable hoedown extension Superscript"),
    AP_INIT_FLAG("HoedownExtLaxSpacing",
                 hoedown_set_extensions_laxspacing,
                 NULL, OR_ALL, "Enable hoedown extension Lax Spacing"),
    AP_INIT_FLAG("HoedownExtDisableIndentedCode",
                 hoedown_set_extensions_disableindentedcode,
                 NULL, OR_ALL, "Disable hoedown extension Indented Code"),
    AP_INIT_FLAG("HoedownExtHighlight",
                 hoedown_set_extensions_highlight,
                 NULL, OR_ALL, "Enable hoedown extension Highlight"),
    AP_INIT_FLAG("HoedownExtFootnotes",
                 hoedown_set_extensions_footnotes,
                 NULL, OR_ALL, "Enable hoedown extension Footnotes"),
    AP_INIT_FLAG("HoedownExtQuote",
                 hoedown_set_extensions_quote,
                 NULL, OR_ALL, "Enable hoedown extension Quote"),
    AP_INIT_FLAG("HoedownExtSpecialAttribute",
                 hoedown_set_extensions_specialattribute,
                 NULL, OR_ALL, "Enable hoedown extension Special Attribute"),
    /* Html Render mode options */
    AP_INIT_FLAG("HoedownRenderSkipHtml", hoedown_set_render_skiphtml,
                 NULL, OR_ALL, "Enable hoedown render Skip HTML"),
    AP_INIT_FLAG("HoedownRenderSkipStyle", hoedown_set_render_skipstyle,
                 NULL, OR_ALL, "Enable hoedown render Skip Style"),
    AP_INIT_FLAG("HoedownRenderSkipImages", hoedown_set_render_skipimages,
                 NULL, OR_ALL, "Enable hoedown render Skip Images"),
    AP_INIT_FLAG("HoedownRenderSkipLinks", hoedown_set_render_skiplinks,
                 NULL, OR_ALL, "Enable hoedown render Skip Links"),
    AP_INIT_FLAG("HoedownRenderExpandTabs", hoedown_set_render_expandtabs,
                 NULL, OR_ALL, "Enable hoedown render Expand Tabs"),
    AP_INIT_FLAG("HoedownRenderSafelink", hoedown_set_render_safelink,
                 NULL, OR_ALL, "Enable hoedown render Safelink"),
    AP_INIT_FLAG("HoedownRenderToc", hoedown_set_render_toc,
                 NULL, OR_ALL, "Enable hoedown render Toc"),
    AP_INIT_FLAG("HoedownRenderHardWrap", hoedown_set_render_hardwrap,
                 NULL, OR_ALL, "Enable hoedown render Hard Wrap"),
    AP_INIT_FLAG("HoedownRenderUseXhtml", hoedown_set_render_usexhtml,
                 NULL, OR_ALL, "Enable hoedown render Use XHTML"),
    AP_INIT_FLAG("HoedownRenderEscape", hoedown_set_render_escape,
                 NULL, OR_ALL, "Enable hoedown render Escape"),
    AP_INIT_FLAG("HoedownRenderPrettify", hoedown_set_render_prettify,
                 NULL, OR_ALL, "Enable hoedown render Prettify"),
    AP_INIT_FLAG("HoedownRenderUseTaskList", hoedown_set_render_usetasklist,
                 NULL, OR_ALL, "Enable hoedown render Use Task List"),
    AP_INIT_FLAG("HoedownRenderSkipEol", hoedown_set_render_skipeol,
                 NULL, OR_ALL, "Enable hoedown render Skip EOL"),
    AP_INIT_FLAG("HoedownRenderTocSkipEscape", hoedown_set_render_tocskipescape,
                 NULL, OR_ALL, "Enable hoedown render Skip Toc Escape"),
    {NULL}
};

static void
hoedown_register_hooks(apr_pool_t *p)
{
    ap_hook_handler(hoedown_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA hoedown_module =
{
    STANDARD20_MODULE_STUFF,
    hoedown_create_dir_config, /* create per-dir    config structures */
    hoedown_merge_dir_config,  /* merge  per-dir    config structures */
    NULL,                      /* create per-server config structures */
    NULL,                      /* merge  per-server config structures */
    hoedown_cmds,              /* table of config file commands       */
    hoedown_register_hooks     /* register hooks                      */
};
