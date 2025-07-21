
#include <ranges>
#include <stack>

#include "nfa.h"

namespace sippy::lang {

struct stack_node {
    size_t str_index;
    nfa::state state_id;
};

nfa::nfa()
    : m_states()
{}

size_t nfa::states() const {
    return m_states.size();
}

nfa::state nfa::add_state(const char* name, const bool is_end) {
    auto [id, state] = add_state_internal(name, is_end);
    return id;
}

void nfa::add_transition(const state src, const state dst, const char* name, const matcher_func matcher, const bool consumes) {
    auto& src_state = m_states[src];
    auto& dst_state = m_states[dst]; // verify such state exists

    add_transition_internal(src_state, dst, name, matcher, consumes);
}

bool nfa::compute(const std::vector<token>& tokens) const {
    std::stack<stack_node> stack;
    stack.push({0, 0});

    while (!stack.empty()) {
        const auto c_item = stack.top();
        stack.pop();

        const auto& state = m_states[c_item.state_id];
        if (state.is_end) {
            // done
            return true;
        }

        const auto token = tokens[c_item.str_index];
        for (const auto & transition : std::ranges::reverse_view(state.transitions)) {
            if (transition.matcher == nullptr || transition.matcher(token)) {
                const auto next_str_index = transition.consumes ? c_item.str_index + 1 : c_item.str_index;
                stack.push({next_str_index, transition.dest_state});
            }
        }
    }

    return false;
}

void nfa::append(const nfa& other, const state join_state) {
    // copy states from second, excluding start, adjust ids
    const auto id_offset = other.m_states.size() - 1;
    for (auto it = other.m_states.begin() + 1; it != other.m_states.end(); ++it) {
        copy_state(*it, id_offset);
    }

    // join state is the new start for second
    auto& join_state_node = m_states[join_state];
    copy_transitions(join_state_node, other.m_states[0], id_offset);
    join_state_node.is_end = false;
}

void nfa::redirect_ends_to(const state end_state) {
    for (auto& state : m_states) {
        if (state.id != end_state && state.is_end) {
            add_transition_internal(state, end_state, "redirected_end", nullptr, false);
            state.is_end = false;
        }
    }
}

void nfa::copy_state(const state_node& other, const size_t id_offset) {
    auto [id, state] = add_state_internal(other.name, other.is_end);
    copy_transitions(state, other, id_offset);
}

std::pair<nfa::state, nfa::state_node&> nfa::add_state_internal(const char* name, bool is_end) {
    auto& state = m_states.emplace_back(0, name, is_end);
    const auto id = m_states.end() - 1 - m_states.begin();
    state.id = id;
    return {id, state};
}

void nfa::add_transition_internal(state_node& node, const state dest, const char* name, const matcher_func matcher, const bool consumes) {
    transition transition{};
    transition.name = name;
    transition.dest_state = dest;
    transition.matcher = matcher;
    transition.consumes = consumes;
    node.transitions.push_back(std::move(transition));
}

void nfa::copy_transitions(state_node& first, const state_node& other, const size_t id_offset) {
    for (const auto& transition : other.transitions) {
        auto transition_copy = transition;
        transition_copy.dest_state += id_offset;
        first.transitions.push_back(std::move(transition_copy));
    }
}

nfa sequence(const nfa& first, const nfa& second, const nfa::state join_state) {
    nfa new_nfa;

    // copy states from first, including start and end
    for (const auto& state : first.m_states) {
        new_nfa.copy_state(state);
    }

    new_nfa.append(second, join_state);

    return new_nfa;
}

nfa parallel(const nfa& first, const nfa& second) {
    nfa new_nfa;
    const auto start = new_nfa.add_state("start");

    new_nfa.append(first, start);
    new_nfa.append(second, start);

    const auto end = new_nfa.add_state("end", true);
    new_nfa.redirect_ends_to(end);

    return new_nfa;
}

}
