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
#include "apr_tables.h"

#include "mod_log_config.h"
#include "uv.h"

#include "pthread.h"
#include "msgpack.h"

#include <stddef.h>

typedef struct {
	char *host;
	unsigned int port;
	struct sockaddr_in server_addr;
	uv_tcp_t socket;
	uv_connect_t connect;
	int retry_count;
} fluentd_t;

typedef struct {
	fluentd_t *fluentd;
	char *uri;
	char *host;
	unsigned int port;
	char *tag;
	int tag_len;
	int write_local;
	void *normal_handle; /* apache mod_log_config handle */
} fluentd_log;

module AP_MODULE_DECLARE_DATA log_fluentd_module;

static pthread_t thread;
static apr_hash_t *fluentd_hash;

static ap_log_writer_init *normal_log_writer_init = NULL;
static ap_log_writer *normal_log_writer = NULL;


static void write_cb(uv_write_t *req, int status)
{
	fluentd_log *log = (fluentd_log*)req->data;

	if (status != 0) {
		//fluentd_open(log->fluentd, log->host, log->port);
	}
	free(req);
}

int log_fluentd_post(fluentd_log *log, msgpack_sbuffer *sbuf)
{
	uv_write_t *req;
	uv_buf_t buf;

	buf = uv_buf_init(sbuf->data, sbuf->size);
	req = (uv_write_t *)malloc(sizeof(*req));
	req->data = log;
	uv_write(req, (uv_stream_t*)&log->fluentd->socket, &buf, 1, write_cb);
}

static void fluentd_connect_cb(uv_connect_t *conn_req, int status)
{
}

static void run_fluentd()
{
	uv_run(uv_default_loop());
}

/* begin fluentd implementation */
int fluentd_open(fluentd_t *p, const char *host, const int port)
{
	p->host = strdup(host);
	p->port = port;
	p->server_addr = uv_ip4_addr(p->host, p->port);

	uv_tcp_init(uv_default_loop(), &p->socket);
	uv_tcp_connect(&p->connect, &p->socket, p->server_addr, fluentd_connect_cb);

	return 0;
}

/* end fluentd implementation */

static void* log_fluentd_writer_init(apr_pool_t *p, server_rec *s, const char *name)
{
	fluentd_t *fluentd;
	fluentd_log *log;
	int error = 0;
	int fluentdWriter = 1;

	if (name != NULL && strstr(name, "fluentd") == NULL) {
		fluentdWriter = 0;
	}

	if (!(log = apr_hash_get(fluentd_hash, name, APR_HASH_KEY_STRING))) {
		log = apr_palloc(p, sizeof(fluentd_log));
		
		if (fluentdWriter == 1) {
			char *uri = NULL;
			char *c = NULL;
			
			
			uri = apr_pstrdup(p,name);
			fluentd = apr_palloc(p, sizeof(fluentd_t));
			log->host = "127.0.0.1";
			log->port = 24224;
			log->tag = "debug.test";
			log->tag_len = strlen(log->tag);
			c = ap_strrchr(uri, ':');
			if (c != NULL) {
				if (c != uri+6) {
					log->port = apr_atoi64(c+1);
					*c = '\0';
				}
			}
			
			c = ap_strrchr(uri, '@');
			if (c != NULL) {
				*c++ = '\0';
				log->host = c;
			}
			
			c = ap_strrchr(uri, ':');
			if (c != NULL) {
				*c++ = '\0';
				log->tag = c;
				log->tag_len = strlen(log->tag);
			}
			
			error = fluentd_open(fluentd, log->host, log->port);

			log->uri = uri;
			log->fluentd = fluentd;
			log->write_local = 0;
			log->normal_handle = NULL;
			pthread_create(&thread,NULL, (void*)run_fluentd,NULL);
		} else {
			log->write_local = 1;
			log->normal_handle = normal_log_writer_init(p, s, name);
		}

		apr_hash_set(fluentd_hash, name, APR_HASH_KEY_STRING, log);
	}

	return log;
}

static apr_status_t log_fluentd_writer(request_rec *r, void *handle, const char **strs, int *strl, int nelts, apr_size_t len)
{
	fluentd_log *log = (fluentd_log *)handle;
	apr_status_t result;

	if (log->write_local == 1) {
		if (log->normal_handle) {
			result = normal_log_writer(r, log->normal_handle, strs, strl, nelts, len);
		}
	} else {
		int i =0;
		char *str;
		msgpack_sbuffer sbuf;
		msgpack_packer pk;
		msgpack_sbuffer_init(&sbuf);
		msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

		msgpack_pack_array(&pk, 3);
		msgpack_pack_raw(&pk,log->tag_len);
		msgpack_pack_raw_body(&pk, log->tag, log->tag_len);
		msgpack_pack_int(&pk, 1329275765);
		msgpack_pack_array(&pk,nelts);
		for (i = 0; i < nelts; i++) {
			msgpack_pack_raw(&pk,strl[i]);
			msgpack_pack_raw_body(&pk, strs[i], strl[i]);
		}

		log_fluentd_post(log,&sbuf);
		msgpack_sbuffer_destroy(&sbuf);
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

static apr_status_t log_fluentd_child_exit(void *data)
{
	apr_pool_t *p = data;
	apr_hash_index_t *index;
	fluentd_log *log;

	for (index = apr_hash_first(p, fluentd_hash); index; index = apr_hash_next(index)) {
		apr_hash_this(index, NULL, NULL, (void*) &log);
	}

	pthread_cancel(thread);
	return OK;
}

static int log_fluentd_transaction(request_rec *r)
{
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

