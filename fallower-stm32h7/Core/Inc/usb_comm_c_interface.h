// Dosya: Core/Inc/usb_comm_c_interface.h

#ifndef USB_COMM_C_INTERFACE_H
#define USB_COMM_C_INTERFACE_H

#include <stdint.h>

// Bu sihirli blok, dosyanın hem C hem de C++ tarafından
// sorunsuzca kullanılabilmesini sağlar.
#ifdef __cplusplus
extern "C" {
#endif

// C kodunun çağıracağı fonksiyonun prototipi budur.
// C++ derleyicisi, extern "C" sayesinde bu fonksiyonun ismini bozmayacak.
void UsbRxQueueAddData(uint8_t* data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif // USB_COMM_C_INTERFACE_H