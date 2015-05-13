/*
 * Copyright (c) 2015 Jan-Piet Mens <jpmens()gmail.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of mosquitto nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <sys/utsname.h>
#include <signal.h>
#include <mosquitto.h>
#include <errno.h>
#include <ctype.h>
#include "uthash.h"
#include "utstring.h"
#include "json.h"
#include "ini.h"	/* https://github.com/benhoyt/inih */

#ifndef TRUE
# define TRUE (1)
#endif
#ifndef FALSE
# define FALSE (0)
#endif

#define PROGNAME	"mqtt-sys"
#define TOPIC_SYS	"$SYS/#"
#define DIM(x)		( sizeof(x) / sizeof(x[0]) )
#define CONFIGFILE	"/usr/local/etc/mqtt-sys.ini"
#define SECTION  	"defaults"

typedef struct {
    const char *host;
    const char *nodename;		/* for collectd; defaults to short uname */
    int port;
    const char *username;
    const char *password;
    const char *psk_key;
    const char *psk_identity;
    const char *ca_file;
} config;

static config cf = {
	.host		= "localhost",
	.port		= 1883
};

/*
 * A hash of metrics with their name (metric), type (e.g. gauge) and
 * optional JSON element.
 */

struct metrics_h {
	const char *metric;
	const char *type;
	const char *element;	/* If NULL, not JSON */
	UT_hash_handle hh;
};

struct topics_h {
    const char *topic;          /* MQTT topic */
    struct metrics_h *mh;	/* hash of metrics to produce per topic */
    UT_hash_handle hh;
};
static struct topics_h *topics_h = NULL;

static int verbose = FALSE;


#define _eq(n) (strcmp(key, n) == 0)
static int handler(void *cf, const char *section, const char *key, const char *val)
{
	config *c = (config *)cf;
	struct topics_h *th;
	struct metrics_h *mh;
	static UT_string *elem, *metric;
	char *p;

	utstring_renew(elem);
	utstring_renew(metric);

	// printf("section=%s  >%s<-->%s\n", section, key, val);

	if (!strcmp(section, SECTION)) {

		if (_eq("host"))
			c->host = strdup(val);
		if (_eq("username"))
			c->username = strdup(val);
		if (_eq("password"))
			c->password = strdup(val);
		if (_eq("psk_key"))
			c->psk_key = strdup(val);
		if (_eq("psk_identity"))
			c->psk_identity = strdup(val);
		if (_eq("ca_file"))
			c->ca_file = strdup(val);
		if (_eq("nodename"))
			c->nodename = strdup(val);

		if (_eq("port"))
			c->port = atoi(val);

		return (1);
	}

	/*
	 * The Section name is MQTT topic. If we've not yet seen this, add
	 * it to the hash, otherwise, push the new metric into the it's
	 * array.
	 * The entry's key is the metric type (gauge, counter)
	 *
	 *  [owntracks/gw/+]	<-- section => topic
	 *  gauge = cars/{tid}/speed@vel
	 *
	 *     key: "gauge"
	 *     val: "cars/{tid}/speed@vel"
	 *           ^^^^^^^^^^^^^^^^ ^^^
	 *              metric        elem
	 *
	 *  [$SYS/broker/uptime]
	 *  counter = *
	 */

	if ((p = strchr(val, '<')) != NULL) {		/* "<vel" */
		utstring_printf(elem, "%s", p + 1);	/* "vel"  */
		*p = 0;
		utstring_printf(metric, "%s", val);
	} else {
		// utstring_printf(elem, "%s", val);
		utstring_clear(elem);

		if (strcmp(val, "*") == 0) {		/* copy section/topic name to metric */
			utstring_printf(metric, "%s", section);
		} else {
			utstring_printf(metric, "%s", val);
		}
	}

	HASH_FIND_STR(topics_h, section, th);
	if (!th) {
		th = (struct topics_h *)malloc(sizeof(struct topics_h));
		th->topic = strdup(section);

		HASH_ADD_KEYPTR( hh, topics_h, th->topic, strlen(th->topic), th );

		/* experiment: add to metric_h with this hash */

		th->mh = NULL;
		mh = (struct metrics_h *)malloc(sizeof(struct metrics_h));
		mh->metric = strdup(utstring_body(metric));
		mh->type = strdup(key);
		mh->element = utstring_len(elem) ? strdup(utstring_body(elem)) : NULL;
		HASH_ADD_KEYPTR( hh, th->mh, mh->metric, strlen(mh->metric), mh );


	} else {
		HASH_FIND_STR(th->mh, val, mh);
		if (mh) {
			puts("PANIC!!!");
		} else {
			mh = (struct metrics_h *)malloc(sizeof(struct metrics_h));
			mh->metric = strdup(utstring_body(metric));
			mh->type = strdup(key);
			mh->element = utstring_len(elem) ? strdup(utstring_body(elem)) : NULL;
			HASH_ADD_KEYPTR( hh, th->mh, mh->metric, strlen(mh->metric), mh );
		}

	}

	return (1);
}


static struct mosquitto *m = NULL;

/*
 * User data for Mosquitto
 */

struct udata {
	char *nodename;
};

void catcher(int sig)
{
	fprintf(stderr, "Going down on signal %d\n", sig);

	if (m) {
		mosquitto_disconnect(m);
		mosquitto_loop_stop(m, false);
		mosquitto_lib_cleanup();
	}
	exit(1);
}

void fatal(void)
{
	if (m) {
		mosquitto_disconnect(m);
		mosquitto_loop_stop(m, false);
		mosquitto_lib_cleanup();
	}
	exit(1);
}

double json_object(JsonNode *json, const char *element)
{
	JsonNode *m;
	double value = 0.0L;

	if ((m = json_find_member(json, element)) == NULL)
		return (value);

	if (m && m->tag == JSON_STRING) {
		value = atof(m->string_);
	} else if (m && m->tag == JSON_NUMBER) {
		value = m->number_;
	}

	return (value);
}

/*
 * Expand the content of `line', which may have one or more {token}
 * in it into `res', using the decoded JSON at `json'.
 */

void xexpand(UT_string *res, const char *line, JsonNode *json)
{
    JsonNode *m;
    static UT_string *token;
    const char *lp = line;

    utstring_renew(token);

    for (lp = line; lp && *lp; lp++ ) {
        if (*lp == '\\') {
            utstring_printf(res, "%c", *++lp);
            continue;
        }
        if (*lp != '{') {
            utstring_printf(res, "%c", *lp);
            continue;
        }

        utstring_renew(token);
        if (*++lp == '}') { /* skip over this { */
            /* Empty token; push back */
            utstring_printf(res, "%c", *lp);
            continue;
        }

        do {
            utstring_printf(token, "%c", *lp++);
        } while (*lp && *lp != '}');
	// printf("TOKEN=[%s]\n", utstring_body(token));

        // printf("LAST=%d\n", *lp);
        if (*lp != '}') {
            /* Push back, incl leading brace */
            utstring_printf(res, "{%s", utstring_body(token));
            break;
        }


	/* See if `token' is a JSON element, and if so, interpolate
	 * its value. If token is not in JSON, stuff it back to
	 * indicate the error.
	 */

	if ((m = json_find_member(json, utstring_body(token))) != NULL) {
		if (m && m->tag == JSON_STRING) {
			utstring_printf(res, "%s", m->string_);
		} else if (m && m->tag == JSON_NUMBER) {
			utstring_printf(res, "%lf", m->number_);
		} else {
			utstring_printf(res, "FIXME-JSON");
		}
	} else {
            /* stuff token and its braces back into result */
            utstring_printf(res, "{%s}", utstring_body(token));
        }
    }
}



void cb_sub(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
	char *topic = msg->topic;
	char *payload = msg->payload;
	struct udata *ud = (struct udata *)userdata;
	time_t now;
	struct topics_h *th, *currth = NULL;
	bool bf;
	struct metrics_h *mh;


	/*
	 * We can't try to find topic in our hash, because this may be the
	 * result of a wildcard subscription. Instead, see if one of the
	 * topics in hash matches the subscription. Slower, but I can't
	 * help that.
	 */

	for (th = topics_h; th != NULL; th = th->hh.next) {
		if (mosquitto_topic_matches_sub(th->topic, topic, &bf) == MOSQ_ERR_SUCCESS) {
			if (bf == 1) {
				currth = th;
				break;
			}
		}
	}

	if (currth == NULL) {
		puts("HUH? PANIC? topic not found");
		return;
	}

	time(&now);

	/*
	 * For each of the metrics configured for this subscription, do the
	 * "needful".
	 * If `element' in metric is NULL, use the original payload; otherwise
	 * it's the name of a JSON element in the (assumed) JSON payload.
	 */

	for (mh = currth->mh; mh != NULL; mh = mh->hh.next) {
		JsonNode *json;
		double number = -1.0L;
		static UT_string *metric_name;

		utstring_renew(metric_name);

		if (verbose)
			fprintf(stderr, "     =====[ %s ] (%s) %s\n", mh->metric, mh->type, mh->element);

		if (mh->element != NULL) {	/* JSON */
			if ((json = json_decode(payload)) == NULL) {
				continue;
			}
			utstring_clear(metric_name);
			xexpand(metric_name, mh->metric, json);

			number = json_object(json, mh->element);

		} else {
			utstring_printf(metric_name, "%s", mh->metric);
			number = atof(payload);
		}
		printf("PUTVAL %s/%s/%s-%s %ld:%.2lf\n",
			ud->nodename,
			PROGNAME,
			mh->type,
			utstring_body(metric_name),
			now,
			number);
	}
}

void cb_disconnect(struct mosquitto *mosq, void *userdata, int rc)
{
	if (rc == 0) {
		// Disconnect requested by client
	} else {
		fprintf(stderr, "%s: disconnected: reason: %d (%s)\n",
			PROGNAME, rc, strerror(errno));
		fatal();
	}
}

int main(int argc, char **argv)
{
	char ch, *progname = *argv;
	int usage = 0, rc;
	struct utsname uts;
	char clientid[80];
	int keepalive = 60;
	int tls_insecure = FALSE;
	struct udata udata;
	char *configfile = CONFIGFILE;
	struct topics_h *th;


	setvbuf(stdout, NULL, _IONBF, 0);

	while ((ch = getopt(argc, argv, "vs:f:")) != EOF) {
		switch (ch) {
			case 'v':
				verbose = TRUE;
				break;

			case 's':
				tls_insecure = TRUE;
				break;
			case 'f':
				configfile = strdup(optarg);
				break;
			default:
				usage = 1;
				break;
		}
	}



	if (ini_parse(configfile, handler, &cf) < 0) {
		fprintf(stderr, "%s: Can't load '%s'\n", PROGNAME, configfile);
		return 1;
	}

	if (usage) {
		fprintf(stderr, "Usage: %s [-v] [-s] [-f configfile]\n", progname);
		exit(1);
	}

	/* Determine nodename: either use the -h value of the MQTT broker
	 * or get local nodename */

	if (cf.nodename == NULL) {
		if (uname(&uts) == 0) {
			char *p;
			cf.nodename = strdup(uts.nodename);

			if ((p = strchr(cf.nodename, '.')) != NULL)
				*p = 0;
		} else {
			cf.nodename = strdup("unknown");
		}
	}

	mosquitto_lib_init();

	udata.nodename = (char *)cf.nodename;

	sprintf(clientid, "%s-%d", PROGNAME, getpid());
	if ((m = mosquitto_new(clientid, TRUE, (void *)&udata)) == NULL) {
		fprintf(stderr, "Out of memory.\n");
		exit(1);
	}

	if (cf.psk_key && cf.psk_identity) {
		rc = mosquitto_tls_psk_set(m, cf.psk_key, cf.psk_identity,NULL);
		if (rc != MOSQ_ERR_SUCCESS) {
			fprintf(stderr, "Cannot set TLS PSK: %s\n",
				mosquitto_strerror(rc));
			exit(3);
		}
	} else if (cf.ca_file) {
		rc = mosquitto_tls_set(m, cf.ca_file, NULL, NULL, NULL, NULL);
		if (rc != MOSQ_ERR_SUCCESS) {
			fprintf(stderr, "Cannot set TLS PSK: %s\n",
				mosquitto_strerror(rc));
			exit(3);
		}

		/* FIXME */
		// mosquitto_tls_opts_set(m, SSL_VERIFY_PEER, "tlsv1", NULL);

		if (tls_insecure) {
#if LIBMOSQUITTO_VERSION_NUMBER >= 1002000
			/* mosquitto_tls_insecure_set() requires libmosquitto 1.2. */
			mosquitto_tls_insecure_set(m, TRUE);
#endif
		}
	}

	if (cf.username) {
		mosquitto_username_pw_set(m, cf.username, cf.password);
	}

	mosquitto_message_callback_set(m, cb_sub);
	mosquitto_disconnect_callback_set(m, cb_disconnect);

	if ((rc = mosquitto_connect(m, cf.host, cf.port, keepalive)) != MOSQ_ERR_SUCCESS) {
		fprintf(stderr, "Unable to connect to %s:%d: %s\n", cf.host, cf.port,
			mosquitto_strerror(rc));
		perror("");
		exit(2);
	}

	signal(SIGINT, catcher);

	/*
	 * Set up an MQTT subscription for each of the topics we have
	 * in the topics hash.
	 */

	for (th = topics_h; th != NULL; th = th->hh.next) {
		// fprintf(stderr, "%s: subscribe to %s\n", PROGNAME, th->topic);
		mosquitto_subscribe(m, NULL, th->topic, 0);
	}

	while (1) {
		int rc = mosquitto_loop(m, -1, 1);
		if (rc) {
			sleep(15);
			mosquitto_reconnect(m);
		}
	}

	/* Unreached */

	/*
	 * There's a tonne of memory we ought to free (topics_h, etc) but
	 * we don't get here, so nobody will notice.
	 */


	mosquitto_disconnect(m);
	mosquitto_lib_cleanup();

	return 0;
}
