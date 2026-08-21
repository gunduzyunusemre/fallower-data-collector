#ifndef SRC_STATE_IDLE_HPP_
#define SRC_STATE_IDLE_HPP_

#include "State.hpp"

class Idle : public State {
public:
    void HandleEvent(Event* event, void* args) override;
    const char* getStateName() const override { return "IDLE"; }

    void OnEnter() override {
        // Bu state'e girildiğinde bir mesaj gönderilebilir (opsiyonel)
        // UsbStateInitTask zaten benzer bir mesaj gönderiyor olabilir [source:106]
        sendStateMessage("System Ready. Waiting for INIT command.");
    }
};

#endif /* SRC_STATE_IDLE_HPP_ */
