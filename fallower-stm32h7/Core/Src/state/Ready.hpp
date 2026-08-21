#ifndef READY_HPP
#define READY_HPP

#include "State.hpp"
#include "../UsbCommunication.hpp"
#include <vector>

class Ready : public State {
public:
    // State'e giriş yapılırken yapılacak işlemler
    virtual void OnEnter() override;
    // Ready.hpp içinde class Ready tanımına ekleyin
    virtual void OnExit() override;

    // Event'leri işleme
    virtual void HandleEvent(Event* event, void* args) override;

    // Register güncelleme işlemi

    // Yardımcı fonksiyon
    const char* getStateName() const { return "READY"; }
};

#endif // READY_HPP
