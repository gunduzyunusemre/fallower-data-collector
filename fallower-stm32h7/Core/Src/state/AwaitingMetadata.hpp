#ifndef SRC_STATE_AWAITING_METADATA_HPP_
#define SRC_STATE_AWAITING_METADATA_HPP_

#include "State.hpp"

class AwaitingMetadata : public State {
public:
    void HandleEvent(Event* event, void* args) override;
    const char* getStateName() const override { return "AwaitingMetadata"; }
    void OnEnter() override;
    void OnExit() override;
};

#endif