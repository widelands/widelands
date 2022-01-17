/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_NETWORK_BUFFEREDCONNECTION_H
#define WL_NETWORK_BUFFEREDCONNECTION_H

#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include "network/network.h"
#include "network/relay_protocol.h"

/**
 * A wrapper around a network connection.
 * Does not contain logic but provides a buffer to read and write
 * RelayCommand / uint8_t / std::string / SendPacket from/to the network stream.
 *
 * Offers the Peeker class to check whether the required data has already been received
 * before trying to read the data.
 */
class BufferedConnection {
public:
	/**
	 * Allows to check whether the required data is completely in the buffer before starting to
	 * receive it.
	 *
	 * The idea of this methods is that the caller can check whether a complete command
	 * can be received before starting to remove data from the buffer.
	 * Otherwise, the caller would have to maintain their own buffer.
	 *
	 * The methods will not remove any bytes, but will check whether the required value can be read.
	 * After checking, an internal peek-pointer will be incremented. The next call of a method will
	 * then start reading after the previous value. Only a successful peek will move the pointer.
	 *
	 * Note that a successful peek does not mean that the received bytes really are of the requested
	 * type, it only means they could be interpreted that way. Whether the type matches is in the
	 * responsibility of the caller.
	 *
	 * Using any method of this class will trigger an internal read from the network socket inside
	 * the given BufferedConnection.
	 *
	 * \warning Calling any receive() method on the given connection will invalidate the Peeker.
	 */
	class Peeker {
	public:
		/**
		 * Creates a Peeker for the given connection.
		 * \warning Calling any receive() method on the given connection will invalidate the Peeker.
		 * \param conn The connection which should be peeked into.
		 * \note The Peeker instance does not own the given connection. It is the responsible of the
		 *       caller to make sure the given instance stays valid.
		 */
		explicit Peeker(BufferedConnection* conn);

		/**
		 * Checks whether a relay command can be read from the buffer.
		 * This method does not modify the buffer contents but increases the peek-pointer.
		 * \param out The command that will be returned next. It will not be removed from the input
		 * queue!
		 *            If \c false is returned the contents are not modified. Can be nullptr.
		 * \return \c True if the value can be read, \c false if not enough data has been received.
		 */
		bool cmd(RelayCommand* out = nullptr);

		/**
		 * Checks whether an uint8_t can be read from the buffer.
		 * This method does not modify the buffer contents but increases the peek-pointer.
		 * \param out The uint8_t that will be returned next. It will not be removed from the input
		 * queue!
		 *            If \c false is returned the contents are not modified. Can be nullptr.
		 * \return \c True if the value can be read, \c false if not enough data has been received.
		 */
		bool uint8_t(uint8_t* out = nullptr);

		/**
		 * Checks whether a std::string can be read from the buffer.
		 * This method does not modify the buffer contents but increases the peek-pointer.
		 * \return \c True if the value can be read, \c false if not enough data has been received.
		 */
		bool string();

		/**
		 * Checks whether a RecvPacket can be read from the buffer.
		 * This method does not modify the buffer contents but increases the peek-pointer.
		 * \return \c True if the value can be read, \c false if not enough data has been received.
		 */
		bool recvpacket();

	private:
		/// The connection to operate on.
		BufferedConnection* conn_;

		/// The position of the next peek.
		size_t peek_pointer_;
	};

	/**
	 * Tries to establish a connection to the given address.
	 * \param host The host to connect to.
	 * \return A pointer to a connected \c BufferedConnection object or a \c nullptr.
	 */
	static std::unique_ptr<BufferedConnection> connect(const NetAddress& host);

	/**
	 * Prepares a socket but does not connect anywhere.
	 * Connecting the socket has to be done by the caller, afterwards \c notify_connected() has to be
	 * called. \return A pair with a pointer to an unconnected \c BufferedConnection object and a
	 * pointer to the internal socket.
	 */
	static std::pair<std::unique_ptr<BufferedConnection>, asio::ip::tcp::socket*>
	create_unconnected();

	/**
	 * Informs this class that the internal socket has been connected to something
	 * by the caller.
	 */
	void notify_connected();

	/**
	 * Closes the connection.
	 * If you want to send a goodbye-message, do so before freeing the object.
	 */
	~BufferedConnection();

	/**
	 * Returns whether the connection is established.
	 * \return \c true if the connection is open, \c false otherwise.
	 */
	bool is_connected() const;

	/**
	 * Closes the connection.
	 * If you want to send a goodbye-message, do so before calling this.
	 */
	void close();

	/**
	 * Receive a command.
	 * \warning Calling this method is only safe when peek_cmd() returned \c true.
	 *          Otherwise the behavior of this method is undefined.
	 * \param out The variable to write the value to.
	 */
	void receive(RelayCommand* out);

	/**
	 * Receive an uint8_t.
	 * \warning Calling this method is only safe when peek_uint8_t() returned \c true.
	 *          Otherwise the behavior of this method is undefined.
	 * \param out The variable to write the value to.
	 */
	void receive(uint8_t* out);

	/**
	 * Receive a string.
	 * \warning Calling this method is only safe when peek_string() returned \c true.
	 *          Otherwise the behavior of this method is undefined.
	 * \param out The variable to write the value to.
	 */
	void receive(std::string* out);

	/**
	 * Receive a RecvPacket.
	 * \warning Calling this method is only safe when peek_recvpacket() returned \c true.
	 *          Otherwise the behavior of this method is undefined.
	 * \param out The variable to write the value to.
	 */
	void receive(RecvPacket* out);

	// Temporary method, will be removed when display of RTT measurements are implemented.
	// Removes a message from type kRoundTripTimeResponse from the buffer.
	void ignore_rtt_response();

	/**
	 * Sends data over the connection.
	 * @note All data that belongs together has to be send with one function call. If added with
	 *       multiple function calls the packet might be disrupted by a higher priority packet,
	 *       breaking the network protocol and leading to a disconnect.
	 * @param priority The priority this data has. A higher priority can be transmitted earlier,
	 *                 even when a large group of low priority data has been scheduled earlier.
	 * @param Fargs A list of data objects that should be send as part of this packet.
	 *              The following data types can be send:
	 *              RelayCommand, uint8_t, std::string, std::vector<uint8_t>, SendPacket.
	 */
	/*
	 This method creates a vector to store the data-to-be-send in. Basically this template magic
	 (called "parameter pack") is similar to the var-arg magic used by printf(). An arbitrary
	 number of arbitrary-typed parameters might be passed as the Fargs parameter. The compiler than
	 decides which of the send_T_() methods have to be called. Since each of them takes a different
	 first argument out of Fargs only one of them can match. The matching methods transforms the
	 received first argument to an uint8_t string and appends it to the given vector. Then it
	 calls send_T_() again, but with one argument less. This results in a "recursive" call
	 until no more arguments are left.
	 */
	template <typename... Targs> void send(NetPriority priority, const Targs&... Fargs) {

		std::vector<uint8_t> v;
		v.reserve(kNetworkBufferSize);

		send_T_(v, Fargs...);

		std::unique_lock<std::mutex> lock(mutex_send_);
		// The map will automatically create the vector for the requested priority if it does not
		// exist
		buffers_to_send_[priority].push(v);
		lock.unlock();
		start_sending();
	}

private:
	// I love this language... Sorry for the next functions,
	// but you have to admit that this is cool! :-D

	/**
	 * Base function that is called when no arguments are left.
	 */
	void send_T_(std::vector<uint8_t>&) {
	}

	/**
	 * Takes one element (here: a RelayCommand) and transforms it to an uint8_t.
	 * @param v The vector to add the data to.
	 * @param cmd The RelayCommand to transform.
	 * @param Fargs Further arguments that will be handled in the next iteration.
	 */
	/// @{
	template <typename... Targs>
	void send_T_(std::vector<uint8_t>& v, RelayCommand cmd, const Targs&... Fargs) {
		v.push_back(static_cast<uint8_t>(cmd));
		send_T_(v, Fargs...);
	}

	template <typename... Targs>
	void send_T_(std::vector<uint8_t>& v, uint8_t u, const Targs&... Fargs) {
		v.push_back(u);
		send_T_(v, Fargs...);
	}

	template <typename... Targs>
	void send_T_(std::vector<uint8_t>& v, const std::string& str, const Targs&... Fargs) {
		v.insert(v.end(), str.cbegin(), str.cend());
		v.push_back(0);
		send_T_(v, Fargs...);
	}

	template <typename... Targs>
	void send_T_(std::vector<uint8_t>& v, const std::vector<uint8_t>& data, const Targs&... Fargs) {
		v.insert(v.end(), data.begin(), data.end());
		send_T_(v, Fargs...);
	}

	template <typename... Targs>
	void send_T_(std::vector<uint8_t>& v, const SendPacket& packet, const Targs&... Fargs) {
		v.insert(v.end(), packet.get_data(), packet.get_data() + packet.get_size());
		send_T_(v, Fargs...);
	}
	/// @}

	/**
	 * Tries to establish a connection to the given host.
	 * If the connection attempt failed, is_connected() will return \c false.
	 * \param host The host to connect to.
	 */
	explicit BufferedConnection(const NetAddress& host);

	/**
	 * Prepares a socket but does not connect anywhere.
	 * Connecting the socket has to be done by the caller,
	 * afterwards \c notify_connected() must be called.
	 */
	explicit BufferedConnection();

	/**
	 * Tries to send some data.
	 * Is called by send() each time new data is given to this class but only
	 * does something when not already sending.
	 * Will continue sending until all buffers_to_send_ are empty.
	 */
	void start_sending();

	/**
	 * Waits on the socket until data can be received.
	 * After data has been received, directly calls itself again to start
	 * the next wait.
	 */
	void start_receiving();

	/**
	 * Reduces the send buffer of the given socket to only contain 20 packets.
	 * @param socket The socket to modify.
	 */
	static void reduce_send_buffer(asio::ip::tcp::socket& socket);

	/// The buffers that are waiting to be send.
	/// The map key is the priority of the packets stored in the queue.
	/// Each packet in the queue is a vector of uint8_t.
	std::map<uint8_t, std::queue<std::vector<uint8_t>>> buffers_to_send_;

	/// An io_service needed by asio. Primarily needed for asynchronous operations.
	asio::io_service io_service_;

	/// The socket that connects us to the host.
	asio::ip::tcp::socket socket_;

	/// Buffer for arriving data. We need to store it until we have enough
	/// to return the required type.
	std::deque<uint8_t> receive_buffer_;
	/// The buffer that is given to the asynchronous receive function
	uint8_t asio_receive_buffer_[kNetworkBufferSize];

	/// A thread used for the asynchronous send/receive methods
	std::thread asio_thread_;
	/// Protects buffers_to_send_
	std::mutex mutex_send_;
	/// Protects receive_buffer_
	std::mutex mutex_receive_;
	/// Whether we are currently sending something, used within start_sending()
	bool currently_sending_;
};

#endif  // end of include guard: WL_NETWORK_BUFFEREDCONNECTION_H
