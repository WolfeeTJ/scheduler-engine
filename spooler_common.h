// $Id: spooler_common.h,v 1.7 2001/02/16 18:23:12 jz Exp $

#ifndef __SPOOLER_COMMON_H
#define __SPOOLER_COMMON_H

namespace sos {
namespace spooler {

#ifdef SYSTEM_WIN
#   define DIR_SEP "\\"
# else
#   define DIR_SEP "/"
#endif


typedef uint                    Thread_id;                  // _beginthreadex()
typedef DWORD                   Process_id;

//-------------------------------------------------------------------------------------------Handle

struct Handle
{
#   ifdef SYSTEM_WIN
                                Handle                      ( HANDLE h = NULL )             : _handle(h) {}
                                Handle                      ( ulong h )                     : _handle((HANDLE)h) {}     // f�r _beginthread()
                               ~Handle                      ()                              { close(); }

        void                    operator =                  ( HANDLE h )                    { set_handle( h ); }
        void                    operator =                  ( ulong h )                     { set_handle( (HANDLE)h ); }   // f�r _beginthread()
                                operator HANDLE             () const                        { return _handle; }
                                operator !                  () const                        { return _handle == 0; }
      //HANDLE*                 operator &                  ()                              { return &_handle; }

        void                    set_handle                  ( HANDLE h )                    { close(); _handle = h; }
        HANDLE                  handle                      () const                        { return _handle; }
        void                    close                       ()                              { if(_handle) { CloseHandle(_handle); _handle=0; } }

        HANDLE                 _handle;
#   endif

  private:
                                Handle                      ( const Handle& );              // Nicht implementiert
    void                        operator =                  ( const Handle& );              // Nicht implementiert
};

static HANDLE null_handle = NULL;

//-------------------------------------------------------------------------------------------Atomic

template<typename T>
struct Atomic
{
    typedef sos::Thread_semaphore::Guard Guard;


                                Atomic                      ( const T& t = T() )    : _value(t) {}

    Atomic&                     operator =                  ( const T& t )          { Guard g = &_lock; ref() = t; return *this; }
                                operator T                  ()                      { Guard g = &_lock; return ref(); }
    T                           read_and_reset              ()                      { return read_and_set( T() ); }
    T                           read_and_set                ( const T& t )          { Guard g = &_lock; T v = _value; _value = t; return v; }
    T&                          ref                         ()                      { return ref(); }

    volatile T                 _value;
    sos::Thread_semaphore      _lock;
};

//------------------------------------------------------------------------------------Simple_atomic
// F�r Typen, die atomar lesbar und schreibbar sind.
// Erforderliche Operationen:
// T&                           operator =                  ( const T& ) atomic
//                              operator T                  () atomic
// bool                         operator ==                 ( const T& ) atomic

template<typename T>
struct Simple_atomic
{
    typedef sos::Thread_semaphore::Guard Guard;


                                Simple_atomic               ( const T& t = T() )    : _value(t) {}

    Simple_atomic&              operator =                  ( const T& t )          { _value = t; return *this; }
                                operator T                  ()                      { return _value; }

    T                           read_and_reset              ()                      { return read_and_set( T() ); }
    T                           read_and_set                ( const T& t )          { if( _value == t )  return _value;  Guard g = &_lock; T v = _value; _value = t; return v; }

    volatile T                 _value;
    sos::Thread_semaphore      _lock;
};

//-------------------------------------------------------------------------------------------------

} //namespace spooler
} //namespace sos

#endif
