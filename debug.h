#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED


#ifndef DEBUG_STREAM
#define DEBUG_STREAM std::cerr
#endif // DEBUG_STREAM


#ifdef DEBUG
#define DBG DEBUG_STREAM << __FILE__ << ":" << __LINE__ << "  " << __func__ << "  "
#else // DEBUG
#define DBG if(false) else DEBUG_STREAM
#endif // DEBUG


#endif // DEBUG_H_INCLUDED
