/* This example code is placed in the public domain. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_EXAMPLE_H
#include "examples.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

/* A very basic TLS client, with X.509 authentication and server certificate
** verification. Note that error checking for missing files etc. is omitted
** for simplicity.
**/

#define MAX_BUF 1024
#define CAFILE "/etc/ssl/certs/ca-certificates.crt"
#define MSG "GET / HTTP/1.0\r\n\r\n"

extern int tcp_connect(const char* server, char* port);
extern void tcp_close(int sd);
extern void session_live_callback(gnutls_session_t session);
int _ssh_verify_certificate_callback(gnutls_session_t session);
int _check_and_print_x509_certificate(
	gnutls_session_t session, const char *hostname,
	const int type, const gnutls_datum_t *cert_list,
	unsigned int* status);
static void tls_log_func(int level, const char* message);

int main(void)
{
	int ret, sd;
	gnutls_session_t session;
	const char *err;
	gnutls_certificate_credentials_t xcred;
	const char* HOSTNAME = "domU-12-31-39-13-C1-26.compute-1.internal";
	const char* HOSTIP = "54.82.141.66";
	//const char* HOSTNAME = "acm.wustl.edu";
	//const char* HOSTIP = "128.252.20.101";

	gnutls_global_init();

	/* X509 stuff */
	gnutls_certificate_allocate_credentials(&xcred);

	/* sets the trusted cas file
	**/
	gnutls_certificate_set_x509_trust_file(xcred, CAFILE,
		GNUTLS_X509_FMT_PEM);
	gnutls_certificate_set_verify_function(xcred,
		_ssh_verify_certificate_callback);

	/* Initialize TLS session */
	gnutls_init(&session, GNUTLS_CLIENT);
	gnutls_session_set_ptr(session, (void *) HOSTNAME);
	gnutls_server_name_set(session, GNUTLS_NAME_DNS, HOSTNAME,
		strlen(HOSTNAME));
	gnutls_global_set_log_level(9);
	gnutls_global_set_log_function(tls_log_func);
	gnutls_heartbeat_enable(session, GNUTLS_HB_LOCAL_ALLOWED_TO_SEND | GNUTLS_HB_PEER_ALLOWED_TO_SEND);

	/* Use default priorities */
	ret = gnutls_priority_set_direct(session, "NORMAL", &err);
	if (ret < 0) {
		if (ret == GNUTLS_E_INVALID_REQUEST) {
			fprintf(stderr, "Syntax error at: %s\n", err);
		}
		exit(1);
	}

	/* put the x509 credentials to the current session
	**/
	gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, xcred);

	/* connect to the peer
	**/
	sd = tcp_connect(HOSTIP, "443");

	gnutls_transport_set_int(session, sd);
	gnutls_handshake_set_timeout(session,
		GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);

	/* Perform the TLS handshake
	**/
	do {
		ret = gnutls_handshake(session);
	}
	while (ret < 0 && gnutls_error_is_fatal(ret) == 0);

	if (ret < 0) {
		fprintf(stderr, "*** Handshake failed\n");
		gnutls_perror(ret);
		goto end;
	} else {
		char *desc;

		desc = gnutls_session_get_desc(session);
		printf("- Session info: %s\n", desc);
		gnutls_free(desc);
	}

	session_live_callback(session);

	gnutls_bye(session, GNUTLS_SHUT_RDWR);

	end:
	tcp_close(sd);
	gnutls_deinit(session);
	gnutls_certificate_free_credentials(xcred);
	gnutls_global_deinit();

	return 0;
}

/* This function will verify the peer's certificate, check
** if the hostname matches. In addition it will perform an
** SSH-style authentication, where ultimately trusted keys
** are only the keys that have been seen before.
**/
int _ssh_verify_certificate_callback(gnutls_session_t session)
{
	unsigned int status;
	const gnutls_datum_t *cert_list;
	unsigned int cert_list_size;
	int ret, type, accept, scanfret;
	const char *hostname;

	/* read hostname */
	hostname = gnutls_session_get_ptr(session);
	printf("Hostname: %s\n", hostname);

	/* get certificate type */
	type = gnutls_certificate_type_get(session);
	if (type != GNUTLS_CRT_X509)
		return GNUTLS_E_CERTIFICATE_ERROR;

	/* load the certificate into memory */
	cert_list = gnutls_certificate_get_peers(session, &cert_list_size);
	if (cert_list == NULL) {
		printf("No certificate was found!\n");
		return GNUTLS_E_CERTIFICATE_ERROR;
	}

	ret = _check_and_print_x509_certificate(session,
		hostname, type, cert_list, &status);
	if (ret < 0)
		return ret;

	/* Do SSH verification */
	/* service may be obtained alternatively using getservbyport() */
	ret = gnutls_verify_stored_pubkey(NULL, NULL, hostname, "https",
		type, &cert_list[0], 0);
	if (ret == GNUTLS_E_NO_CERTIFICATE_FOUND) {
		printf("Host %s is not known.", hostname);

		/* Ask the user */
		printf("Accept this certificate? 1=yes: ");
		do {
			scanfret = scanf("%d", &accept);
		} while (scanfret < 0);

		/* if not trusted */
		if (accept != 1) {
			return GNUTLS_E_CERTIFICATE_ERROR;
		}
	} else if (ret == GNUTLS_E_CERTIFICATE_KEY_MISMATCH) {
		printf("Warning: host %s is known but has another key associated.",
			hostname);
		printf("It might be that the server has multiple keys, or you are under attack\n");

		/* Ask the user */
		printf("Accept this certificate? 1=yes: ");
		do {
			scanfret = scanf("%d", &accept);
		} while (scanfret <= 0);

		/* if not trusted */
		if (accept != 1) {
			return GNUTLS_E_CERTIFICATE_ERROR;
		}
	} else if (ret < 0) {
		printf("gnutls_verify_stored_pubkey: %s\n",
			gnutls_strerror(ret));
		return ret;
	}

	/* user trusts the key -> store it */
	if (ret != 0) {
		ret = gnutls_store_pubkey(NULL, NULL, hostname, "https",
			type, &cert_list[0], 0, 0);
		if (ret < 0)
			printf("gnutls_store_pubkey: %s\n",
				gnutls_strerror(ret));
	}

	/* notify gnutls to continue handshake normally */
	return 0;
}

int _check_and_print_x509_certificate(
	gnutls_session_t session, const char *hostname,
	const int type, const gnutls_datum_t *cert_list,
	unsigned int *status){

	int ret;
	gnutls_datum_t out;
	gnutls_x509_crt_t cert;

	/* This verification function uses the trusted CAs in the credentials
	** structure. So you must have installed one or more CA certificates.
	**/
	ret = gnutls_certificate_verify_peers3(
		session, hostname, status);
	if (ret < 0) {
		printf("Error\n");
		return GNUTLS_E_CERTIFICATE_ERROR;
	}

	/* Print the certificate */
	gnutls_x509_crt_init(&cert);
	gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER);
	ret = gnutls_x509_crt_print(cert, GNUTLS_CRT_PRINT_ONELINE, &out);
	if (ret < 0) {
		printf("Error\n");
		return GNUTLS_E_CERTIFICATE_ERROR;
	}
	printf("Certificate: %s\n", out.data);
	gnutls_free(out.data);

	/* Get the status of the certificate */
	ret = gnutls_certificate_verification_status_print(
		*status, type, &out, 0);
	if (ret < 0) {
		printf("Error\n");
		return GNUTLS_E_CERTIFICATE_ERROR;
	}
	printf("Cert Status: %s\n", out.data);
	gnutls_free(out.data);

	/* Certificate is not trusted */
	//if (*status != 0)
	//	return GNUTLS_E_CERTIFICATE_ERROR;

	return 0;
}

static void tls_log_func(int level, const char* message){
	printf("<%d> %s", level, message);
}
