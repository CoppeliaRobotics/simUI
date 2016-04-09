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

