/* 
**  mod_log_fluentd.c -- Apache sample log_fluentd module
**  [Autogenerated via ``apxs -n log_fluentd -g'']
**
**  To play with this sample module first compile it into a
**  DSO file and install it into Apache's modules directory 
**  by running:
**
**    $ apxs -c -i mod_log_fluentd.c
**
**  Then activate it in Apache's apache2.conf file for instance
**  for the URL /log_fluentd in as follows:
**
**    #   apache2.conf
**    LoadModule log_fluentd_module modules/mod_log_fluentd.so
**    <Location /log_fluentd>
**    SetHandler log_fluentd
**    </Location>
**
**  Then after restarting Apache via
**
**    $ apachectl restart
**
**  you immediately can request the URL /log_fluentd and watch for the
**  output of this module. This can be achieved for instance via:
**
**    $ lynx -mime_header http://localhost/log_fluentd 
**
**  The output should be similar to the following one:
**
**    HTTP/1.1 200 OK
**    Date: Tue, 31 Mar 1998 14:42:22 GMT
**    Server: Apache/1.3.4 (Unix)
**    Connection: close
**    Content-Type: text/html
**  
**    The sample page from mod_log_fluentd.c
*/ 

#include "httpd.h"
#include "http_config.h"
#include "http_log.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "util_time.h"

#include "apr_strings.h"
#include "apr_lib.h"
#include "apr_hash.h"
#include "apr_hooks.h"
#include "apr_optional.h"
#include "apr_reslist.h"

#include "mod_log_config.h"

typedef struct {
	void *normal_handle; /* apache mod_log_config handle */
} fluentd_log;

module AP_MODULE_DECLARE_DATA log_fluentd_module;


static apr_hash_t *fluentd_hash;

static ap_log_writer_init *normal_log_writer_init = NULL;
static ap_log_writer *normal_log_writer = NULL;

static void* log_fluentd_writer_init(apr_pool_t *p, server_rec *s, const char *name)
{
	fluentd_log *log;

	if (!(log = apr_hash_get(fluentd_hash, name, APR_HASH_KEY_STRING))) {
		log = apr_palloc(p, sizeof(fluentd_log));

		// god, this does not work!
		// log->normal_handle = normal_log_writer_init(p, s, name);
		log->normal_handle = NULL;
		apr_hash_set(fluentd_hash, name, APR_HASH_KEY_STRING, log);
	}

	return log;
}


static apr_status_t log_fluentd_writer(request_rec *r, void *handle, const char **strs, int *strl, int nelts, apr_size_t len)
{
	fluentd_log *log = (fluentd_log *)handle;
	apr_status_t result;

	ap_log_rerror(APLOG_MARK,APLOG_ERR, 0, r, "Hello mod_log_fluentd");
	if (log->normal_handle) {
		result = normal_log_writer(r, log->normal_handle, strs, strl, nelts, len);
	}

	return OK;
}

static int log_fluentd_pre_config(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp)
{
	static APR_OPTIONAL_FN_TYPE(ap_log_set_writer_init) *log_set_writer_init_fn = NULL;
	static APR_OPTIONAL_FN_TYPE(ap_log_set_writer) *log_set_writer_fn = NULL;

	log_set_writer_init_fn = APR_RETRIEVE_OPTIONAL_FN(ap_log_set_writer_init);
	log_set_writer_fn = APR_RETRIEVE_OPTIONAL_FN(ap_log_set_writer);

	if (log_set_writer_init_fn && log_set_writer_fn) {
		if (!normal_log_writer_init) {
			module *mod_log_config = ap_find_linked_module("mod_log_config.c");

			normal_log_writer_init = log_set_writer_init_fn(log_fluentd_writer_init);
			normal_log_writer = log_set_writer_fn(log_fluentd_writer);
		}
	}
	return OK;
}

static int log_fluentd_transaction(request_rec *r)
{
	/* for testing */
	ap_log_rerror(APLOG_MARK,APLOG_ERR, 0, r, "Hello Chobie");
	return OK;
}

static apr_status_t log_fluentd_child_exit(void *data)
{
	apr_pool_t *p = data;
	apr_hash_index_t *index;
	fluentd_log *log;

	for (index = apr_hash_first(p, fluentd_hash); index; index = apr_hash_next(index)) {
		apr_hash_this(index, NULL, NULL, (void*) &log);
	}

	ap_log_perror(APLOG_MARK, APLOG_DEBUG, 0, p, "Hello World");	
	return OK;
}

static void log_fluentd_child_init(apr_pool_t *p, server_rec *s)
{
	apr_pool_cleanup_register(p, p, log_fluentd_child_exit, log_fluentd_child_exit);
}

static void log_fluentd_register_hooks(apr_pool_t *p)
{
	static const char *pre[] = { "mod_log_config.c", NULL };
	fluentd_hash = apr_hash_make(p);

	ap_hook_pre_config(log_fluentd_pre_config, pre, NULL, APR_HOOK_REALLY_FIRST);
	ap_hook_child_init(log_fluentd_child_init, NULL, NULL, APR_HOOK_MIDDLE);
	ap_hook_log_transaction(log_fluentd_transaction,NULL,NULL,APR_HOOK_MIDDLE);
}

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA log_fluentd_module = {
    STANDARD20_MODULE_STUFF, 
    NULL,                  /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    NULL,                  /* table of config file commands       */
    log_fluentd_register_hooks  /* register hooks                      */
};

