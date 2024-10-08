/*
 * SPDX-FileCopyrightText: 2024 Yaşar Arabacı <yasar11732@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FED_MAINLOOP
#define FED_MAINLOOP
#include "fed.h"
#include "str.h"
#include "xmlparse.h"
#include "transfer.h"

#include <assert.h>

static void process_curl_events(fed *f) {

    CURLMsg *msg = NULL;
    int numMsg = 0;

    while(notnull(msg = curl_multi_info_read(f->mh, &numMsg))) {
        transfer_t *t = NULL;
        
        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &t);
        
        assert(notnull(t));
        assert(msg->msg == CURLMSG_DONE);
        if(msg->data.result == CURLE_OK) {
            save_feed_details(t);
            if(t->cbData > 0)
                process_response(t);
        } else {
            fprintf(stderr, "%s: %s\r\n", t->url, curl_easy_strerror(msg->data.result));
        }

        free_transfer(t);
        curl_multi_remove_handle(f->mh, msg->easy_handle);
        curl_easy_cleanup(msg->easy_handle);
        curl_slist_free_all(t->headers);
    }
}

static bool main_loop(fed *f) {
    bool success = true;

    do {
        success = curl_multi_perform(f->mh, &f->runningHandles) == CURLM_OK;

        if(success) {
            process_curl_events(f);
            add_transfers(f);
            success = curl_multi_wait(f->mh, NULL, 0, 1000, NULL) == CURLM_OK;
        } else {
             assert(false);
        }

    } while(success && f->runningHandles > 0);

    return success;
}

#endif
