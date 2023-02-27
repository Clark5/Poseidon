#include "make-event.h"
#include "log.h"

NS_LOG_COMPONENT_DEFINE ("MakeEvent");

namespace ns3 {

EventImpl * MakeEvent (void (*f)(void))
{
  NS_LOG_FUNCTION (f);
  // zero arg version
  class EventFunctionImpl0 : public EventImpl
  {
public:
    typedef void (*F)(void);

    EventFunctionImpl0 (F function)
      : m_function (function)
    {
    }
    virtual ~EventFunctionImpl0 ()
    {
    }
protected:
    virtual void Notify (void)
    {
      (*m_function)();
    }
private:
    F m_function;
  } *ev = new EventFunctionImpl0 (f);
  return ev;
}

} // namespace ns3
