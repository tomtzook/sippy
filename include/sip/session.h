#pragma once

#include <functional>
#include <unordered_map>

#include <sip/account.h>
#include <sip/transport.h>

namespace sippy::sip {

class session;
class dialog;
using dialog_ptr = std::shared_ptr<dialog>;

struct session_info {
    sip::transport transport;
    connection_info conn_info;
};

struct dialog_info {
    const session_info& session;
    std::string local_tag;
    std::optional<std::string> remote_tag;
};

struct transaction_info {
    const dialog_info& dialog;
    std::string branch;
};

using response_callback = std::function<bool(dialog&, message_ptr&&)>;

struct transaction {
    transaction_info info;
    response_callback callback;
};

class dialog {
public:
    dialog(channel_ptr channel, std::string_view tag, const session_info& info);

    std::string generate_callid() const;

    void request_register(
        std::string_view target_uri,
        std::string_view from_uri,
        sip::headers::authorization&& auth_header,
        response_callback&& callback,
        header_container&& additional_headers);
    void request_register(
        std::string_view target_uri,
        std::string_view from_uri,
        sip::headers::authorization&& auth_header,
        response_callback&& callback);
    void request_invite(
        std::string_view from_uri,
        std::string_view to_uri,
        std::string_view call_id,
        response_callback&& callback,
        header_container&& additional_headers);
    void request_invite(
        std::string_view from_uri,
        std::string_view to_uri,
        std::string_view call_id,
        response_callback&& callback);
    void request_ack(
        std::string_view from_uri,
        std::string_view to_uri,
        std::string_view call_id,
        response_callback&& callback,
        header_container&& additional_headers);
    void request_ack(
        std::string_view from_uri,
        std::string_view to_uri,
        std::string_view call_id,
        response_callback&& callback);
    void request_bye(
        std::string_view from_uri,
        std::string_view to_uri,
        std::string_view call_id,
        response_callback&& callback,
        header_container&& additional_headers);
    void request_bye(
        std::string_view from_uri,
        std::string_view to_uri,
        std::string_view call_id,
        response_callback&& callback);

    void request(message_ptr&& message, response_callback&& callback);
    void respond(status_code code, message_ptr&& original_request);

private:
    message_ptr _create_request_register(
        std::string_view target_uri,
        std::string_view from_uri,
        sip::headers::authorization&& auth_header);
    message_ptr _create_request_invite(
        std::string_view from_uri,
        std::string_view to_uri,
        std::string_view call_id);
    message_ptr _create_request_ack(
        std::string_view from_uri,
        std::string_view to_uri,
        std::string_view call_id);
    message_ptr _create_request_bye(
        std::string_view from_uri,
        std::string_view to_uri,
        std::string_view call_id);

    void send(const transaction& transaction, message_ptr&& message);
    void on_new_message(message_ptr&& message, const std::optional<std::string>& branch);
    void try_assign_remote_tag(const message_ptr& message);

    const transaction& create_transaction(response_callback&& callback = nullptr);
    uint32_t next_sequence_number();

    channel_ptr m_channel;
    dialog_info m_info;

    std::unordered_map<std::string, transaction> m_transactions;
    uint32_t m_sequence_num;

    friend class session;
};

class session {
public:
    using listen_callback = std::function<void(const dialog_ptr&, message_ptr&&)>;
    using open_callback = std::function<void(session&, uint64_t)>;
    using error_callback = std::function<void(uint64_t)>;

    session(transport_container_ptr transport_container, connection_info&& conn_info);

    void listen(sip::method method, listen_callback&& callback);
    void on_error(error_callback&& callback);

    void open(open_callback&& callback);

    dialog_ptr create_dialog();

private:
    void on_new_message(message_ptr&& message);

    transport_container_ptr m_transport;
    session_info m_info;
    channel_ptr m_channel;
    error_callback m_error_callback;

    std::unordered_map<sip::method, listen_callback> m_listeners;
    std::unordered_map<std::string, dialog_ptr> m_dialogs;
};

}
