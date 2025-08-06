#pragma once

#include <functional>
#include <memory>

#include <looper_types.h>
#include <looper_tcp.h>

#include <sip/types.h>
#include <sip/message.h>

namespace sippy::sip {

struct connection_info {
    std::string local_address;
    uint16_t local_port;

    std::string remote_address;
    uint16_t remote_port;
};

class channel {
public:
    using read_callback = std::function<void(message_ptr&&)>;
    using error_callback = std::function<void(uint64_t)>;

    virtual ~channel() = default;

    virtual void on_read(read_callback&& callback) = 0;
    virtual void on_error(error_callback&& callback) = 0;

    virtual void start_read() = 0;
    virtual void send(message_ptr&& message) = 0;
};

using channel_ptr = std::shared_ptr<channel>;

class transport_container {
public:
    using open_callback = std::function<void(channel_ptr&&, uint64_t)>;

    virtual ~transport_container() = default;

    [[nodiscard]] virtual transport type() const = 0;

    virtual void open(const connection_info& info, open_callback&& callback) = 0;
};

using transport_container_ptr = std::shared_ptr<transport_container>;

class tcp_channel final : public channel {
public:
    explicit tcp_channel(looper::tcp tcp);
    ~tcp_channel() override;

    void on_read(read_callback&& callback) override;
    void on_error(error_callback&& callback) override;

    void start_read() override;
    void send(message_ptr&& message) override;

private:
    looper::tcp m_tcp;
    read_callback m_read_callback;
    error_callback m_error_callback;
};

class tcp_transport final : public transport_container {
public:
    tcp_transport();
    ~tcp_transport() override;

    [[nodiscard]] transport type() const override;

    void open(const connection_info& info, open_callback&& callback) override;

private:
    looper::loop m_loop;
};

}
