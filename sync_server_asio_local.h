#ifndef BWGAME_SYNC_SERVER_ASIO_LOCAL_H
#define BWGAME_SYNC_SERVER_ASIO_LOCAL_H

#include "sync_server_asio_socket.h"

namespace bwgame {

struct sync_server_asio_local: sync_server_asio_socket<asio::local::stream_protocol::socket> {
	struct acceptor_t {
		sync_server_asio_socket& server;
		asio::local::stream_protocol::acceptor acceptor{server.io_service};
		asio::local::stream_protocol::socket socket{server.io_service};
		acceptor_t(sync_server_asio_socket& server) : server(server) {}
	};

	void accept_handler(const asio::error_code& ec, std::shared_ptr<acceptor_t> acceptor) {
		if (!ec) new_connection_handler(std::move(acceptor->socket));
		auto* a = &*acceptor;
		a->acceptor.async_accept(a->socket, std::bind(&sync_server_asio_local::accept_handler, this, std::placeholders::_1, std::move(acceptor)));
	}
	
	void assign(const asio::local::stream_protocol::socket::native_handle_type& handle) {
		asio::local::stream_protocol::socket socket(io_service, asio::local::stream_protocol::socket::protocol_type(), handle);
		new_connection_handler(std::move(socket));
	}
	
	void bind(const asio::local::stream_protocol::endpoint& ep) {
		auto a = std::make_shared<acceptor_t>(*this);
		auto& acceptor = a->acceptor;
		acceptor.open();
		acceptor.bind(ep);
		acceptor.listen(asio::socket_base::max_connections);
		acceptor.async_accept(a->socket, std::bind(&sync_server_asio_local::accept_handler, this, std::placeholders::_1, a));
	}
	
	void connect(const asio::local::stream_protocol::endpoint& ep) {
		asio::local::stream_protocol::socket socket(io_service);
		socket.connect(ep);
		new_connection_handler(std::move(socket));
	}

	bool try_connect(const asio::local::stream_protocol::endpoint& ep) {
		asio::local::stream_protocol::socket socket(io_service);
		asio::error_code ec;
		socket.connect(ep, ec);
		if (ec) return false;
		new_connection_handler(std::move(socket));
		return true;
	}
	
	bool try_bind(const asio::local::stream_protocol::endpoint& ep) {
		auto a = std::make_shared<acceptor_t>(*this);
		auto& acceptor = a->acceptor;
		asio::error_code ec;
		acceptor.open();
		acceptor.bind(ep, ec);
		if (ec) return false;
		acceptor.listen(asio::socket_base::max_connections, ec);
		if (ec) return false;
		acceptor.async_accept(a->socket, std::bind(&sync_server_asio_local::accept_handler, this, std::placeholders::_1, a));
		return true;
	}
	
};

}

#endif
