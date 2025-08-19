
#include "reader.h"


namespace sippy::sdp {

reader::reader(std::istream &is)
    : m_is(is)
    , m_reader(is)
{}

session_description reader::read() {
    session_description description{};

    read(description.version);
    read(description.origin);
    read_optional(description.information);
    read_optional(description.uri);
    read_vector(description.emails);
    read_vector(description.phone_numbers);
    read_optional(description.connection_information);
    read_vector(description.bandwidths);

    do {
        time_description time_description{};
        read_description(time_description);
    } while (peek<fields::time_active>());

    read_vector(description.attributes);

    while (peek<fields::media_description>()) {
        media_description media_description{};
        read_description(media_description);
    };

    validate_message(description);

    return description;
}

void reader::read_description(time_description& description) {
    read(description.times);
    read_vector(description.repeat_times);
    read_optional(description.timezone);
}

void reader::read_description(media_description& description) {
    read(description.media);
    read_optional(description.information);
    read_vector(description.connections);
    read_vector(description.bandwidths);
    read_vector(description.attributes);
}

}
