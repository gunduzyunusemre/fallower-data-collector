
#ifndef SRC_STATE_EVENT_HPP_
#define SRC_STATE_EVENT_HPP_

enum EventType {
	Initialize,
	Ready,
	Calibrate,
    StartStreaming,
	StopStreaming,
};

class Event {
public:
    virtual EventType GetType() const = 0;
    virtual ~Event() {}
};

class InitializeEvent : public Event {
public:
    EventType GetType() const override { return Initialize; }
};

class CalibrateEvent : public Event {
public:
    EventType GetType() const override { return Calibrate; }
};

class ReadyEvent : public Event {
public:
    EventType GetType() const override { return Ready; }
};


class StartStreamingEvent : public Event {
public:
    EventType GetType() const override { return StartStreaming; }
};

class StopStreamingEvent : public Event {
public:
    EventType GetType() const override { return StopStreaming; }
};


#endif /* SRC_STATE_EVENT_HPP_ */
