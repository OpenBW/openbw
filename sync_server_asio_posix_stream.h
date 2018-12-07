#ifndef BWGAME_SYNC_SERVER_ASIO_POSIX_STREAM_H
#define BWGAME_SYNC_SERVER_ASIO_POSIX_STREAM_H

#include "sync_server_asio_socket.h"

namespace bwgame {

struct fd_socket {
	asio::posix::stream_descriptor in;
	asio::posix::stream_descriptor out;

	void close() {
		in.close();
		out.close();
	}

	bool is_open() const {
		return in.is_open() && out.is_open();
	}

	template<typename... args_T>
	auto async_read_some(args_T&&... args) {
		return in.async_read_some(std::forward<args_T>(args)...);
	}

	template<typename... args_T>
	auto async_write_some(args_T&&... args) {
		return out.async_write_some(std::forward<args_T>(args)...);
	}
};

struct sync_server_asio_posix_stream: sync_server_asio_socket<fd_socket> {

	void open(int read_fd, int write_fd) {
		asio::posix::stream_descriptor read(io_service, read_fd);
		asio::posix::stream_descriptor write(io_service, write_fd);
		new_connection_handler({std::move(read), std::move(write)});
	}

};

}

#endif
