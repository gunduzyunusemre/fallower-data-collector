#ifndef SRC_STATE_STATE_HPP_
#define SRC_STATE_STATE_HPP_

#include "../UsbCommunication.hpp"

// Forward declarations
class StateMachine;  // Forward declaration
class Event;

class State {
protected:
    StateMachine* context_;  // Pointer to the state machine context

    // USB iletişimi için yardımcı metodlar
    bool sendMessage(const char* message) {
        UsbCommunication* usb = UsbCommunication::getInstance();
        return usb ? usb->sendMessage(message) : false;
    }

    bool sendStateMessage(const char* message) {
        UsbCommunication* usb = UsbCommunication::getInstance();
        return usb ? usb->sendStatusMessage(getStateName(), message) : false;
    }
    
    // Overload for custom state name
    bool sendStateMessage(const char* stateName, const char* message) {
        UsbCommunication* usb = UsbCommunication::getInstance();
        return usb ? usb->sendStatusMessage(stateName, message) : false;
    }

    // Her state'in kendi adını döndürmesi için
    virtual const char* getStateName() const = 0;

public:
    virtual ~State() {}

    void set_context(StateMachine* context) {
        this->context_ = context;
    }

    virtual void HandleEvent(Event* event, void* args) = 0;

    // State'e girildiğinde çağrılacak
    virtual void OnEnter() {
        // State giriş mesajı gönder
        sendStateMessage("State active");
    }

    virtual void OnExit() {
    }

    const char* getName() const { return getStateName(); }
};

#endif /* SRC_STATE_STATE_HPP_ */
