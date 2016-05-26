#include "Event.h"

Event::Event()
{
}

EventOnClick::EventOnClick()
    : onclick("")
{
}

EventOnChange::EventOnChange()
    : onchange(""),
      onchangeActive(false)
{
}

EventOnChangeInt::EventOnChangeInt()
{
}

EventOnChangeString::EventOnChangeString()
{
}

EventOnEditingFinished::EventOnEditingFinished()
    : oneditingfinished("")
{
}

