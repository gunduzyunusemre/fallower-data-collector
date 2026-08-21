// 5. Let's fix StateFactory.cpp
// StateFactory.cpp
#include "StateFactory.hpp"
#include "Idle.hpp"
#include "Init.hpp"
#include "Ready.hpp"
#include "Stream.hpp"
#include "Calibrate.hpp"

State* StateFactory::createState(StateType type) {
    switch (type) {
        case StateType::IDLE:
            return new Idle();
        case StateType::INIT:
            return new Init();
        case StateType::READY:
            return new Ready();
        case StateType::STREAM:
            return new Stream();
        case StateType::CALIBRATE:
            return new Calibrate();
        default:
            // İsteğe bağlı: Hata durumunda Idle'a dönebilir veya null dönebilir
            return new Idle(); // veya return nullptr;
    }
}
