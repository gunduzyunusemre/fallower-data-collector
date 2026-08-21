#include "AwaitingMetadata.hpp"
#include "Event.hpp"

void AwaitingMetadata::HandleEvent(Event* event, void* args) {
    // Bu state'de event'ler paket olarak işleneceği için bu metod kullanılmayacak.
}

void AwaitingMetadata::OnEnter() {
    sendStateMessage("Waiting for AI model metadata.");
}

void AwaitingMetadata::OnExit() {
    // Bu state'den çıkarken yapılacak bir şey varsa buraya yazılır.
}