/*! \file DiscoveryServer.hpp
	\brief Server side implementation of beacon functionality to register client nodes and provides info about other nodes (discovery)
*/

#ifndef MARLIN_BEACON_BEACON_HPP
#define MARLIN_BEACON_BEACON_HPP

#include <marlin/net/core/Timer.hpp>
#include <marlin/net/core/EventLoop.hpp>
#include <marlin/net/udp/UdpTransportFactory.hpp>
#include <map>

#include <sodium.h>

#include <spdlog/fmt/bin_to_hex.h>

namespace marlin {
namespace beacon {

//! Class implementing the server side node discovery functionality
/*!
	Features:
    \li Uses the custom marlin UDPTransport for message delivery
	\li HEARTBEAT - nodes which ping with a heartbeat are registered
	\li DISCPEER - nodes which ping with Discpeer are sent a list of peers
*/
template<typename DiscoveryServerDelegate>
class DiscoveryServer {
private:
	using Self = DiscoveryServer<DiscoveryServerDelegate>;

	using BaseTransportFactory = net::UdpTransportFactory<
		DiscoveryServer<DiscoveryServerDelegate>,
		DiscoveryServer<DiscoveryServerDelegate>
	>;
	using BaseTransport = net::UdpTransport<
		DiscoveryServer<DiscoveryServerDelegate>
	>;

	BaseTransportFactory f;

	// Discovery protocol
	void did_recv_DISCPROTO(BaseTransport &transport);
	void send_LISTPROTO(BaseTransport &transport);

	void did_recv_DISCPEER(BaseTransport &transport);
	void send_LISTPEER(BaseTransport &transport);

	void did_recv_HEARTBEAT(BaseTransport &transport, net::Buffer &&bytes);

	void heartbeat_timer_cb();
	net::Timer<Self, &Self::heartbeat_timer_cb> heartbeat_timer;

	std::unordered_map<BaseTransport *, std::pair<uint64_t, std::array<uint8_t, 32>>> peers;
	// Can we change about implementation to tuple instead of tuple and use get<> instead of second?
	std::unordered_map<BaseTransport *, std::array<uint8_t,20>> peerCrypAddr; 
public:
	// Listen delegate
	bool should_accept(net::SocketAddress const &addr);
	void did_create_transport(BaseTransport &transport);

	// Transport delegate
	void did_dial(BaseTransport &transport);
	void did_recv_packet(BaseTransport &transport, net::Buffer &&packet);
	void did_send_packet(BaseTransport &transport, net::Buffer &&packet);

	DiscoveryServer(net::SocketAddress const &addr);

	DiscoveryServerDelegate *delegate;
};


// Impl

//---------------- Discovery protocol functions begin ----------------//


/*!
	\li Callback on receipt of disc proto
	\li Sends back the protocols supported on this node
*/
template<typename DiscoveryServerDelegate>
void DiscoveryServer<DiscoveryServerDelegate>::did_recv_DISCPROTO(
	BaseTransport &transport
) {
	SPDLOG_DEBUG("DISCPROTO <<< {}", transport.dst_addr.to_string());

	send_LISTPROTO(transport);
}

template<typename DiscoveryServerDelegate>
void DiscoveryServer<DiscoveryServerDelegate>::send_LISTPROTO(
	BaseTransport &transport
) {
	char *message = new char[2] {0, 1};

	net::Buffer p(message, 2);
	transport.send(std::move(p));
}


/*!
	\li Callback on receipt of disc peer
	\li Sends back the list of peers
*/
template<typename DiscoveryServerDelegate>
void DiscoveryServer<DiscoveryServerDelegate>::did_recv_DISCPEER(
	BaseTransport &transport
) {
	SPDLOG_DEBUG("DISCPEER <<< {}", transport.dst_addr.to_string());

	send_LISTPEER(transport);
}


/*!
	sends the list of peers on this node

\verbatim

0               1               2               3
0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
+++++++++++++++++++++++++++++++++
|      0x00     |      0x03     |
---------------------------------
|            AF_INET            |
-----------------------------------------------------------------
|                        IPv4 Address (1)                       |
-----------------------------------------------------------------
|            Port (1)           |
---------------------------------
|            AF_INET            |
-----------------------------------------------------------------
|                        IPv4 Address (2)                       |
-----------------------------------------------------------------
|            Port (2)           |
-----------------------------------------------------------------
|                              ...                              |
-----------------------------------------------------------------
|            AF_INET            |
-----------------------------------------------------------------
|                        IPv4 Address (N)                       |
-----------------------------------------------------------------
|            Port (N)           |
+++++++++++++++++++++++++++++++++

\endverbatim
*/
template<typename DiscoveryServerDelegate>
void DiscoveryServer<DiscoveryServerDelegate>::send_LISTPEER(
	BaseTransport &transport
) {
	auto iter = peers.begin(); auto iter0 = peerCrypAddr.begin();

	while(iter != peers.end() && iter0 != peerCrypAddr.begin()) {
		char *message = new char[1100] {0, 3};
		size_t size = 2;

		for(
			;
			iter != peers.end() && size + 7 + crypto_box_PUBLICKEYBYTES +20 < 1100 && iter0 != peerCrypAddr.end();
			iter++
		) {
			if(iter->first == &transport) continue;

			iter->first->dst_addr.serialize(message+size, 8);
			std::memcpy(message+size+8, iter->second.second.data(), crypto_box_PUBLICKEYBYTES);
			std::memcpy(message+size+8+crypto_box_PUBLICKEYBYTES, iter0->second.data(),20);
			size += 8 + crypto_box_PUBLICKEYBYTES + 20;
			iter0++;
		}

		net::Buffer p(message, size);
		transport.send(std::move(p));
	}
}

/*!
	\li Callback on receipt of heartbeat from a client node
	\li Refreshes the entry of the node with current timestamp
*/
template<typename DiscoveryServerDelegate>
void DiscoveryServer<DiscoveryServerDelegate>::did_recv_HEARTBEAT(
	BaseTransport &transport,
	net::Buffer &&bytes
) {
	SPDLOG_DEBUG(
		"HEARTBEAT <<< {}, {:spn}",
		transport.dst_addr.to_string(),
		spdlog::to_hex(bytes.data()+2, bytes.data()+34)
	);

	peers[&transport].first = net::EventLoop::now();
	std::memcpy(peers[&transport].second.data(), bytes.data()+2, crypto_box_PUBLICKEYBYTES);
	std::memcpy(peerCrypAddr[&transport].data(), bytes.data()+2+crypto_box_PUBLICKEYBYTES, 20);
	SPDLOG_DEBUG("Received <<< {:spn}",spdlog::to_hex(peerCrypAddr[&transport].data(),peerCrypAddr[&transport].data()+20));
}


/*!
	callback to periodically cleanup the old peers which have been inactive for more than a minute (inactive = not received heartbeat)
*/
template<typename DiscoveryServerDelegate>
void DiscoveryServer<DiscoveryServerDelegate>::heartbeat_timer_cb() {
	auto now = net::EventLoop::now();

	auto iter = peers.begin();auto iter0 = peerCrypAddr.begin();
	while(iter != peers.end() and iter0 != peerCrypAddr.end()) {
		// Remove stale peers if inactive for a minute
		if(now - iter->second.first > 60000) {
			iter = peers.erase(iter);
			iter0 = peerCrypAddr.erase(iter0);
		} else {
			iter++;
			iter0++;
		}
	}
}

//---------------- Discovery protocol functions begin ----------------//


//---------------- Listen delegate functions begin ----------------//

template<typename DiscoveryServerDelegate>
bool DiscoveryServer<DiscoveryServerDelegate>::should_accept(
	net::SocketAddress const &
) {
	return true;
}

template<typename DiscoveryServerDelegate>
void DiscoveryServer<DiscoveryServerDelegate>::did_create_transport(
	BaseTransport &transport
) {
	transport.setup(this);
}

//---------------- Listen delegate functions end ----------------//


//---------------- Transport delegate functions begin ----------------//

template<typename DiscoveryServerDelegate>
void DiscoveryServer<DiscoveryServerDelegate>::did_dial(
	BaseTransport &
) {}


//! receives the packet and processes them
/*!
	Determines the type of packet by reading the first byte and redirects the packet to appropriate function for further processing

	\li \b first-byte	:	\b type
	\li 0			:	DISCPROTO
	\li 1			:	ERROR - LISTPROTO, meant for client
	\li 2			:	DISCPEER
	\li 3			:	ERROR - LISTPEER, meant for client
	\li 4			:	HEARTBEAT
*/
template<typename DiscoveryServerDelegate>
void DiscoveryServer<DiscoveryServerDelegate>::did_recv_packet(
	BaseTransport &transport,
	net::Buffer &&packet
) {
	switch(packet.read_uint8(1)) {
		// DISCPROTO
		case 0: did_recv_DISCPROTO(transport);
		break;
		// LISTPROTO
		case 1: SPDLOG_ERROR("Unexpected LISTPROTO from {}", transport.dst_addr.to_string());
		break;
		// DISCOVER
		case 2: did_recv_DISCPEER(transport);
		break;
		// PEERLIST
		case 3: SPDLOG_ERROR("Unexpected LISTPEER from {}", transport.dst_addr.to_string());
		break;
		// HEARTBEAT
		case 4: did_recv_HEARTBEAT(transport, std::move(packet));
		break;
		// UNKNOWN
		default: SPDLOG_TRACE("UNKNOWN <<< {}", transport.dst_addr.to_string());
		break;
	}
}

template<typename DiscoveryServerDelegate>
void DiscoveryServer<DiscoveryServerDelegate>::did_send_packet(
	BaseTransport &transport __attribute__((unused)),
	net::Buffer &&packet
) {
	switch(packet.read_uint8(1)) {
		// DISCPROTO
		case 0: SPDLOG_TRACE("DISCPROTO >>> {}", transport.dst_addr.to_string());
		break;
		// LISTPROTO
		case 1: SPDLOG_TRACE("LISTPROTO >>> {}", transport.dst_addr.to_string());
		break;
		// DISCPEER
		case 2: SPDLOG_TRACE("DISCPEER >>> {}", transport.dst_addr.to_string());
		break;
		// LISTPEER
		case 3: SPDLOG_TRACE("LISTPEER >>> {}", transport.dst_addr.to_string());
		break;
		// HEARTBEAT
		case 4: SPDLOG_TRACE("HEARTBEAT >>> {}", transport.dst_addr.to_string());
		break;
		// UNKNOWN
		default: SPDLOG_TRACE("UNKNOWN >>> {}", transport.dst_addr.to_string());
		break;
	}
}

//---------------- Transport delegate functions end ----------------//

template<typename DiscoveryServerDelegate>
DiscoveryServer<DiscoveryServerDelegate>::DiscoveryServer(
	net::SocketAddress const &addr
) : heartbeat_timer(this) {
	f.bind(addr);
	f.listen(*this);

	heartbeat_timer.start(10000, 10000);
}

} // namespace beacon
} // namespace marlin

#endif // MARLIN_BEACON_BEACON_HPP
