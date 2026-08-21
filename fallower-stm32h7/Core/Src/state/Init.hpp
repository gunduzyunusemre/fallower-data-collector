// Init.hpp
#ifndef SRC_STATE_INIT_HPP_
#define SRC_STATE_INIT_HPP_

#include "State.hpp"

class Init : public State {
public:
    void HandleEvent(Event* event, void* args) override;
    const char* getStateName() const override { return "INIT"; }

    void OnEnter() override;
};

#endif /* SRC_STATE_INIT_HPP_ */
