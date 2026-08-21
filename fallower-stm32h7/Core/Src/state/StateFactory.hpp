#ifndef SRC_STATE_STATEFACTORY_HPP_
#define SRC_STATE_STATEFACTORY_HPP_

// Forward declaration for State
class State;

enum class StateType {
    IDLE,   // Başlangıç state'i
    INIT,
    READY,
    STREAM,
    CALIBRATE
};

class StateFactory {
public:
    static State* createState(StateType type);
};

#endif /* SRC_STATE_STATEFACTORY_HPP_ */
