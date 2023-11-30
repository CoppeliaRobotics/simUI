#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED

#include "config.h"

#include <string>

class Event
{
protected:

public:
    Event();
    virtual ~Event();

    friend class SIM;
};

class EventOnClick : public Event
{
protected:
    std::string onclick;

public:
    EventOnClick();
    virtual ~EventOnClick();

    friend class SIM;
};

class EventOnChange : public Event
{
protected:
    std::string onchange;

public:
    EventOnChange();
    virtual ~EventOnChange();

    friend class SIM;
};

class EventOnChangeInt : public EventOnChange
{
public:
    EventOnChangeInt();
    virtual ~EventOnChangeInt();

    friend class SIM;
};

class EventOnChangeDouble : public EventOnChange
{
public:
    EventOnChangeDouble();
    virtual ~EventOnChangeDouble();

    friend class SIM;
};

class EventOnChangeString : public EventOnChange
{
public:
    EventOnChangeString();
    virtual ~EventOnChangeString();

    friend class SIM;
};

class EventOnEditingFinished : public Event
{
protected:
    std::string oneditingfinished;

public:
    EventOnEditingFinished();
    virtual ~EventOnEditingFinished();

    friend class SIM;
};

class EventOnLinkActivated : public Event
{
protected:
    std::string onLinkActivated;

public:
    EventOnLinkActivated();
    virtual ~EventOnLinkActivated();

    friend class SIM;
};

class EventOnKeyPress : public Event
{
protected:
    std::string onKeyPress;

public:
    EventOnKeyPress();
    virtual ~EventOnKeyPress();

    friend class SIM;
};

class EventOnMouseDown : public Event
{
protected:
    std::string onMouseDown;

public:
    EventOnMouseDown();
    virtual ~EventOnMouseDown();

    friend class SIM;
};

class EventOnMouseUp : public Event
{
protected:
    std::string onMouseUp;

public:
    EventOnMouseUp();
    virtual ~EventOnMouseUp();

    friend class SIM;
};

class EventOnMouseMove : public Event
{
protected:
    std::string onMouseMove;

public:
    EventOnMouseMove();
    virtual ~EventOnMouseMove();

    friend class SIM;
};

#endif // EVENT_H_INCLUDED

