#include "Ready.hpp"
#include "Event.hpp"
#include "Context.hpp"
#include "StateFactory.hpp"
#include "../UsbCommunication.hpp"
#include <stdio.h>
#include "cmsis_os.h"  // FreeRTOS için gerekli başlık


void Ready::HandleEvent(Event* event, void* args) {
    if (!event) return;

    switch (event->GetType()) {
        case Initialize:
            sendStateMessage("System already initialized");
            break;

        default:
            sendStateMessage("Command received in Ready state");
            break;
    }
}



void Ready::OnEnter() {
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (usb) {
        usb->sendStatusMessage(getStateName(), "Ready state active. Waiting for command...");
    }
}

void Ready::OnExit() {
    // USB kuyruğunu temizle
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (usb) {
        usb->clearQueue();  // Bu fonksiyonu UsbCommunication'a eklemeniz gerekecek
    }
}
