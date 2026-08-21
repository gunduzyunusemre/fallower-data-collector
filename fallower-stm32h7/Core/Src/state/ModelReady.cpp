#include "ModelReady.hpp"
#include "Event.hpp"

void ModelReady::HandleEvent(Event* event, void* args) {
    // Bu state'de event'ler paket olarak işleneceği için bu metod kullanılmayacak.
}

void ModelReady::OnEnter() {
    sendStateMessage("AI model is ready for inference.");
    
    // Notify PC that model is successfully set up and ready for START command
    sendStateMessage("MODEL_READY", "AI model loaded and ready for START command");
}

void ModelReady::OnExit() {
    // Bu state'den çıkarken yapılacak bir şey varsa buraya yazılır.
}