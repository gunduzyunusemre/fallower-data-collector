#include "VerifyingData.hpp"
#include "Event.hpp"
#include "cmsis_os.h"

// Forward declaration
void ModelInstallerTask(void* parameters);

void VerifyingData::HandleEvent(Event* event, void* args) {
    // Bu state'de event'ler paket olarak işleneceği için bu metod kullanılmayacak.
}

void VerifyingData::OnEnter() {
    sendStateMessage("Verifying AI model data integrity.");
    
    // Start ModelInstallerTask for verification and model setup
    BaseType_t result = xTaskCreate(
        ModelInstallerTask,
        "ModelInstaller",
        1024,  // Stack size
        NULL,  // Parameters
        tskIDLE_PRIORITY + 1,  // Priority
        NULL   // Task handle
    );
    
    if (result != pdPASS) {
        sendStateMessage("ERROR: Failed to create ModelInstaller task");
    }
}

void VerifyingData::OnExit() {
    // Bu state'den çıkarken yapılacak bir şey varsa buraya yazılır.
}