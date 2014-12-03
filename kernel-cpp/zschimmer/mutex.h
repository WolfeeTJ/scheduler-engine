// $Id: mutex.h 14119 2010-10-29 10:57:39Z jz $

#ifndef ZSCHIMMER_MUTEX_H
#define ZSCHIMMER_MUTEX_H

#include "mutex_base.h"

//-------------------------------------------------------------------------------------------------
#ifdef Z_WINDOWS
    #include "z_windows_mutex.h"
    namespace zschimmer 
    {
        typedef windows::Mutex          Mutex;
        typedef windows::System_mutex   System_mutex;
        using windows::enter_mutex;
        using windows::leave_mutex;
    }
#else
    #include "z_posix_mutex.h"
    namespace zschimmer 
    {
        typedef posix::Mutex            Mutex;
        typedef posix::System_mutex     System_mutex;
        using posix::enter_mutex;
        using posix::leave_mutex;
    }
#endif
//-------------------------------------------------------------------------------------------------

namespace zschimmer {

//--------------------------------------------------------------------------------------Mutex_guard

struct Mutex_guard : Non_cloneable
{
                                Mutex_guard                 ()                                      : _mutex(NULL), _function(""), _file(""), _line_nr(0) {}
                                Mutex_guard                 ( Mutex* m, const char* function = "", const char* file = "", int line = 0 ) { enter(m,function,file,line); }
                               ~Mutex_guard                 ()                                      { leave(); }

                                operator bool               () const                                { return _mutex != NULL; }
    void                        leave                       ()                                      { leave_();  __assume( _mutex == NULL ); }
    void                        enter                       ( Mutex* m, const char* function = "", const char* file="", int line=0 ) { enter_( m, function, file, line ); __assume(_mutex); }

  private:
    void                        enter_                      ( Mutex* m, const char* function, const char* file, int line );
    void                        leave_                      ();

    Mutex*                     _mutex;
    const char*                _function;
    const char*                _file;
    int                        _line_nr;
};

//---------------------------------------------------------------------------------Fast_mutex_guard

struct Fast_mutex_guard : Non_cloneable
{
                                Fast_mutex_guard            ()                                      : _mutex(NULL) {}
                                Fast_mutex_guard            (Mutex* m)                              { enter(m); }
                               ~Fast_mutex_guard            ()                                      { leave(); }

                                operator bool               () const                                { return _mutex != NULL; }
    void                        enter                       (Mutex* m)                              { m->enter(), _mutex = m; }
    void                        leave                       ()                                      { if (_mutex) _mutex->leave(), _mutex = NULL; }

  private:
    Mutex*                     _mutex;
};

//-----------------------------------------------------------------------------------Mutex_releaser
// Gibt gesperrtes Mutex vorübergehen frei, also das Gegenteil von Mutex_guard.

struct Mutex_releaser : Non_cloneable
{
                                Mutex_releaser              (Mutex* m)                              : _mutex(m), _leaved(false) { leave(); }
                               ~Mutex_releaser              ()                                      { enter(); }

    void                        enter                       ()                                      { if (_leaved)  _mutex->enter(), _leaved = false; }
    void                        leave                       ()                                      { assert(!_leaved); _mutex->leave();  _leaved = true; }

  private:
    Mutex*                     _mutex;
    bool                       _leaved;
};

//------------------------------------------------------------------------------------------Z_MUTEX
#ifdef __GNUC__
    #define Z_MUTEX( MUTEX )                                                                         \
        for( zschimmer::Mutex_guard __guard__ ( &MUTEX, "", __FILE__, __LINE__ ); __guard__; __guard__.leave() )   // mit Z_FUNCTION stürzt gcc 3.3.1 ab.
#else
    #define Z_MUTEX( MUTEX )                                                                         \
        for( zschimmer::Mutex_guard __guard__ ( &MUTEX, __FUNCTION__, __FILE__, __LINE__ ); __guard__; __guard__.leave() )
#endif

#define Z_FAST_MUTEX( MUTEX )                                                                         \
    for( zschimmer::Fast_mutex_guard __guard__ ( &MUTEX ); __guard__; __guard__.leave() )

//-----------------------------------------------------------------------------------Z_MUTEX_RETURN
#define Z_MUTEX_RETURN( LOCK, TYPE, EXPR )                                                           \
    {                                                                                                   \
        TYPE _result_;                                                                                  \
        Z_MUTEX(LOCK)  _result_ = EXPR;                                                                 \
        return EXPR;                                                                                    \
    }                                                                                                   \
    while(0)
//-----------------------------------------------------------------------------------My_thread_only

struct My_thread_only
{
                            My_thread_only              ();

    bool                    is_owners_thread            () const;
    void                    assert_is_owners_thread     () const;

    private:
    const Thread_id        _owners_thread_id;
};

//-----------------------------------------------------------------------------Z_COM_MY_THREAD_ONLY

#define Z_COM_MY_THREAD_ONLY \
    do {  if( !this->is_owners_thread() )  return E_ACCESSDENIED;  } while(0)

//-------------------------------------------------------------------------------------In_recursion

struct In_recursion
{
    In_recursion( bool* in_recursion_flag )    
    :
        _in_recursion_flag(in_recursion_flag)
    {
        _was_in_recursion = *_in_recursion_flag;
        *_in_recursion_flag = true;
    }


    ~In_recursion()    
    {
        *_in_recursion_flag = _was_in_recursion;
    }


    operator bool() 
    { 
        return _was_in_recursion; 
    }


    bool*   _in_recursion_flag;
    bool    _was_in_recursion;
};

//-------------------------------------------------------------------------------------------------

} //namespace zschimmer

#endif
