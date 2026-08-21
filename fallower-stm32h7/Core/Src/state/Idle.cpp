#include "Idle.hpp"
#include "Event.hpp"
#include "StateMachine.hpp"
#include "StateFactory.hpp"
#include "../UsbCommunication.hpp"


void Idle::HandleEvent(Event* event, void* args) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (!event) return;

    // --- KALDIRILACAK KISIM BAŞLANGICI ---
    /*
    if (event->GetType() == Initialize) {
        if (usb) usb->sendMessage("[IDLE] Initialize event detected\r\n");
        if (this->context_) {
            // Use STATE_INIT instead of StateFactory::createState
            this->context_->changeState(STATE_INIT); // KALDIRILACAK
            if (usb) usb->sendMessage("[IDLE] Changed state to INIT\r\n");
        } else {
            if (usb) usb->sendMessage("[IDLE] ERROR: Context is null!\r\n");
        }
    } else {
        if (usb) usb->sendMessage("[IDLE] Command ignored. Please send 'INIT' first.\r\n");
    }
    */
    // --- KALDIRILACAK KISIM SONU ---

    // --- YENİ EKLENECEK VEYA DEĞİŞTİRİLECEK KISIM BAŞLANGICI ---
    // Idle durumundayken INIT dışındaki komutlar genellikle hata mesajı üretmelidir.
    // Bu kontrol StateMachine::processCommand içinde zaten yapılıyor olabilir.
    // Eğer Idle durumuna özgü başka olaylar (event) işlenecekse, buraya eklenebilir.
    // Örneğin, bir timeout olayı veya başka bir içsel tetikleyici.

    // Şimdilik, Idle durumundayken gelen ve INIT olmayan komutlara
    // genel bir mesaj gönderilebilir (bu StateMachine::processCommand içinde de yapılabilir).
    if (event->GetType() != Initialize) { // Veya daha spesifik kontrol
         if (usb) usb->sendStatusMessage(getStateName(), "Command ignored. System is in IDLE state. Send 'INIT' first.");
    }
    // --- YENİ EKLENECEK VEYA DEĞİŞTİRİLECEK KISIM SONU ---
}

// Idle.hpp içindeki OnEnter kısmı genellikle kalabilir.
/*
void Idle::OnEnter() override {
    sendStateMessage("System Ready. Waiting for INIT command.");
}
*/
