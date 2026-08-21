#include "LoadingData.hpp"
#include "Event.hpp"

void LoadingData::HandleEvent(Event* event, void* args) {
    // Bu state'de event'ler paket olarak işleneceği için bu metod kullanılmayacak.
}

void LoadingData::OnEnter() {
    sendStateMessage("Loading AI model data - ready for data transfer.");
    
    // Send ACK to PC to indicate ready for data transfer
    sendStateMessage("ACK", "Ready for data transfer");
}

void LoadingData::OnExit() {
    // Bu state'den çıkarken yapılacak bir şey varsa buraya yazılır.
}