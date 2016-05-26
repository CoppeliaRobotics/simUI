#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED

#include <string>

class Event
{
protected:

    friend class UIFunctions;
};

class EventOnClick : public Event
{
protected:
    std::string onclick;

    friend class UIFunctions;
};

class EventOnChange : public Event
{
protected:
    std::string onchange;
    bool onchangeActive; // set to true only during the time the callback is running
                         // used to prevent stack overflow when triggering an event
                         // from the relative change callback

    friend class UIFunctions;
};

class EventOnChangeInt : public EventOnChange
{
    friend class UIFunctions;
};

class EventOnChangeString : public EventOnChange
{
    friend class UIFunctions;
};

#endif // EVENT_H_INCLUDED

