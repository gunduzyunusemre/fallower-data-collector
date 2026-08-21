#ifndef SRC_STATE_LOADING_DATA_HPP_
#define SRC_STATE_LOADING_DATA_HPP_

#include "State.hpp"

class LoadingData : public State {
public:
    void HandleEvent(Event* event, void* args) override;
    const char* getStateName() const override { return "LoadingData"; }
    void OnEnter() override;
    void OnExit() override;
};

#endif