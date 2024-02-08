/* THIS FILE IS GENERATED BY gen_metrics.py. DO NOT HAND EDIT. */

#include "../fd_metrics_base.h"

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_OFF  (173UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_CNT  (5UL)

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_SUCCESS_OFF  (173UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_SUCCESS_NAME "quic_tile_non_quic_reassembly_append_success"
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_SUCCESS_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_SUCCESS_DESC "Result of fragment reassembly for a non-QUIC UDP transaction. (Success)"

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_OVERSIZE_OFF  (174UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_OVERSIZE_NAME "quic_tile_non_quic_reassembly_append_error_oversize"
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_OVERSIZE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_OVERSIZE_DESC "Result of fragment reassembly for a non-QUIC UDP transaction. (Oversize message)"

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_SKIP_OFF  (175UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_SKIP_NAME "quic_tile_non_quic_reassembly_append_error_skip"
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_SKIP_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_SKIP_DESC "Result of fragment reassembly for a non-QUIC UDP transaction. (Out-of-order data within QUIC stream)"

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_TRANSACTION_OFF  (176UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_TRANSACTION_NAME "quic_tile_non_quic_reassembly_append_error_transaction"
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_TRANSACTION_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_TRANSACTION_DESC "Result of fragment reassembly for a non-QUIC UDP transaction. (Rejected transaction)"

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_STATE_OFF  (177UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_STATE_NAME "quic_tile_non_quic_reassembly_append_error_state"
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_STATE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_APPEND_ERROR_STATE_DESC "Result of fragment reassembly for a non-QUIC UDP transaction. (Unexpected slot state)"

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_OFF  (178UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_CNT  (5UL)

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_SUCCESS_OFF  (178UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_SUCCESS_NAME "quic_tile_non_quic_reassembly_publish_success"
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_SUCCESS_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_SUCCESS_DESC "Result of publishing reassmbled fragment for a non-QUIC UDP transaction. (Success)"

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_OVERSIZE_OFF  (179UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_OVERSIZE_NAME "quic_tile_non_quic_reassembly_publish_error_oversize"
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_OVERSIZE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_OVERSIZE_DESC "Result of publishing reassmbled fragment for a non-QUIC UDP transaction. (Oversize message)"

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_SKIP_OFF  (180UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_SKIP_NAME "quic_tile_non_quic_reassembly_publish_error_skip"
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_SKIP_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_SKIP_DESC "Result of publishing reassmbled fragment for a non-QUIC UDP transaction. (Out-of-order data within QUIC stream)"

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_TRANSACTION_OFF  (181UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_TRANSACTION_NAME "quic_tile_non_quic_reassembly_publish_error_transaction"
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_TRANSACTION_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_TRANSACTION_DESC "Result of publishing reassmbled fragment for a non-QUIC UDP transaction. (Rejected transaction)"

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_STATE_OFF  (182UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_STATE_NAME "quic_tile_non_quic_reassembly_publish_error_state"
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_STATE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_REASSEMBLY_PUBLISH_ERROR_STATE_DESC "Result of publishing reassmbled fragment for a non-QUIC UDP transaction. (Unexpected slot state)"

#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_OFF  (183UL)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_CNT  (5UL)

#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_SUCCESS_OFF  (183UL)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_SUCCESS_NAME "quic_tile_reassembly_append_success"
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_SUCCESS_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_SUCCESS_DESC "Result of fragment reassembly for a QUIC transaction. (Success)"

#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_OVERSIZE_OFF  (184UL)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_OVERSIZE_NAME "quic_tile_reassembly_append_error_oversize"
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_OVERSIZE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_OVERSIZE_DESC "Result of fragment reassembly for a QUIC transaction. (Oversize message)"

#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_SKIP_OFF  (185UL)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_SKIP_NAME "quic_tile_reassembly_append_error_skip"
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_SKIP_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_SKIP_DESC "Result of fragment reassembly for a QUIC transaction. (Out-of-order data within QUIC stream)"

#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_TRANSACTION_OFF  (186UL)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_TRANSACTION_NAME "quic_tile_reassembly_append_error_transaction"
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_TRANSACTION_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_TRANSACTION_DESC "Result of fragment reassembly for a QUIC transaction. (Rejected transaction)"

#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_STATE_OFF  (187UL)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_STATE_NAME "quic_tile_reassembly_append_error_state"
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_STATE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_APPEND_ERROR_STATE_DESC "Result of fragment reassembly for a QUIC transaction. (Unexpected slot state)"

#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_OFF  (188UL)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_CNT  (5UL)

#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_SUCCESS_OFF  (188UL)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_SUCCESS_NAME "quic_tile_reassembly_publish_success"
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_SUCCESS_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_SUCCESS_DESC "Result of publishing reassmbled fragment for a QUIC transaction. (Success)"

#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_OVERSIZE_OFF  (189UL)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_OVERSIZE_NAME "quic_tile_reassembly_publish_error_oversize"
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_OVERSIZE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_OVERSIZE_DESC "Result of publishing reassmbled fragment for a QUIC transaction. (Oversize message)"

#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_SKIP_OFF  (190UL)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_SKIP_NAME "quic_tile_reassembly_publish_error_skip"
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_SKIP_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_SKIP_DESC "Result of publishing reassmbled fragment for a QUIC transaction. (Out-of-order data within QUIC stream)"

#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_TRANSACTION_OFF  (191UL)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_TRANSACTION_NAME "quic_tile_reassembly_publish_error_transaction"
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_TRANSACTION_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_TRANSACTION_DESC "Result of publishing reassmbled fragment for a QUIC transaction. (Rejected transaction)"

#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_STATE_OFF  (192UL)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_STATE_NAME "quic_tile_reassembly_publish_error_state"
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_STATE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_PUBLISH_ERROR_STATE_DESC "Result of publishing reassmbled fragment for a QUIC transaction. (Unexpected slot state)"

#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_NOTIFY_CLOBBERED_OFF  (193UL)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_NOTIFY_CLOBBERED_NAME "quic_tile_reassembly_notify_clobbered"
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_NOTIFY_CLOBBERED_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_REASSEMBLY_NOTIFY_CLOBBERED_DESC "Reassembly slot was clobbered before it was notified."

#define FD_METRICS_COUNTER_QUIC_TILE_QUIC_PACKET_TOO_SMALL_OFF  (194UL)
#define FD_METRICS_COUNTER_QUIC_TILE_QUIC_PACKET_TOO_SMALL_NAME "quic_tile_quic_packet_too_small"
#define FD_METRICS_COUNTER_QUIC_TILE_QUIC_PACKET_TOO_SMALL_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_QUIC_PACKET_TOO_SMALL_DESC "Count of packets received on the QUIC port that were too small to be a valid IP packet."

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_PACKET_TOO_SMALL_OFF  (195UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_PACKET_TOO_SMALL_NAME "quic_tile_non_quic_packet_too_small"
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_PACKET_TOO_SMALL_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_PACKET_TOO_SMALL_DESC "Count of packets received on the non-QUIC port that were too small to be a valid IP packet."

#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_PACKET_TOO_LARGE_OFF  (196UL)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_PACKET_TOO_LARGE_NAME "quic_tile_non_quic_packet_too_large"
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_PACKET_TOO_LARGE_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_TILE_NON_QUIC_PACKET_TOO_LARGE_DESC "Count of packets received on the non-QUIC port that were too large to be a valid transaction."

#define FD_METRICS_HISTOGRAM_QUIC_TILE_FRAG_PROCESSING_TIME_OFF  (197UL)
#define FD_METRICS_HISTOGRAM_QUIC_TILE_FRAG_PROCESSING_TIME_NAME "quic_tile_frag_processing_time"
#define FD_METRICS_HISTOGRAM_QUIC_TILE_FRAG_PROCESSING_TIME_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_QUIC_TILE_FRAG_PROCESSING_TIME_DESC "Duration in seconds of the fragment processing time within QUIC"
#define FD_METRICS_HISTOGRAM_QUIC_TILE_FRAG_PROCESSING_TIME_MIN  (0.0)
#define FD_METRICS_HISTOGRAM_QUIC_TILE_FRAG_PROCESSING_TIME_MAX  (0.0001)
#define FD_METRICS_HISTOGRAM_QUIC_TILE_FRAG_PROCESSING_TIME_CVT  (FD_METRICS_CONVERTER_SECONDS)

#define FD_METRICS_HISTOGRAM_QUIC_TILE_FINAL_PROCESSING_TIME_OFF  (214UL)
#define FD_METRICS_HISTOGRAM_QUIC_TILE_FINAL_PROCESSING_TIME_NAME "quic_tile_final_processing_time"
#define FD_METRICS_HISTOGRAM_QUIC_TILE_FINAL_PROCESSING_TIME_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_QUIC_TILE_FINAL_PROCESSING_TIME_DESC "Duration in seconds of the final fragment processing time within QUIC"
#define FD_METRICS_HISTOGRAM_QUIC_TILE_FINAL_PROCESSING_TIME_MIN  (0.0)
#define FD_METRICS_HISTOGRAM_QUIC_TILE_FINAL_PROCESSING_TIME_MAX  (0.0001)
#define FD_METRICS_HISTOGRAM_QUIC_TILE_FINAL_PROCESSING_TIME_CVT  (FD_METRICS_CONVERTER_SECONDS)

#define FD_METRICS_COUNTER_QUIC_RECEIVED_PACKETS_OFF  (231UL)
#define FD_METRICS_COUNTER_QUIC_RECEIVED_PACKETS_NAME "quic_received_packets"
#define FD_METRICS_COUNTER_QUIC_RECEIVED_PACKETS_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_RECEIVED_PACKETS_DESC "Number of IP packets received."

#define FD_METRICS_COUNTER_QUIC_RECEIVED_BYTES_OFF  (232UL)
#define FD_METRICS_COUNTER_QUIC_RECEIVED_BYTES_NAME "quic_received_bytes"
#define FD_METRICS_COUNTER_QUIC_RECEIVED_BYTES_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_RECEIVED_BYTES_DESC "Total bytes received (including IP, UDP, QUIC headers)."

#define FD_METRICS_COUNTER_QUIC_SENT_PACKETS_OFF  (233UL)
#define FD_METRICS_COUNTER_QUIC_SENT_PACKETS_NAME "quic_sent_packets"
#define FD_METRICS_COUNTER_QUIC_SENT_PACKETS_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_SENT_PACKETS_DESC "Number of IP packets sent."

#define FD_METRICS_COUNTER_QUIC_SENT_BYTES_OFF  (234UL)
#define FD_METRICS_COUNTER_QUIC_SENT_BYTES_NAME "quic_sent_bytes"
#define FD_METRICS_COUNTER_QUIC_SENT_BYTES_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_SENT_BYTES_DESC "Total bytes sent (including IP, UDP, QUIC headers)."

#define FD_METRICS_GAUGE_QUIC_CONNECTIONS_ACTIVE_OFF  (235UL)
#define FD_METRICS_GAUGE_QUIC_CONNECTIONS_ACTIVE_NAME "quic_connections_active"
#define FD_METRICS_GAUGE_QUIC_CONNECTIONS_ACTIVE_TYPE (FD_METRICS_TYPE_GAUGE)
#define FD_METRICS_GAUGE_QUIC_CONNECTIONS_ACTIVE_DESC "The number of currently active QUIC connections."

#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_CREATED_OFF  (236UL)
#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_CREATED_NAME "quic_connections_created"
#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_CREATED_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_CREATED_DESC "The total number of connections that have been created."

#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_CLOSED_OFF  (237UL)
#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_CLOSED_NAME "quic_connections_closed"
#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_CLOSED_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_CLOSED_DESC "Number of connections gracefully closed."

#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_ABORTED_OFF  (238UL)
#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_ABORTED_NAME "quic_connections_aborted"
#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_ABORTED_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_ABORTED_DESC "Number of connections aborted."

#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_RETRIED_OFF  (239UL)
#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_RETRIED_NAME "quic_connections_retried"
#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_RETRIED_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_CONNECTIONS_RETRIED_DESC "Number of connections established with retry."

#define FD_METRICS_COUNTER_QUIC_CONNECTION_ERROR_NO_SLOTS_OFF  (240UL)
#define FD_METRICS_COUNTER_QUIC_CONNECTION_ERROR_NO_SLOTS_NAME "quic_connection_error_no_slots"
#define FD_METRICS_COUNTER_QUIC_CONNECTION_ERROR_NO_SLOTS_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_CONNECTION_ERROR_NO_SLOTS_DESC "Number of connections that failed to create due to lack of slots."

#define FD_METRICS_COUNTER_QUIC_CONNECTION_ERROR_TLS_FAIL_OFF  (241UL)
#define FD_METRICS_COUNTER_QUIC_CONNECTION_ERROR_TLS_FAIL_NAME "quic_connection_error_tls_fail"
#define FD_METRICS_COUNTER_QUIC_CONNECTION_ERROR_TLS_FAIL_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_CONNECTION_ERROR_TLS_FAIL_DESC "Number of connections that aborted due to TLS failure."

#define FD_METRICS_COUNTER_QUIC_CONNECTION_ERROR_RETRY_FAIL_OFF  (242UL)
#define FD_METRICS_COUNTER_QUIC_CONNECTION_ERROR_RETRY_FAIL_NAME "quic_connection_error_retry_fail"
#define FD_METRICS_COUNTER_QUIC_CONNECTION_ERROR_RETRY_FAIL_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_CONNECTION_ERROR_RETRY_FAIL_DESC "Number of connections that failed during retry (e.g. invalid token)."

#define FD_METRICS_COUNTER_QUIC_HANDSHAKES_CREATED_OFF  (243UL)
#define FD_METRICS_COUNTER_QUIC_HANDSHAKES_CREATED_NAME "quic_handshakes_created"
#define FD_METRICS_COUNTER_QUIC_HANDSHAKES_CREATED_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_HANDSHAKES_CREATED_DESC "Number of handshake flows created."

#define FD_METRICS_COUNTER_QUIC_HANDSHAKE_ERROR_ALLOC_FAIL_OFF  (244UL)
#define FD_METRICS_COUNTER_QUIC_HANDSHAKE_ERROR_ALLOC_FAIL_NAME "quic_handshake_error_alloc_fail"
#define FD_METRICS_COUNTER_QUIC_HANDSHAKE_ERROR_ALLOC_FAIL_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_HANDSHAKE_ERROR_ALLOC_FAIL_DESC "Number of handshakes dropped due to alloc fail."

#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_OFF  (245UL)
#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_CNT  (4UL)

#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_BIDI_CLIENT_OFF  (245UL)
#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_BIDI_CLIENT_NAME "quic_stream_opened_bidi_client"
#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_BIDI_CLIENT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_BIDI_CLIENT_DESC "Number of streams opened. (Bidirectional client)"

#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_BIDI_SERVER_OFF  (246UL)
#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_BIDI_SERVER_NAME "quic_stream_opened_bidi_server"
#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_BIDI_SERVER_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_BIDI_SERVER_DESC "Number of streams opened. (Bidirectional server)"

#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_UNI_CLIENT_OFF  (247UL)
#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_UNI_CLIENT_NAME "quic_stream_opened_uni_client"
#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_UNI_CLIENT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_UNI_CLIENT_DESC "Number of streams opened. (Unidirectional client)"

#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_UNI_SERVER_OFF  (248UL)
#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_UNI_SERVER_NAME "quic_stream_opened_uni_server"
#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_UNI_SERVER_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_STREAM_OPENED_UNI_SERVER_DESC "Number of streams opened. (Unidirectional server)"

#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_OFF  (249UL)
#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_CNT  (4UL)

#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_BIDI_CLIENT_OFF  (249UL)
#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_BIDI_CLIENT_NAME "quic_stream_closed_bidi_client"
#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_BIDI_CLIENT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_BIDI_CLIENT_DESC "Number of streams closed. (Bidirectional client)"

#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_BIDI_SERVER_OFF  (250UL)
#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_BIDI_SERVER_NAME "quic_stream_closed_bidi_server"
#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_BIDI_SERVER_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_BIDI_SERVER_DESC "Number of streams closed. (Bidirectional server)"

#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_UNI_CLIENT_OFF  (251UL)
#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_UNI_CLIENT_NAME "quic_stream_closed_uni_client"
#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_UNI_CLIENT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_UNI_CLIENT_DESC "Number of streams closed. (Unidirectional client)"

#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_UNI_SERVER_OFF  (252UL)
#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_UNI_SERVER_NAME "quic_stream_closed_uni_server"
#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_UNI_SERVER_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_STREAM_CLOSED_UNI_SERVER_DESC "Number of streams closed. (Unidirectional server)"

#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_OFF  (253UL)
#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_CNT  (4UL)

#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_BIDI_CLIENT_OFF  (253UL)
#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_BIDI_CLIENT_NAME "quic_stream_active_bidi_client"
#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_BIDI_CLIENT_TYPE (FD_METRICS_TYPE_GAUGE)
#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_BIDI_CLIENT_DESC "Number of active streams. (Bidirectional client)"

#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_BIDI_SERVER_OFF  (254UL)
#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_BIDI_SERVER_NAME "quic_stream_active_bidi_server"
#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_BIDI_SERVER_TYPE (FD_METRICS_TYPE_GAUGE)
#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_BIDI_SERVER_DESC "Number of active streams. (Bidirectional server)"

#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_UNI_CLIENT_OFF  (255UL)
#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_UNI_CLIENT_NAME "quic_stream_active_uni_client"
#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_UNI_CLIENT_TYPE (FD_METRICS_TYPE_GAUGE)
#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_UNI_CLIENT_DESC "Number of active streams. (Unidirectional client)"

#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_UNI_SERVER_OFF  (256UL)
#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_UNI_SERVER_NAME "quic_stream_active_uni_server"
#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_UNI_SERVER_TYPE (FD_METRICS_TYPE_GAUGE)
#define FD_METRICS_GAUGE_QUIC_STREAM_ACTIVE_UNI_SERVER_DESC "Number of active streams. (Unidirectional server)"

#define FD_METRICS_COUNTER_QUIC_STREAM_RECEIVED_EVENTS_OFF  (257UL)
#define FD_METRICS_COUNTER_QUIC_STREAM_RECEIVED_EVENTS_NAME "quic_stream_received_events"
#define FD_METRICS_COUNTER_QUIC_STREAM_RECEIVED_EVENTS_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_STREAM_RECEIVED_EVENTS_DESC "Number of stream RX events."

#define FD_METRICS_COUNTER_QUIC_STREAM_RECEIVED_BYTES_OFF  (258UL)
#define FD_METRICS_COUNTER_QUIC_STREAM_RECEIVED_BYTES_NAME "quic_stream_received_bytes"
#define FD_METRICS_COUNTER_QUIC_STREAM_RECEIVED_BYTES_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_QUIC_STREAM_RECEIVED_BYTES_DESC "Total stream payload bytes received."


#define FD_METRICS_QUIC_TOTAL (54UL)
extern const fd_metrics_meta_t FD_METRICS_QUIC[FD_METRICS_QUIC_TOTAL];
