#ifndef HAVE_CONFIG_H
	#define HAVE_CONFIG_H

	// #define SSP_DEBUG_PRINTF // debug for dprintf
	// #define SSP_DEBUG_EXT // debug php ext
	// #define SSP_DEBUG_TRIGGER // debug trigger handler
	// #define SSP_DEBUG_CONV // debug ssp_conv_setup/connect/disconnect
	#define SSP_CODE_TIMEOUT 10
	#define SSP_CODE_TIMEOUT_GLOBAL
	// #define SSP_RUNTIME
	#define ASYNC_SEND 1 // equal 1 is async send socket data
	#define DISABLE_GC_COLLECT_CYCLES
	#define MSG_QUEUE_MAX_MSGS 10000 // message queue for max message number
	#define MSG_QUEUE_NTHREADS 100 // message queue for max

	#define SOCKET_SNDTIMEO 10000
	#define SOCKET_SNDBUF 16*1024
	#define SOCKET_RCVTIMEO 10000
	#define SOCKET_RCVBUF 16*1024

	#define SOCKET_CONV_SNDTIMEO 10000
	#define SOCKET_CONV_SNDBUF 128*1024
	#define SOCKET_CONV_RCVTIMEO 10000
	#define SOCKET_CONV_RCVBUF 128*1024
#endif
