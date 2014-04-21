// See license in demo.c

#include <gnutls/gnutls.h>

// Wrapper functions for gnutls_heartbeat_ping[_data]
int send_random_heartbeat(gnutls_session_t session, unsigned int flags){
	return gnutls_heartbeat_ping(session, 32,
		3, GNUTLS_HEARTBEAT_WAIT | flags);
}
int send_custom_heartbeat(gnutls_session_t session, const char* data,
	size_t length, unsigned int flags){
	// GNUTLS adds 16 characters to all heartbeat messages;
	// we need to compensate for that here.
	return gnutls_heartbeat_ping_data(session, data, 16 + length,
		3, GNUTLS_HEARTBEAT_WAIT | flags);
}