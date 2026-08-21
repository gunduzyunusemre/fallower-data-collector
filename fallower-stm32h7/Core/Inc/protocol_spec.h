#ifndef PROTOCOL_SPEC_H
#define PROTOCOL_SPEC_H

#include <stdint.h>

// Protokol Başlangıç ve Bitiş İşaretleri
#define SOP 0xAA  // Start of Packet
#define EOP 0x55  // End of Packet

// Paket Tipleri - Python ile uyumlu
typedef enum {
    PACKET_TYPE_START_OF_TRANSFER = 0x01,
    PACKET_TYPE_DATA = 0x02,        // Model weight verisi
    PACKET_TYPE_END_OF_TRANSFER = 0x03,         // End of Transfer
    PACKET_TYPE_ACK = 0x04,         // Onay paketi
    PACKET_TYPE_NACK = 0x05,        // Hata paketi
    PACKET_TYPE_PROGRESS = 0x06,    // Progress
    PACKET_TYPE_METADATA = 0x07,    // Model metadata bilgisi - Python compatible!
    PACKET_TYPE_READ_MODEL = 0x08,
    PACKET_TYPE_TASK_COMPLETE = 0x09,
    PACKET_TYPE_TASK_FAILED = 0x0A
} PacketType_t;

// Paket Header Yapısı
typedef struct __attribute__((packed)) {
    uint8_t sop;           // Start of Packet (0xAA)
    uint8_t type;          // Paket tipi (PacketType_t olarak cast edilecek)
    uint16_t length;       // Payload uzunluğu
} PacketHeader_t;

// Metadata yapısı (Python ile uyumlu format)
// Python: name(8) + version(4) + size(4) + crc(4) = 20 bytes
typedef struct __attribute__((packed)) {
    uint8_t  model_name[8];     // Model ismi (8 bytes)
    uint16_t version_major;     // Major version (2 bytes)
    uint16_t version_minor;     // Minor version (2 bytes) 
    uint32_t total_size;        // Model boyutu (4 bytes)
    uint32_t crc32;            // CRC32 checksum (4 bytes)
} Metadata_t;  // Total: 20 bytes

#endif // PROTOCOL_SPEC_H