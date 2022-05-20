#pragma once

#define BOOST_ASIO_NO_DEPRECATED
#define BOOST_ASIO_DISABLE_BUFFER_DEBUGGING
#define BOOST_ASIO_NO_TYPEID

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/NetBase.hpp>
#include <zec_ext/IO/Packet.hpp>
#include <zec_ext/IO/PacketBuffer.hpp>

ZEC_NS
{

    namespace asio        = boost::asio;
    namespace beast       = boost::beast;
    namespace ip          = boost::asio::ip;
    namespace websocket   = boost::beast::websocket;

    using tcp   = boost::asio::ip::tcp;
    using udp   = boost::asio::ip::udp;

    using xAsioError = boost::system::error_code;

    using xNativeIoContext = asio::io_context;
    using xTcpEndpoint = tcp::endpoint;
    using xUdpEndpoint = udp::endpoint;

    using xAsioConstBuffer = boost::asio::const_buffer;
    using xAsioMutableBuffer = boost::asio::mutable_buffer;
    using xBeastDynamicBuffer = boost::beast::multi_buffer;
    
    class xIoCaster final
    : xNonCopyable
    {
    public:
        ZEC_INLINE xNativeIoContext & operator ()(xIoContext & IoContext) const { return IoContext._Native.As<xNativeIoContext>(); }
    };

}
