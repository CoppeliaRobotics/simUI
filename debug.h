#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED


#ifndef DEBUG_STREAM
#define DEBUG_STREAM std::cerr
#endif // DEBUG_STREAM


#ifdef __PRETTY_FUNCTION__
#define DBG_WHAT __PRETTY_FUNCTION__
#else
#define DBG_WHAT __func__
#endif


#ifdef NO_COLOR
#define DBG_WRAP_BEGIN
#define DBG_WRAP_END
#else
#define DBG_WRAP_BEGIN "\033[1;31m"
#define DBG_WRAP_END "\033[0m"
#endif


#ifdef DEBUG
#define DBG DEBUG_STREAM << DBG_WRAP_BEGIN << __FILE__ << ":" << __LINE__ << "  " << DBG_WHAT << DBG_WRAP_END << "  "
#else // DEBUG
#define DBG if(false) else DEBUG_STREAM
#endif // DEBUG


#endif // DEBUG_H_INCLUDED
