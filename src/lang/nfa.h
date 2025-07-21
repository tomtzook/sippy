#pragma once

#include <vector>

namespace sippy::lang {

template<typename T>
class nfa {
public:
    using state = size_t;
    using matcher_func = bool(*)(const T&);

    nfa();

    [[nodiscard]] state initial_state() const;
    [[nodiscard]] state first_end_state() const;
    [[nodiscard]] size_t states() const;

    state add_state(const char* name, bool is_end = false);
    void add_transition(state src, state dst, const char* name, matcher_func matcher, bool consumes = true);

    [[nodiscard]] bool compute(const std::vector<T>& values) const;

private:
    struct transition {
        const char* name;
        state dest_state;
        bool consumes;
        matcher_func matcher;
    };
    struct state_node {
        state id;
        const char* name;
        bool is_end;
        std::vector<transition> transitions;
    };
    struct stack_node {
        size_t str_index;
        state state_id;
    };

    void append(const nfa& other, state join_state);
    void redirect_ends_to(state end_state);

    void copy_state(const state_node& other, size_t id_offset = 0);
    std::pair<state, state_node&> add_state_internal(const char* name, bool is_end);
    void add_transition_internal(state_node& node, state dest, const char* name, matcher_func matcher, bool consumes = true);
    void copy_transitions(state_node& first, const state_node& other, size_t id_offset = 0);

    std::vector<state_node> m_states;

    template<typename T2>
    friend nfa<T2> sequence(const nfa<T2>& first, const nfa<T2>& second, typename nfa<T2>::state join_state);
    template<typename T2>
    friend nfa<T2> parallel(const nfa<T2>& first, const nfa<T2>& second);
};

template<typename T>
nfa<T>::nfa()
    : m_states()
{}

template<typename T>
typename nfa<T>::state nfa<T>::initial_state() const {
    return m_states[0].id;
}

template<typename T>
typename nfa<T>::state nfa<T>::first_end_state() const {
    for (const auto& state : m_states) {
        if (state.is_end) {
            return state.id;
        }
    }

    throw std::runtime_error("not found");
}

template<typename T>
size_t nfa<T>::states() const {
    return m_states.size();
}

template<typename T>
typename nfa<T>::state nfa<T>::add_state(const char* name, const bool is_end) {
    auto [id, state] = add_state_internal(name, is_end);
    return id;
}

template<typename T>
void nfa<T>::add_transition(const state src, const state dst, const char* name, const matcher_func matcher, const bool consumes) {
    auto& src_state = m_states[src];
    auto& dst_state = m_states[dst]; // verify such state exists

    add_transition_internal(src_state, dst, name, matcher, consumes);
}

template<typename T>
bool nfa<T>::compute(const std::vector<T>& values) const {
    std::stack<stack_node> stack;
    // state 0 is always the initial
    stack.push({0, 0});

    while (!stack.empty()) {
        const auto c_item = stack.top();
        stack.pop();

        const auto& state = m_states[c_item.state_id];
        if (state.is_end) {
            // done
            return true;
        }

        const auto& token = values[c_item.str_index];
        for (auto it = state.transitions.rbegin(); it != state.transitions.rend(); ++it) {
            if (it->matcher == nullptr || it->matcher(token)) {
                const auto next_str_index = it->consumes ? c_item.str_index + 1 : c_item.str_index;
                stack.push({next_str_index, it->dest_state});
            }
        }
    }

    return false;
}

template<typename T>
void nfa<T>::append(const nfa& other, const state join_state) {
    // copy states from second, excluding start, adjust ids
    const auto id_offset = other.m_states.size() - 1;
    for (auto it = other.m_states.begin() + 1; it != other.m_states.end(); ++it) {
        copy_state(*it, id_offset);
    }

    // join state is the new start for second
    auto& join_state_node = m_states[join_state];
    copy_transitions(join_state_node, other.m_states[0], id_offset + 1);
    join_state_node.is_end = false;
}

template<typename T>
void nfa<T>::redirect_ends_to(const state end_state) {
    for (auto& state : m_states) {
        if (state.id != end_state && state.is_end) {
            add_transition_internal(state, end_state, "redirected_end", nullptr, false);
            state.is_end = false;
        }
    }
}

template<typename T>
void nfa<T>::copy_state(const state_node& other, const size_t id_offset) {
    auto [id, state] = add_state_internal(other.name, other.is_end);
    copy_transitions(state, other, id_offset);
}

template<typename T>
std::pair<typename nfa<T>::state, typename nfa<T>::state_node&> nfa<T>::add_state_internal(const char* name, bool is_end) {
    auto& state = m_states.emplace_back(0, name, is_end);
    const auto id = m_states.end() - 1 - m_states.begin();
    state.id = id;
    return {id, state};
}

template<typename T>
void nfa<T>::add_transition_internal(state_node& node, const state dest, const char* name, const matcher_func matcher, const bool consumes) {
    transition transition{};
    transition.name = name;
    transition.dest_state = dest;
    transition.matcher = matcher;
    transition.consumes = consumes;
    node.transitions.push_back(std::move(transition));
}

template<typename T>
void nfa<T>::copy_transitions(state_node& first, const state_node& other, const size_t id_offset) {
    for (const auto& transition : other.transitions) {
        auto transition_copy = transition;
        transition_copy.dest_state += id_offset;
        first.transitions.push_back(std::move(transition_copy));
    }
}

template<typename T>
nfa<T> one_step(const typename nfa<T>::matcher_func matcher) {
    nfa<T> nfa;
    const auto state_0 = nfa.add_state("q0");
    const auto state_1 = nfa.add_state("q1", true);
    nfa.add_transition(state_0, state_1, "matcher", matcher);

    return std::move(nfa);
}

template<typename T>
void optional(nfa<T>& nfa) {
    nfa.add_transition(nfa.initial_state(), nfa.first_end_state(), "optional", nullptr, false);
}

template<typename T>
nfa<T> sequence(const nfa<T>& first, const nfa<T>& second, const typename nfa<T>::state join_state) {
    nfa<T> new_nfa;

    // copy states from first, including start and end
    for (const auto& state : first.m_states) {
        new_nfa.copy_state(state);
    }

    new_nfa.append(second, join_state);

    return std::move(new_nfa);
}

template<typename T>
nfa<T> parallel(const nfa<T>& first, const nfa<T>& second) {
    nfa<T> new_nfa;
    const auto start = new_nfa.add_state("start");

    new_nfa.append(first, start);
    new_nfa.append(second, start);

    const auto end = new_nfa.add_state("end", true);
    new_nfa.redirect_ends_to(end);

    return std::move(new_nfa);
}

}
