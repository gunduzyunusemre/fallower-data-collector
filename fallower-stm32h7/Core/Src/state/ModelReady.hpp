#ifndef SRC_STATE_MODEL_READY_HPP_
#define SRC_STATE_MODEL_READY_HPP_

#include "State.hpp"

class ModelReady : public State {
public:
    void HandleEvent(Event* event, void* args) override;
    const char* getStateName() const override { return "ModelReady"; }
    void OnEnter() override;
    void OnExit() override;
};

#endif