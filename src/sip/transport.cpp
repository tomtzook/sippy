
#include <sip/transport.h>

namespace sippy::sip {

tcp_channel::tcp_channel(const looper::tcp tcp)
    : m_tcp(tcp)
    , m_read_callback()
    , m_error_callback()
{}

tcp_channel::~tcp_channel() {
    if (m_tcp != looper::empty_handle) {
        looper::destroy_tcp(m_tcp);
        m_tcp = looper::empty_handle;
    }
}

void tcp_channel::on_read(read_callback&& callback) {
    m_read_callback = std::move(callback);
}

void tcp_channel::on_error(error_callback&& callback) {
    m_error_callback = std::move(callback);
}

void tcp_channel::start_read() {
    looper::start_tcp_read(m_tcp, [this](looper::loop, looper::handle, const std::span<const uint8_t> data, const looper::error error)-> void {
        if (error != 0) {
            m_error_callback(error);
        } else {
            auto msg = parse(data);
            m_read_callback(std::move(msg));
        }
    });
}

void tcp_channel::send(message_ptr&& message) {
    uint8_t buffer[4096] = {};
    const auto written = write({buffer, sizeof(buffer)}, std::move(message));
    if (written < 0) {
        throw std::runtime_error("write size peek failed");
    }

    looper::write_tcp(m_tcp, {buffer, static_cast<size_t>(written)}, [this](looper::loop, looper::handle, const looper::error error)-> void {
        if (error != 0) {
            m_error_callback(error);
        }
    });
}

tcp_transport::tcp_transport(const looper::loop loop)
    : m_loop(loop) {
}

transport tcp_transport::type() const {
    return transport::tcp;
}

void tcp_transport::open(const connection_info& info, open_callback&& callback) {
    const auto tcp = looper::create_tcp(m_loop);
    looper::bind_tcp(tcp, info.local_address, info.local_port);
    looper::connect_tcp(tcp, info.remote_address, info.remote_port, [callback](looper::loop, const looper::handle tcp_handle, const looper::error error)-> void {
        if (error != 0) {
            callback(channel_ptr(), error);
        } else {
            callback(std::make_unique<tcp_channel>(tcp_handle), 0);
        }
    });
}

}
