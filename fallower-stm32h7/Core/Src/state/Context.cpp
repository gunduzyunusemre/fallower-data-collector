//// Context.cpp
//#include "Context.hpp"
//#include "State.hpp"
//#include "Event.hpp"
//
//Context::Context(State* state) : state_(nullptr) {
//    this->TransitionTo(state);
//}
//
//Context::~Context() {
//    delete state_;
//}
//
//void Context::TransitionTo(State* state) {
//    UsbCommunication* usb = UsbCommunication::getInstance();
//
//    if (usb && usb->isReady()) {
//        usb->sendStateMessage("CONTEXT", "TransitionTo called");
//    }
//
//    if (state == nullptr) {
//        if (usb && usb->isReady()) {
//            usb->sendStateMessage("CONTEXT", "ERROR: New state is null!");
//        }
//        return;
//    }
//
//    if (this->state_ != nullptr) {
//        if (usb && usb->isReady()) {
//            char stateMsg[50];
//            snprintf(stateMsg, sizeof(stateMsg), "Deleting old state: %s", this->state_->getName());
//            usb->sendStateMessage("CONTEXT", stateMsg);
//        }
//        delete this->state_;
//    } else {
//        if (usb && usb->isReady()) {
//            usb->sendStateMessage("CONTEXT", "No previous state to delete");
//        }
//    }
//
//    this->state_ = state;
//    this->state_->set_context(this);
//
//    if (usb && usb->isReady()) {
//        char stateMsg[50];
//        snprintf(stateMsg, sizeof(stateMsg), "New state set: %s", this->state_->getName());
//        usb->sendStateMessage("CONTEXT", stateMsg);
//    }
//
//    // Yeni state'e giriş
//    this->state_->OnEnter();
//
//    if (usb && usb->isReady()) {
//        usb->sendStateMessage("CONTEXT", "OnEnter called for new state");
//    }
//}
//
//void Context::Request(Event* event, void* args) {
//    this->state_->HandleEvent(event, args);
//}
