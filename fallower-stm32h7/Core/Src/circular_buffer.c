#include "circular_buffer.h"
#include <string.h>
#include "main.h" // HAL kütüphanesinden __disable_irq gibi makrolar için

// Kritik bölge makroları
#define CB_ENTER_CRITICAL() uint32_t primask_status = __get_PRIMASK(); __disable_irq()
#define CB_EXIT_CRITICAL()  __set_PRIMASK(primask_status)

/**
 * @brief Dairesel tamponu (Circular Buffer) başlatır.
 * 
 * Verilen bellek alanını kullanarak tampon yapısını ilkinize eder. Baş ve kuyruk 
 * indislerini sıfırlar.
 * 
 * @param cb Başlatılacak tampon yapısı pointer'ı.
 * @param buffer_storage Verilerin saklanacağı ham bellek alanı. [byte array]
 * @param size Tamponun toplam boyutu. [byte]
 * @return None
 * 
 * @note Bu fonksiyon kritik bölge (critical section) korumalıdır. ISR içinden çağrılabilir 
 *       ancak genellikle sistem başlatma aşamasında kullanılır.
 */
void CB_Init(CircularBuffer_t* cb, uint8_t* buffer_storage, size_t size) {
    if (!cb || !buffer_storage || size == 0) return;
    CB_ENTER_CRITICAL();
    cb->buffer = buffer_storage;
    cb->size = size;
    cb->head = 0;
    cb->tail = 0;
    cb->is_full = false;
    CB_EXIT_CRITICAL();
}

/**
 * @brief Tampon içeriğini sıfırlar.
 * 
 * Belleği temizlemez, sadece baş ve kuyruk indislerini başa döndürür.
 * 
 * @param cb Sıfırlanacak tampon pointer'ı.
 * @return None
 * 
 * @note Kritik bölge korumalıdır. Kesme (ISR) içinden güvenle çağrılabilir.
 */
void CB_Clear(CircularBuffer_t* cb) {
    if (!cb) return;
    CB_ENTER_CRITICAL();
    cb->head = 0;
    cb->tail = 0;
    cb->is_full = false;
    CB_EXIT_CRITICAL();
}

/**
 * @brief Tamponda bulunan veri miktarını döndürür.
 * 
 * @param cb Tampon pointer'ı.
 * @return size_t Tamponda okunmayı bekleyen toplam veri miktarı. [byte]
 * 
 * @note Atomik okuma yaptığı için iç içe kritik bölgelerden kaçınır. ISR içinden çağrılabilir.
 */
size_t CB_GetDataSize(const CircularBuffer_t* cb) {
    if (!cb) return 0;
    
    // Read volatile values atomically to avoid critical section in const function
    volatile size_t head_snapshot = cb->head;
    volatile size_t tail_snapshot = cb->tail;
    volatile bool is_full_snapshot = cb->is_full;
    
    if (is_full_snapshot) {
        return cb->size;
    } else {
        if (head_snapshot >= tail_snapshot) {
            return head_snapshot - tail_snapshot;
        } else {
            return cb->size - tail_snapshot + head_snapshot;
        }
    }
}

/**
 * @brief Tampondaki boş alan miktarını döndürür.
 * 
 * @param cb Tampon pointer'ı.
 * @return size_t Yazılabilecek boş alan miktarı. [byte]
 * 
 * @note Kritik bölge korumalıdır. ISR içinden çağrılabilir.
 */
size_t CB_GetFreeSpace(const CircularBuffer_t* cb) {
    if (!cb) return 0;
    
    // Avoid nested critical section - inline the calculation
    size_t data_size = 0;
    CB_ENTER_CRITICAL();
    if (cb->is_full) {
        data_size = cb->size;
    } else {
        if (cb->head >= cb->tail) {
            data_size = cb->head - cb->tail;
        } else {
            data_size = cb->size - cb->tail + cb->head;
        }
    }
    CB_EXIT_CRITICAL();
    
    return cb->size - data_size;
}

/**
 * @brief Tampona veri yazar.
 * 
 * Verilen veriyi dairesel yapıya uygun şekilde tamponun baş (head) kısmına ekler.
 * Boş yer yoksa yazma işlemi yapılmaz.
 * 
 * @param cb Tampon pointer'ı.
 * @param data Yazılacak veri bloğu. [byte array]
 * @param len Yazılacak veri uzunluğu. [byte]
 * @return bool Yazma başarılıysa true, yer yoksa false döner.
 * 
 * @note Kritik bölge korumalıdır. ISR içinden (örn. ADC verisi toplarken) güvenle çağrılabilir. 
 *       Ancak `len` boyutu çok büyükse kesme gecikmesine (jitter) neden olabilir.
 */
bool CB_Write(CircularBuffer_t* cb, const uint8_t* data, size_t len) {
    if (!cb || !data || len == 0) return false;
    
    // Check free space inline to avoid nested critical section
    CB_ENTER_CRITICAL();
    
    // Calculate free space
    size_t free_space;
    if (cb->is_full) {
        free_space = 0;
    } else {
        size_t data_size;
        if (cb->head >= cb->tail) {
            data_size = cb->head - cb->tail;
        } else {
            data_size = cb->size - cb->tail + cb->head;
        }
        free_space = cb->size - data_size;
    }
    
    // Check if we have enough space
    if (len > free_space) {
        CB_EXIT_CRITICAL();
        return false;
    }
    
    // Write data
    for (size_t i = 0; i < len; i++) {
        cb->buffer[cb->head] = data[i];
        cb->head = (cb->head + 1) % cb->size;
    }
    
    if (cb->head == cb->tail) {
        cb->is_full = true;
    }
    
    CB_EXIT_CRITICAL();
    return true;
}

/**
 * @brief Tampondan veri okur.
 * 
 * Belirtilen miktarda veriyi tamponun kuyruk (tail) kısmından çeker.
 * 
 * @param cb Tampon pointer'ı.
 * @param data Okunan verinin yazılacağı yer. [byte array]
 * @param len Okunmak istenen maksimum miktar. [byte]
 * @return size_t Fiilen okunan veri miktarı. [byte]
 * 
 * @note Kritik bölge korumalıdır. Genellikle RTOS task'larında veri işlemek için kullanılır. 
 *       ISR içinden çağrılması gerekirse işlem süresine dikkat edilmelidir.
 */
size_t CB_Read(CircularBuffer_t* cb, uint8_t* data, size_t len) {
    if (!cb || !data || len == 0) return 0;
    
    size_t data_size = CB_GetDataSize(cb);
    size_t bytes_to_read = (len > data_size) ? data_size : len;

    CB_ENTER_CRITICAL();
    for (size_t i = 0; i < bytes_to_read; i++) {
        data[i] = cb->buffer[cb->tail];
        cb->tail = (cb->tail + 1) % cb->size;
    }
    cb->is_full = false;
    CB_EXIT_CRITICAL();
    
    return bytes_to_read;
}
