
#ifndef SRC_RADARHANDLER_HPP_
#define SRC_RADARHANDLER_HPP_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "stm32h7xx_hal.h"
#include "cmsis_os.h"
#include <queue.h>
#include "main.h"
#include <string.h>
// COMMENTED: These includes cause build issues in Release mode
// radarHandler doesn't actually use these classes directly
// #include "state/Event.hpp"
// #include "state/Context.hpp"
// #include "state/Stream.hpp"
#include <memory>
// #include "state/StateMachine.hpp"

extern osMessageQueueId_t spiTransmitReceiveQueueHandle;

class radarHandler {
public:
	static int16_t calculate_frame_average();
	static int startStreamingTask(int frameCount);
	static void stopStreamingTask();
	static void streamTaskCallback(void *argument);
};

#endif /* SRC_RADARHANDLER_HPP_ */
