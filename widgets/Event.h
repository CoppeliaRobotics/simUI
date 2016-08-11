#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED

#include <string>

class Event
{
protected:

public:
    Event();
    virtual ~Event();

    friend class UIFunctions;
};

class EventOnClick : public Event
{
protected:
    std::string onclick;

public:
    EventOnClick();
    virtual ~EventOnClick();

    friend class UIFunctions;
};

class EventOnChange : public Event
{
protected:
    std::string onchange;

public:
    EventOnChange();
    virtual ~EventOnChange();

    friend class UIFunctions;
};

class EventOnChangeInt : public EventOnChange
{
public:
    EventOnChangeInt();
    virtual ~EventOnChangeInt();

    friend class UIFunctions;
};

class EventOnChangeDouble : public EventOnChange
{
public:
    EventOnChangeDouble();
    virtual ~EventOnChangeDouble();

    friend class UIFunctions;
};

class EventOnChangeString : public EventOnChange
{
public:
    EventOnChangeString();
    virtual ~EventOnChangeString();

    friend class UIFunctions;
};

class EventOnEditingFinished : public Event
{
protected:
    std::string oneditingfinished;

public:
    EventOnEditingFinished();
    virtual ~EventOnEditingFinished();

    friend class UIFunctions;
};

#endif // EVENT_H_INCLUDED

