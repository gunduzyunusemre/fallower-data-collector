#ifndef SRC_STATE_VERIFYING_DATA_HPP_
#define SRC_STATE_VERIFYING_DATA_HPP_

#include "State.hpp"

class VerifyingData : public State {
public:
    void HandleEvent(Event* event, void* args) override;
    const char* getStateName() const override { return "VerifyingData"; }
    void OnEnter() override;
    void OnExit() override;
};

#endif