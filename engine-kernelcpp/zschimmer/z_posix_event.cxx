// $Id: z_posix_event.cxx 13466 2008-03-15 16:31:51Z jz $

#include "zschimmer.h"
#include "z_posix.h"
#include "mutex.h"
#include "threads.h"
#include "log.h"


#ifdef Z_UNIX

#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/socket.h>

#ifdef Z_SOLARIS
#   include <stropts.h>
#   include <sys/filio.h>       // ioctl( , FIONBIO )
#endif


const pthread_cond_t  pthread_cond_initial    = PTHREAD_COND_INITIALIZER;


namespace zschimmer {
namespace posix {

//-------------------------------------------------------------------------------------Event::Event

Event::Event( const string& name )
: 
    _zero_(this+1),
    _pthread_cond(pthread_cond_initial)
{
    _name = name;
    create();
}

//------------------------------------------------------------------------------------Event::~Event

Event::~Event()
{
    close();
}

//-------------------------------------------------------------------------------------Event::close

void Event::close()
{
    if( _created )
    {
        int err = pthread_cond_destroy( &_pthread_cond );
        if( err )  Z_LOG( "pthread_cond_destroy err=" << err << '\n' );

        _created = false;
    }
}

//------------------------------------------------------------------------------------Event::create

void Event::create()
{
    int err = pthread_cond_init( &_pthread_cond, NULL );
    if( err )  throw_errno( err, "pthread_cond_init", name().c_str() );

    _created = true;
}

//-------------------------------------------------------------------------------------Event::signal

void Event::signal( const string& name )
{
    Z_MUTEX( _mutex )
    {
        _signal_name = name;
        _signaled    = true;

        if( _waiting )
        {
            Z_LOG( "pthread_cond_signal " << _name << " \"" << _signal_name << "\"\n" );

            int err = pthread_cond_signal( &_pthread_cond );
            if( err )  throw_errno( err, "pthread_cond_signal", _name.c_str(), name.c_str() );
        }
    }
}

//------------------------------------------------------------------------------Event::async_signal

void Event::async_signal( const char* )
{
    // pthread_mutex_lock:
    // The  mutex  functions  are  not  async-signal  safe.  What  this  means  is  that  they
    // should  not  be  called from  a signal handler. In particular, calling pthread_mutex_lock 
    // or pthread_mutex_unlock from a signal handler may deadlock the calling thread.

    _signaled = true;
}

//------------------------------------------------------------------------Event::signaled_then_reset

bool Event::signaled_then_reset()
{
    if( !_signaled )  return false;
    
    bool signaled = false;

    Z_MUTEX( _mutex )
    {
        signaled = _signaled;
        reset();
    }

    return signaled;
}

//-------------------------------------------------------------------------------------Event::reset

void Event::reset()
{
    Z_MUTEX( _mutex )
    {
        _signaled = false;
        _signal_name = "";

        //BOOL ok = ResetEvent( _handle );
        //if( !ok )  throw_mswin( "ResetEvent", _name.c_str() );
    }
}

//---------------------------------------------------------------------------------------Event::wait

void Event::wait()
{
    Z_MUTEX( _mutex )
    {
        if( _signaled )  return;

        _waiting++;
        
        Z_DEBUG_ONLY( Z_LOG( "pthread_cond_wait " << _name << "\n" ) );
        int err = pthread_cond_wait( &_pthread_cond, &_mutex._system_mutex );   // _mutex wird hier freigegeben und wieder gesperrt!
        
        _waiting--;

        if( err )
        {
            if( err == EINTR )  Z_LOG( "err=EINTR\n" );
            else throw_errno( err, "pthread_cond_wait", _name.c_str() );
        }
        else
        {
            Z_DEBUG_ONLY( Z_LOG( "pthread_cond_wait: " << as_text() << " signalled!\n" ) );
        }
    }
}

//--------------------------------------------------------------------------------------Event::wait

bool Event::wait( double seconds )
{
    Z_MUTEX( _mutex )
    {
        if( _signaled )  { 
            //Z_LOG( "pthread_cond_timedwait() nicht gerufen, weil _signaled==true\n" ); 
            return true; 
        }

        _waiting++;

        struct timeval  now;
        struct timespec t;
        struct timezone tz;
        
        gettimeofday( &now, &tz );

        memset( &t, 0, sizeof t );
        timespec_add( &t, seconds );
        timespec_add( &t, now );

        Z_DEBUG_ONLY( Z_LOG( "pthread_cond_timedwait(" << seconds << ") " << _name << '\n' ) );

        int err = pthread_cond_timedwait( &_pthread_cond, &_mutex._system_mutex, &t );

        _waiting--;

        if( err )  
        {
            if( err == ETIMEDOUT )  return false;
            if( err == EINTR     )  { Z_LOG( "pthread_cond_timedwait() err=EINTR\n" ); return true; }
            throw_errno( err, "pthread_cond_timedwait", _name.c_str() );
        }

        Z_DEBUG_ONLY( Z_LOG( "pthread_cond_timedwait: " << as_text() << " signalled!\n" ) );
    }

    return true;
}

//-------------------------------------------------------------------------------------------------

} //namespace posix
} //namespace zschimmer

#endif
