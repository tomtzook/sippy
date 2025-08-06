
#include <sip/session.h>
#include "util/hex.h"

namespace sippy::sip {

static std::string generate_tag() {
    return util::random_hex_string(10);
}

static std::optional<std::string> get_tag(const message_ptr& msg) {
    for (int i = 0; i < msg->header_count<headers::from>(); i++) {
        const auto& header = msg->header<headers::from>(i);
        if (header.tag.has_value()) {
            return header.tag.value();
        }
    }

    return std::nullopt;
}

static std::string generate_branch() {
    return util::random_hex_string(10);
}

static std::optional<std::string> get_branch(const message_ptr& msg, const connection_info& conn_info) {
    for (int i = 0; i < msg->header_count<headers::via>(); i++) {
        const auto& header = msg->header<headers::via>(i);
        if (header.port == conn_info.local_port && header.host == conn_info.local_address) {
            auto it = header.tags.find("branch");
            if (it != header.tags.end()) {
                return it->second;
            }
        }
    }

    return std::nullopt;
}

dialog::dialog(channel_ptr channel, const std::string_view tag, const session_info& info)
    : m_channel(std::move(channel))
    , m_info{.session = info, .tag = std::string(tag)}
    , m_transactions()
    , m_sequence_num(1)
{}

std::string dialog::generate_callid() const {
    return std::format("{}@{}", util::random_hex_string(6), m_info.session.conn_info.local_address);
}

void dialog::request_register(
    const std::string_view target_uri,
    const std::string_view from_uri,
    sip::headers::authorization&& auth_header,
    response_callback&& callback) {
    auto msg = create_request_register(
        target_uri,
        from_uri,
        generate_callid(),
        1,
        1800,
        70
    );
    msg->add_header(std::move(auth_header));

    return request(std::move(msg), std::move(callback));
}

void dialog::request_bye(
    const std::string_view from_uri,
    const std::string_view to_uri,
    const std::string_view call_id,
    response_callback&& callback) {
    auto msg = create_request(
        sip::method::bye,
        to_uri,
        from_uri,
        to_uri,
        call_id,
        1,
        1800,
        70
    );

    return request(std::move(msg), std::move(callback));
}

void dialog::request(message_ptr&& message, response_callback&& callback) {
    if (!message->is_request() || !message->is_valid()) {
        throw std::runtime_error("message is either not valid or not a request");
    }

    auto branch = generate_branch();
    transaction new_transaction({m_info, branch}, std::move(callback));
    auto [it, inserted] = m_transactions.emplace(branch, std::move(new_transaction));
    if (!inserted) {
        throw std::runtime_error("transaction already exists");
    }

    const auto seq_num = next_sequence_number();
    if (message->has_header<headers::cseq>()) {
        auto& cseq = message->header<headers::cseq>();
        cseq.method = message->request_line().method;
        cseq.seq_num = seq_num;
    } else {
        auto cseq = headers::cseq();
        cseq.method = message->request_line().method;
        cseq.seq_num = seq_num;
        message->add_header(cseq);
    }

    send(it->second, std::move(message));
}

void dialog::send(const transaction& transaction, message_ptr&& message) {
    auto& from = message->header<headers::from>();
    from.tag = m_info.tag;

    {
        headers::via via;
        via.version = version::version_2_0;
        via.transport = transaction.info.dialog.session.transport;
        via.host = transaction.info.dialog.session.conn_info.local_address;
        via.port = transaction.info.dialog.session.conn_info.local_port;
        via.tags["branch"] = transaction.info.branch;
        message->add_header(std::move(via));
    }
    {
        headers::contact contact;
        contact.uri = std::format("sip:{}@{}", transaction.info.dialog.session.conn_info.local_address, transaction.info.dialog.session.conn_info.local_port);
        message->add_header(std::move(contact));
    }

    m_channel->send(std::move(message));
}

void dialog::on_new_message(message_ptr&& message, const std::optional<std::string>& branch) {
    if (branch.has_value()) {
        auto it = m_transactions.find(branch.value());
        if (it != m_transactions.end()) {
            const auto done = it->second.callback(*this, std::move(message));
            if (done) {
                m_transactions.erase(it);
            }
        }
    }
}

uint32_t dialog::next_sequence_number() {
    return m_sequence_num++;
}

session::session(transport_container_ptr transport_container, connection_info&& conn_info)
    : m_transport(std::move(transport_container))
    , m_info(m_transport->type(), std::move(conn_info))
    , m_channel()
    , m_listeners()
    , m_dialogs()
{}

void session::listen(sip::method method, listen_callback&& callback) {
    m_listeners.emplace(method, std::move(callback));
}

void session::on_error(error_callback&& callback) {
    m_error_callback = std::move(callback);
}

void session::open(open_callback&& callback) {
    m_transport->open(m_info.conn_info, [this, callback](channel_ptr&& channel, const uint64_t error)->void {
       if (error == 0) {
           m_channel = std::move(channel);
           m_channel->on_read([this](message_ptr&& message)->void {
               on_new_message(std::move(message));
           });
           m_channel->on_error([this](const uint64_t error)->void {
               m_error_callback(error);
           });
           m_channel->start_read();
       }

        callback(*this, error);
   });
}

dialog_ptr session::create_dialog() {
    auto tag = generate_tag();
    auto new_dialog = std::make_shared<dialog>(m_channel, tag, m_info);
    m_dialogs.emplace(tag, new_dialog);

    return new_dialog;
}

void session::on_new_message(message_ptr&& message) {
    const auto tag_opt = get_tag(message);
    const auto branch_opt = get_branch(message, m_info.conn_info);

    if (tag_opt.has_value()) {
        auto it = m_dialogs.find(tag_opt.value());
        if (it != m_dialogs.end()) {
            // we have a dialog for this
            it->second->on_new_message(std::move(message), branch_opt);
            return;
        } else {
            // has our via but unknown dialog
        }
    }

    if (message->is_request()) {
        // new request for us
        auto it = m_listeners.find(message->request_line().method);
        if (it != m_listeners.end()) {
            const auto new_dialog = create_dialog();
            it->second(new_dialog, std::move(message));
            return;
        } else {
            // we have no listeners for this message
        }
    } else {
        // response to us without a dialog
    }
}

}
