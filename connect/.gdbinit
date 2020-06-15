set breakpoint pending on

# connect test
set args -port 9998 -stop-reconnect-on 3

# received greeting from peer
#b zmtp_engine.cpp:166

# return protocol_error if handshake times out and bytes read >0
#b stream_engine_base.cpp:732

# connection closed by peer
#b stream_engine_base.cpp:762

# stop reconnecting on failed handshake
#b stream_engine_base.cpp:690

# stop reconnecting if connect returns ECONNREFUSED
#b tcp_connecter.cpp:109

# read from tcp socket
#b tcp.cpp:304


#b zmq::stream_engine_base_t::timer_event
#b zmq::stream_engine_base_t::set_handshake_timer
#b zmq::tcp_connecter_t::connect
#b zmq::zmtp_engine_t::plug_internal
#b zmq::zmtp_engine_t::handshake
#b zmq::zmtp_engine_t::handshake_v1_0_unversioned
#b zmq::zmtp_engine_t::handshake_v1_0
#b zmq::zmtp_engine_t::handshake_v2_0
#b zmq::zmtp_engine_t::handshake_v3_x
#b zmq::zmtp_engine_t::handshake_v3_0
#b zmq::zmtp_engine_t::handshake_v3_1
#b zmq::socket_base_t::close
#b zmq::socket_base_t::~socket_base_t

# socket events
#b zmq::socket_base_t::event_connected
#b zmq::socket_base_t::event_connect_delayed
#b zmq::socket_base_t::event_connect_retried
#b zmq::socket_base_t::event_listening
#b zmq::socket_base_t::event_bind_failed
#b zmq::socket_base_t::event_accepted
#b zmq::socket_base_t::event_accept_failed
#b zmq::socket_base_t::event_closed
#b zmq::socket_base_t::event_close_failed
b zmq::socket_base_t::event_disconnected
b zmq::socket_base_t::event_handshake_failed_no_detail
#b zmq::socket_base_t::event_handshake_failed_protocol
#b zmq::socket_base_t::event_handshake_failed_auth
#b zmq::socket_base_t::event_handshake_succeeded
