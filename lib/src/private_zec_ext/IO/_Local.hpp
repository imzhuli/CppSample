#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/WebSocket.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;

ZEC_NS
{

    ZEC_STATIC_INLINE asio::io_context * Real(const xIoContext * IoContextPtr) { return (asio::io_context *)IoContextPtr->Native(); }

}
