#pragma once

#include <vector>
#include <string>

#include "lexer.h"

namespace sippy::lang {

class nfa {
public:
    using state = size_t;
    using matcher_func = bool(*)(const token&);

    nfa();

    [[nodiscard]] size_t states() const;

    state add_state(const char* name, bool is_end = false);
    void add_transition(state src, state dst, const char* name, matcher_func matcher, bool consumes = true);

    [[nodiscard]] bool compute(const std::vector<token>& tokens) const;

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

    void append(const nfa& other, state join_state);
    void redirect_ends_to(state end_state);

    void copy_state(const state_node& other, size_t id_offset = 0);
    std::pair<state, state_node&> add_state_internal(const char* name, bool is_end);
    void add_transition_internal(state_node& node, state dest, const char* name, matcher_func matcher, bool consumes = true);
    void copy_transitions(state_node& first, const state_node& other, size_t id_offset = 0);

    std::vector<state_node> m_states;

    friend nfa sequence(const nfa& first, const nfa& second, nfa::state join_state);
    friend nfa parallel(const nfa& first, const nfa& second);
};

nfa sequence(const nfa& first, const nfa& second, nfa::state join_state);
nfa parallel(const nfa& first, const nfa& second);

}
