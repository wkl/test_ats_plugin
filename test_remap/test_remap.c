/*
  test_remap.c

  Created by Conan Wang <conanmind@gmail.com> Nov 2012
*/
#include <stdio.h>
#include <string.h>

#include <ts/ts.h>
#include <ts/remap.h>

#define PLUGIN_NAME "test_remap"
#define PLUGIN_VERSION "0.1"

typedef struct {
} remap_config;

static TSTextLogObject log;


static void
handle_request(TSHttpTxn txnp, TSRemapRequestInfo* rri, remap_config* conf)
{
  TSMBuffer bufp = rri->requestBufp;
  TSMLoc hdr_loc = rri->requestHdrp;

  TSMLoc host_field_loc;
  host_field_loc = TSMimeHdrFieldFind(bufp, hdr_loc,
                                      TS_MIME_FIELD_HOST, TS_MIME_LEN_HOST);
  const char * host;
  int host_len;

  if (host_field_loc) {
    host = TSMimeHdrFieldValueStringGet(bufp, hdr_loc, host_field_loc,
                                        0, &host_len);
    TSDebug(PLUGIN_NAME, "host header: '%.*s'", host_len, host);
  }

  char * url;
  int url_len;
  url = TSHttpTxnEffectiveUrlStringGet(txnp, &url_len);
  TSDebug(PLUGIN_NAME, "url: '%.*s'", url_len, url);
  TSfree(url);

  TSHandleMLocRelease(bufp, hdr_loc, host_field_loc);
}

/*
  This function will be called for every request of the channel.
*/
TSRemapStatus
TSRemapDoRemap(void* ih, TSHttpTxn rh, TSRemapRequestInfo* rri)
{
  remap_config* conf = ih;

  handle_request(rh, rri, conf);

  return TSREMAP_NO_REMAP;  /* Continue with next remap plugin in chain */
}

/*
 Create remap instance for each remap rule
*/
TSReturnCode
TSRemapNewInstance(int argc, char* argv[], void** ih, char* errbuf,
                   int errbuf_size)
{
  remap_config* conf = TSmalloc(sizeof(remap_config));

  *ih = (void*)conf;

  TSDebug(PLUGIN_NAME, "creating remap instance for '%s' -> '%s'",
          argv[0], argv[1]);

  return TS_SUCCESS;
}

/* Release instance memory allocated in TSRemapNewInstance */
void
TSRemapDeleteInstance(void* ih)
{
  TSDebug(PLUGIN_NAME, "deleting remap instance %p", ih);

  if (!ih) return;

  remap_config* conf = ih;

  TSfree(conf);
}

static int
check_ts_version()
{
  const char *ts_version = TSTrafficServerVersionGet();
  int result = 0;

  if (ts_version) {
    int major_ts_version = 0;
    int minor_ts_version = 0;
    int patch_ts_version = 0;

    if (sscanf(ts_version, "%d.%d.%d", &major_ts_version, &minor_ts_version,
                &patch_ts_version) != 3) {
      return 0;
    }

    /* Need at least TS 3.0.0 */
    if (major_ts_version >= 3) {
      result = 1;
    }

  }
  return result;
}

/*
  Initalize the plugin as a remap plugin.
*/
TSReturnCode 
TSRemapInit(TSRemapInterface* api_info, char* errbuf, int errbuf_size)
{
  if (!api_info) {
    strncpy(errbuf, "[TSRemapInit] - Invalid TSRemapInterface argument",
            errbuf_size - 1);
    return TS_ERROR;
  }

  if (api_info->size < sizeof(TSRemapInterface)) {
    strncpy(errbuf, 
            "[TSRemapInit] - Incorrect size of TSRemapInterface structure",
            errbuf_size - 1);
    return TS_ERROR;
  }

  TSReturnCode error;
  if (!log) {
    error = TSTextLogObjectCreate(PLUGIN_NAME, TS_LOG_MODE_ADD_TIMESTAMP, &log);
    if (!log || error == TS_ERROR) {
      TSError("[%s] Error creating log file\n", PLUGIN_NAME);
    }
  }

  if (!check_ts_version()) {
    TSError("[%s] Plugin requires Traffic Server 3.0.0 or later", PLUGIN_NAME);
  }

  TSDebug(PLUGIN_NAME, "%s plugin is initialized, version: %s",
          PLUGIN_NAME, PLUGIN_VERSION);

  if (log) {
    TSTextLogObjectWrite(log, "%s plugin is initialized, version: %s",
        PLUGIN_NAME, PLUGIN_VERSION);
  }

  return TS_SUCCESS;
}

