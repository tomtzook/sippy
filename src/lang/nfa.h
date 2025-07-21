#pragma once

#include <vector>

namespace sippy::lang {

template<typename T>
class nfa {
public:
    using state = size_t;
    using group = size_t;
    using matcher_func = bool(*)(const T&);

    struct group_match {
        group id;
        size_t start;
        ssize_t end;
    };
    struct match {
        bool matched;
        std::vector<group_match> groups;
    };

    nfa();

    [[nodiscard]] state initial_state() const;
    [[nodiscard]] state first_end_state() const;
    [[nodiscard]] size_t states() const;

    state add_state(const char* name, bool is_end = false);
    void add_transition(state src, state dst, const char* name, matcher_func matcher, bool consumes = true);

    group new_group(state start, state end);

    [[nodiscard]] match compute(const std::vector<T>& values) const;

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
        std::vector<group> group_starts;
        std::vector<group> group_ends;
    };
    struct stack_node {
        size_t pos;
        state state_id;
        std::vector<group_match> groups;
    };

    void process_groups(stack_node& node) const;

    void append(const nfa& other, state join_state);
    void redirect_ends_to(state end_state);

    void copy_state(const state_node& other, size_t id_offset = 0, size_t group_offset = 0);
    std::pair<state, state_node&> add_state_internal(const char* name, bool is_end);
    void add_transition_internal(state_node& node, state dest, const char* name, matcher_func matcher, bool consumes = true);
    void copy_transitions(state_node& first, const state_node& other, size_t id_offset = 0);
    void copy_groups(state_node& first, const state_node& other, size_t id_offset = 0);

    std::vector<state_node> m_states;
    group m_next_group_id;

    template<typename T2>
    friend nfa<T2> sequence(const nfa<T2>& first, const nfa<T2>& second, typename nfa<T2>::state join_state);
    template<typename T2>
    friend nfa<T2> parallel(const nfa<T2>& first, const nfa<T2>& second);
};

template<typename T>
nfa<T>::nfa()
    : m_states()
    , m_next_group_id(0)
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
typename nfa<T>::group nfa<T>::new_group(state start, state end) {
    auto group = ++m_next_group_id;

    auto& start_state = m_states[start];
    auto& end_state = m_states[end];

    start_state.group_starts.push_back(group);
    end_state.group_ends.push_back(group);

    return group;
}

template<typename T>
typename nfa<T>::match nfa<T>::compute(const std::vector<T>& values) const {
    std::stack<stack_node> stack;
    // state 0 is always the initial
    stack.push({0, 0});

    while (!stack.empty()) {
        auto c_item = std::move(stack.top());
        stack.pop();

        process_groups(c_item);

        const auto& state = m_states[c_item.state_id];
        if (state.is_end) {
            // done
            match match{};
            match.matched = true;
            match.groups = std::move(c_item.groups);

            return std::move(match);
        }

        const auto& token = values[c_item.pos];
        for (auto it = state.transitions.rbegin(); it != state.transitions.rend(); ++it) {
            if (it->matcher == nullptr || it->matcher(token)) {
                const auto next_pos = it->consumes ? c_item.pos + 1 : c_item.pos;
                stack.push({next_pos, it->dest_state, c_item.groups});
            }
        }
    }

    return {false};
}

template<typename T>
void nfa<T>::process_groups(stack_node& node) const {
    const auto& state = m_states[node.state_id];
    for (const auto& group : state.group_starts) {
        group_match match{group, node.pos, -1};
        node.groups.push_back(match);
    }

    for (const auto& group : state.group_ends) {
        for (auto& match : node.groups) {
            if (match.id == group) {
                match.end = node.pos;
                break;
            }
        }
    }
}

template<typename T>
void nfa<T>::append(const nfa& other, const state join_state) {
    // copy states from second, excluding start, adjust ids
    const auto id_offset = m_states.size() - 1;
    const auto group_offset = m_next_group_id;
    for (auto it = other.m_states.begin() + 1; it != other.m_states.end(); ++it) {
        copy_state(*it, id_offset, group_offset);
    }

    // join state is the new start for second
    auto& join_state_node = m_states[join_state];
    copy_transitions(join_state_node, other.m_states[0], id_offset);
    copy_groups(join_state_node, other.m_states[0], group_offset);
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
void nfa<T>::copy_state(const state_node& other, const size_t id_offset, const size_t group_offset) {
    auto [id, state] = add_state_internal(other.name, other.is_end);

    copy_transitions(state, other, id_offset);
    copy_groups(state, other, group_offset);
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
void nfa<T>::copy_groups(state_node& first, const state_node& other, size_t id_offset) {
    for (const auto& group : other.group_starts) {
        first.group_starts.push_back(group + id_offset);
    }
    for (const auto& group : other.group_ends) {
        first.group_ends.push_back(group + id_offset);
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

    new_nfa.m_next_group_id = first.m_next_group_id;
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
