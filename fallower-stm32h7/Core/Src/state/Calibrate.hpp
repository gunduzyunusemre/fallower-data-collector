#ifndef SRC_STATE_CALIBRATE_HPP_
#define SRC_STATE_CALIBRATE_HPP_

#include "State.hpp"
#include <stdint.h>

class Calibrate : public State {
private:
    // Kalibrasyon işlemi için ADC frame ortalamasını hesaplar
    int16_t calculate_frame_average();

public:
    void HandleEvent(Event* event, void* args) override;
    const char* getStateName() const override { return "CALIBRATE"; }

    // Calibrate durumuna giriş yapıldığında çağrılacak
    void OnEnter() override;
};

#endif /* SRC_STATE_CALIBRATE_HPP_ */
