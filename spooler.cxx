// $Id: spooler.cxx,v 1.137 2002/11/24 15:32:57 jz Exp $
/*
    Hier sind implementiert

    Script_instance
    Spooler
    spooler_main()
    sos_main()
*/

#include "spooler.h"
#include "spooler_version.h"

#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#ifdef Z_WINDOWS
#   include <direct.h>
#endif

#include "../kram/sosprof.h"
#include "../kram/sosopt.h"
#include "../kram/sleep.h"
#include "../kram/log.h"
#include "../file/anyfile.h"
#include "../kram/licence.h"
#include "../kram/sos_mail.h"


namespace sos {

extern const Bool _dll = false;

namespace spooler {

const char* default_factory_ini  = "factory.ini";
const string new_suffix          = "~new";  // Suffix f�r den neuen Spooler, der den bisherigen beim Neustart ersetzen soll
const double renew_wait_interval = 0.25;
const double renew_wait_time     = 30;      // Wartezeit f�r Br�ckenspooler, bis der alte Spooler beendet ist und der neue gestartet werden kann.
const double wait_for_thread_termination                 = latter_day;  // Haltbarkeit des Geduldfadens
const double wait_step_for_thread_termination            = 5.0;         // 1. N�rgelabstand
const double wait_step_for_thread_termination2           = 600.0;       // 2. N�rgelabstand
//const double wait_for_thread_termination_after_interrupt = 1.0;


bool                            spooler_is_running      = false;
bool                            ctrl_c_pressed          = false;
static Spooler*                 spooler                 = NULL;


/*
struct Set_console_code_page
{
                                Set_console_code_page       ( uint cp )         { _cp = GetConsoleOutputCP(); SetConsoleOutputCP( cp ); }
                               ~Set_console_code_page       ()                  { SetConsoleOutputCP( _cp ); }

    uint                       _cp;
};
*/

static void set_ctrl_c_handler( bool on );

//---------------------------------------------------------------------------------send_error_email

void send_error_email( const string& error_text, int argc, char** argv )
{
    try
    {
        string from = read_profile_string( default_factory_ini, "spooler", "log_mail_from"   );
        string to   = read_profile_string( default_factory_ini, "spooler", "log_mail_to"     );
        string cc   = read_profile_string( default_factory_ini, "spooler", "log_mail_cc"     );
        string bcc  = read_profile_string( default_factory_ini, "spooler", "log_mail_bcc"    );
        string smtp = read_profile_string( default_factory_ini, "spooler", "smtp"            );

        Mail_message msg;
        msg.init();
        msg.set_from( from );

        if( to   != "" )  msg.set_to  ( to   );
        if( cc   != "" )  msg.set_cc  ( cc   );
        if( bcc  != "" )  msg.set_bcc ( bcc  );
        if( smtp != "" )  msg.set_smtp( smtp );

        msg.set_subject( "FEHLER BEI SPOOLER-START: " + error_text );
        msg.add_header_field( "X-SOS-Spooler", "" );

        string body = "Der Spooler-Dienst konnte nicht gestartet werden.\n"
                      "\n"
                      "\n"
                      "Der Aufruf war:\n"
                      "\n";
                       

        for( int i = 0; i < argc; i++ )  body += argv[i], body += ' ';
        body += "\n\n\n"
                "Fehlermeldung:\n";
        body += error_text;
        msg.set_body( body );

        try
        {
            msg.send(); 
        }
        catch( const Xc& ) 
        { 
            msg.enqueue(); 
        }
    }
    catch( const Xc& ) {}
}

//---------------------------------------------------------------------------------thread_info_text
#ifdef Z_WINDOWS

static string thread_info_text( HANDLE h )
{
    FILETIME creation_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    DWORD    exit_code;
    BOOL     ok;
    char     buffer [200];
    string   result;


    ok = GetExitCodeThread( h, &exit_code );
    if( ok )  if( exit_code == STILL_ACTIVE )  result += "active";
                                         else  result = "terminated with exit code " + as_string(exit_code);
         else result = "Unbekannter Thread " + as_hex_string((int)h);

    ok = GetThreadTimes( h, &creation_time, &exit_time, &kernel_time, &user_time );
    if( ok )
    {
        sprintf( buffer, ", user_time=%-0.10g, kernel_time=%-0.10g", 
                 (double)windows::int64_from_filetime( user_time   ) / 1E7,
                 (double)windows::int64_from_filetime( kernel_time ) / 1E7 );

        result += buffer;
    }

    return result;
}

#endif
//---------------------------------------------------------------------read_profile_mail_on_process

int read_profile_mail_on_process( const string& profile, const string& section, const string& entry, int deflt )
{
    string v = read_profile_string( profile, section, entry );

    if( v == "" )  return deflt;

    try
    {
        if( isdigit( (uint)v[0] ) )  return as_int(v);
                               else  return as_bool(v);
    }
    catch( const Xc& ) { return deflt; }
}

//------------------------------------------------------------------read_profile_history_on_process

int read_profile_history_on_process( const string& profile, const string& section, const string& entry, int deflt )
{
    string result;
    string v = read_profile_string( profile, section, entry );

    if( v == "" )  return deflt;

    try
    {
        if( isdigit( (uint)v[0] ) )  return as_int(v);
                               else  return as_bool(v);
    }
    catch( const Xc& ) { return deflt; }
}

//-----------------------------------------------------------------------------read_profile_archive

Archive_switch read_profile_archive( const string& profile, const string& section, const string& entry, Archive_switch deflt )
{
    string value = read_profile_string( profile, section, entry );

    if( value == "" )  return deflt;
    if( lcase(value) == "gzip" )  return arc_gzip;

    return read_profile_bool( profile, section, entry, false )? arc_yes : arc_no;
}

//----------------------------------------------------------------------------read_profile_with_log

With_log_switch read_profile_with_log( const string& profile, const string& section, const string& entry, With_log_switch deflt )
{
    return read_profile_archive( profile, section, entry, deflt );
}

//-----------------------------------------------------------------------------------ctrl_c_handler
#ifdef Z_WINDOWS

    static BOOL WINAPI ctrl_c_handler( DWORD dwCtrlType )
    {
        if( dwCtrlType == CTRL_C_EVENT  &&  !ctrl_c_pressed )
        {
            ctrl_c_pressed = true;
            fprintf( stderr, "Spooler wird wegen Ctrl-C beendet ...\n" );
            spooler->signal( "Ctrl+C" );
            return true;
        }
        else
            return false;
    }

#else

    static void ctrl_c_handler( int sig )
    {
        set_ctrl_c_handler( false );

        //if( !ctrl_c_pressed )
        //{
            ctrl_c_pressed = true;
            fprintf( stderr, "Spooler wird wegen Ctrl-C beendet ...\n" );
            spooler->signal( "Ctrl+C" );
        //}
    }

#endif
//-------------------------------------------------------------------------------set_ctrl_c_handler

static void set_ctrl_c_handler( bool on )
{
#   ifdef Z_WINDOWS

        SetConsoleCtrlHandler( ctrl_c_handler, on );

#    else

        ::signal( SIGINT , on? ctrl_c_handler : SIG_DFL );
        ::signal( SIGTERM, on? ctrl_c_handler : SIG_DFL );

#   endif
}

//---------------------------------------------------------------------------------Spooler::Spooler

Spooler::Spooler() 
: 
    _zero_(this+1), 
    _security(this),
    _communication(this), 
    _java_vm(this),
    _prefix_log(1),
    _wait_handles(this,&_prefix_log),
    _log(this),
    _module(this,&_prefix_log),
    _log_level( log_info ),
    _db(this),
    _factory_ini( default_factory_ini ),

    _smtp_server   ("-"),   // F�r spooler_log.cxx: Nicht setzen, damit Default aus sos.ini erhalten bleibt
    _log_mail_from ("-"),
    _log_mail_cc   ("-"),
    _log_mail_bcc  ("-"),
    _mail_queue_dir("-")
{

    _tcp_port = 4444;
    _udp_port = 4444;
    _priority_max = 1000;       // Ein Wert > 1, denn 1 ist die voreingestelle Priorit�t der Jobs
            

    _com_log     = new Com_log( &_prefix_log );
    _com_spooler = new Com_spooler( this );
    _variables   = new Com_variable_set();

    set_ctrl_c_handler( true );
    spooler = this;
}

//--------------------------------------------------------------------------------Spooler::~Spooler

Spooler::~Spooler() 
{
    spooler = NULL;
    set_ctrl_c_handler( false );

    if( !_thread_list.empty() )  
    {
        wait_until_threads_stopped( latter_day );
        _thread_list.clear();
    }

    _object_set_class_list.clear();

    _communication.close(0.0);
    _security.clear();

    _event.close();
    _wait_handles.close();

    //nicht n�tig  Z_FOR_EACH( Job_chain_map, _job_chain_map, it )  it->second->close();

    // COM-Objekte entkoppeln, falls noch jemand eine Referenz darauf hat:
    if( _com_spooler )  _com_spooler->close();
    if( _com_log     )  _com_log->close();
}

//--------------------------------------------------------------------------Spooler::security_level
// Anderer Thread

Security::Level Spooler::security_level( const Host& host )
{
    Security::Level result = Security::seclev_none;

    THREAD_LOCK( _lock )
    {
        result = _security.level( host );
    }

    return result;
}

//--------------------------------------------------------------------------Spooler::threads_as_xml
// Anderer Thread

xml::Element_ptr Spooler::threads_as_xml( const xml::Document_ptr& document, Show_what show )
{
    xml::Element_ptr threads = document.createElement( "threads" );

    dom_append_nl( threads );

    THREAD_LOCK_LOG( _lock, "threads_as_xml" )
    {
        FOR_EACH( Thread_list, _thread_list, it )
        {
            threads.appendChild( (*it)->dom( document, show ) );
            dom_append_nl( threads );
        }
    }

    return threads;
}

//--------------------------------------------------------------Spooler::wait_until_threads_stopped
#ifdef SPOOLER_USE_THREADS

void Spooler::wait_until_threads_stopped( Time until )
{
    assert( current_thread_id() == _thread_id );

    Wait_handles wait_handles ( this, &_prefix_log );

    Thread_list::iterator it = _thread_list.begin();
    while( it != _thread_list.end() )
    {
        Spooler_thread* thread = *it;
        if( !thread->_terminated  &&  thread->_thread_handle.handle() )  wait_handles.add_handle( (*it)->_thread_handle.handle() );
        it++;
    }

    int c = 0;
    while( wait_handles.length() > 0 )
    {
        Time until_step = Time::now() + (++c < 10? wait_step_for_thread_termination : wait_step_for_thread_termination2 );
        if( until_step > until )  until_step = until;

        int index = wait_handles.wait_until( until_step );
        if( index >= 0 ) 
        {
            HANDLE h = wait_handles[index];
            FOR_EACH( Thread_list, _thread_list, it )  
            {
                Spooler_thread* thread = *it;
                if( thread->_thread_handle.handle() == h ) 
                {
                    _log.info( "Thread " + thread->name() + " beendet" );
                    wait_handles.remove_handle( h );
                    //18.10.2002 THREAD_LOCK( _lock )  _thread_list.erase( thread );
                }
            }
        }

        if( wait_handles.length() > 0 )  sos_sleep( 0.01 );  // Zur Versch�nerung: N�chsten Threads Zeit lassen, sich zu beenden

        if( Time::now() > until )  break;

        if( index < 0 )
        {
            FOR_EACH( Thread_list, _thread_list, it )  
            {
                Spooler_thread* thread = *it;

                if( !thread->_terminated )
                {
                    string msg = "Warten auf Thread " + thread->name() + " [" + thread_info_text( thread->_thread_handle.handle() ) + "]";
                    Job* job = thread->_current_job;
                    if( job )  msg += ", Job " + job->name() + " " + job->job_state();
                    _log.info( msg );
                }
            }
        }
    }
}

#endif
//--------------------------------------------------------------------------Spooler::signal_threads

void Spooler::signal_threads( const string& signal_name )
{
    assert( current_thread_id() == _thread_id );

    FOR_EACH( Thread_list, _thread_list, it )  (*it)->signal( signal_name );
}

//------------------------------------------------------------------------------Spooler::get_thread
// Anderer Thread

Spooler_thread* Spooler::get_thread( const string& thread_name )
{
    Spooler_thread* thread = get_thread_or_null( thread_name );
    if( !thread )  throw_xc( "SPOOLER-128", thread_name );

    return thread;
}

//----------------------------------------------------------------------Spooler::get_thread_or_null
// Anderer Thread

Spooler_thread* Spooler::get_thread_or_null( const string& thread_name )
{
    THREAD_LOCK( _lock )
    {
        FOR_EACH( Thread_list, _thread_list, it )  if( stricmp( (*it)->name().c_str(), thread_name.c_str() ) == 0 )  return *it;
    }

    return NULL;
}

//--------------------------------------------------------------------Spooler::get_object_set_class
// Anderer Thread

Object_set_class* Spooler::get_object_set_class( const string& name )
{
    Object_set_class* c = get_object_set_class_or_null( name );
    if( !c )  throw_xc( "SPOOLER-101", name );
    return c;
}

//-------------------------------------------------------------Spooler::get_object_set_class_or_null
// Anderer Thread

Object_set_class* Spooler::get_object_set_class_or_null( const string& name )
{
    THREAD_LOCK( _lock )
    {
        FOR_EACH( Object_set_class_list, _object_set_class_list, it )
        {
            if( (*it)->_name == name )  return *it;
        }
    }

    return NULL;
}

//---------------------------------------------------------------------------------Spooler::get_job
// Anderer Thread

Job* Spooler::get_job( const string& job_name )
{
    Job* job = get_job_or_null( job_name );
    if( !job  ||  !job->state() )  throw_xc( "SPOOLER-108", job_name );
    return job;
}

//-------------------------------------------------------------------------Spooler::get_job_or_null
// Anderer Thread

Job* Spooler::get_job_or_null( const string& job_name )
{
    THREAD_LOCK_LOG( _lock, "Spooler::get_job_or_null" )
    {
        FOR_EACH( Thread_list, _thread_list, it )
        {
            Job* job = (*it)->get_job_or_null( job_name );
            if( job )  return job;
        }
    }

    return NULL;
}

//---------------------------------------------------------------------Spooler::thread_by_thread_id

Spooler_thread* Spooler::thread_by_thread_id( Thread_id id )                    
{     
    Thread_id_map::iterator it;

    THREAD_LOCK( _thread_id_map_lock )  it = _thread_id_map.find(id); 

    return it != _thread_id_map.end()? it->second : NULL; 
}

//---------------------------------------------------------------------------Spooler::signal_object
// Anderer Thread

void Spooler::signal_object( const string& object_set_class_name, const Level& level )
{
    THREAD_LOCK_LOG( _lock, "Spooler::signal_object" )  FOR_EACH( Thread_list, _thread_list, t )  (*t)->signal_object( object_set_class_name, level );
}

//-------------------------------------------------------------------------------Spooler::set_state

void Spooler::set_state( State state )
{
    assert( current_thread_id() == _thread_id );

    if( _state == state )  return;

    _state = state;
    _log.info( state_name() );

    if( _state_changed_handler )  (*_state_changed_handler)( this, NULL );
}

//------------------------------------------------------------------------------Spooler::state_name

string Spooler::state_name( State state )
{
    switch( state )
    {
        case s_stopped:             return "stopped";
        case s_starting:            return "starting";
        case s_running:             return "running";
        case s_paused:              return "paused";
        case s_stopping:            return "stopping";
        case s_stopping_let_run:    return "stopping_let_run";
        default:                    return as_string( (int)state );
    }
}

//--------------------------------------------------------------------------------Spooler::load_arg

void Spooler::load_arg()
{
    assert( current_thread_id() == _thread_id );

    for( Sos_option_iterator opt ( _argc, _argv ); !opt.end(); opt.next() )
    {
        if( opt.with_value( "ini" ) )  _factory_ini = opt.value();
    }


    string log_level = as_string( _log_level );

    _spooler_id         = read_profile_string    ( _factory_ini, "spooler", "id"                 );
    _config_filename    = read_profile_string    ( _factory_ini, "spooler", "config"             );
    _log_directory      = read_profile_string    ( _factory_ini, "spooler", "log-dir"            );  // veraltet
    _log_directory      = read_profile_string    ( _factory_ini, "spooler", "log_dir"            , _log_directory );  _log_directory_as_option_set = !_log_directory.empty();
    _include_path       = read_profile_string    ( _factory_ini, "spooler", "include-path"       );  // veraltet
    _include_path       = read_profile_string    ( _factory_ini, "spooler", "include_path"       , _include_path );   _include_path_as_option_set  = !_include_path.empty();
    _spooler_param      = read_profile_string    ( _factory_ini, "spooler", "param"              );                   _spooler_param_as_option_set = !_spooler_param.empty();
    log_level           = read_profile_string    ( _factory_ini, "spooler", "log_level"          , log_level );   
    _history_columns    = read_profile_string    ( _factory_ini, "spooler", "history_columns"    );
    _history_yes        = read_profile_bool      ( _factory_ini, "spooler", "history"            , true );
    _history_on_process = read_profile_history_on_process( _factory_ini, "spooler", "history_on_process", 0 );
    _history_archive    = read_profile_archive   ( _factory_ini, "spooler", "history_archive"    , arc_no );
    _history_with_log   = read_profile_with_log  ( _factory_ini, "spooler", "history_with_log"   , arc_no );
    _db_name            = read_profile_string    ( _factory_ini, "spooler", "db"                 );
    _need_db            = read_profile_bool      ( _factory_ini, "spooler", "need_db"            , true                );
    _history_tablename  = read_profile_string    ( _factory_ini, "spooler", "db_history_table"   , "spooler_history"   );
    _variables_tablename= read_profile_string    ( _factory_ini, "spooler", "db_variables_table" , "spooler_variables" );
    _java_vm._filename  = read_profile_string    ( _factory_ini, "java"   , "vm"                 , "msjava.dll"        );
  //_java_vm._ini_options= read_profile_string   ( _factory_ini, "java"   , "options" );
    _java_vm._ini_class_path= read_profile_string( _factory_ini, "java"   , "class_path" );
    _java_vm._javac     = read_profile_string    ( _factory_ini, "java"   , "javac"              , "javac"             );


    try
    {
        for( Sos_option_iterator opt ( _argc, _argv ); !opt.end(); opt.next() )
        {
            if( opt.flag      ( "V"                ) )  ;   // wurde in sos_main() bearbeitet
            else
            if( opt.flag      ( "service"          ) )  ;   // wurde in sos_main() bearbeitet
            else
            if( opt.with_value( "service"          ) )  ;   // wurde in sos_main() bearbeitet
            else
            if( opt.with_value( "log"              ) )  ;   // wurde in sos_main() bearbeitet
            else
            if( opt.with_value( "ini"              ) )  ;   //
            else
            if( opt.with_value( "config"           ) )  _config_filename = opt.value();
            else
            if( opt.with_value( "cd"               ) )  { string dir = opt.value(); if( chdir( dir.c_str() ) )  throw_errno( errno, "chdir", dir.c_str() ); }
            else
            if( opt.with_value( "id"               ) )  _spooler_id = opt.value();
            else
            if( opt.with_value( "log-dir"          ) )  _log_directory = opt.value(),  _log_directory_as_option_set = true;
            else
            if( opt.with_value( "include-path"     ) )  _include_path = opt.value(),  _include_path_as_option_set = true;
            else
            if( opt.with_value( "param"            ) )  _spooler_param = opt.value(),  _spooler_param_as_option_set = true;
            else
            if( opt.with_value( "log-level"        ) )  log_level = opt.value();
            else
            if( opt.with_value( "job"              ) )  _job_name = opt.value();        // Nicht von SOS beauftragt
            else
                throw_sos_option_error( opt );
        }

        _temp_dir = read_profile_string( _factory_ini, "spooler", "tmp", get_temp_path() + "spooler" );
        _temp_dir = replace_regex( _temp_dir, "[\\/]+", Z_DIR_SEPARATOR );
        _temp_dir = replace_regex( _temp_dir, "\\" Z_DIR_SEPARATOR "$", "" );
        if( _spooler_id != "" )  _temp_dir += Z_DIR_SEPARATOR + _spooler_id;

        _manual = !_job_name.empty();
        if( _manual  &&  _log_directory.empty() )  _log_directory = "*stderr";

        _log_level = make_log_level( log_level );

        if( _log_level <= log_debug_spooler )  _debug = true;
        if( _config_filename.empty() )  throw_xc( "SPOOLER-115" );
    }
    catch( const Sos_option_error& )
    {
        if( !_is_service )
        {
            cerr << "usage: " << _argv[0] << "\n"
                    "       -cd=PATH\n"
                    "       -config=XMLFILE\n"
                    "       -service-\n"
                    "       -log=HOSTWARELOGFILENAME\n"
                    "       -log-dir=DIRECTORY|*stderr\n"
                    "       -id=ID\n"
                    "       -param=PARAM\n"
                    "       -include-path=PATH\n"
                    "       -log-level=error|warn|info|debug|debug1|...|debug9\n";
        }

        throw;
    }
}

//------------------------------------------------------------------------------------Spooler::load

void Spooler::load()
{
    assert( current_thread_id() == _thread_id );

    set_state( s_starting );
    //_log ist noch nicht ge�ffnet   _log.info( "Spooler::load " + _config_filename );


    tzset();

    _security.clear();             
    load_arg();

    _prefix_log.init( this );

    char hostname[200];  // Nach _communication.init() und nach _prefix_log.init()!
    if( gethostname( hostname, sizeof hostname ) == SOCKET_ERROR )  hostname[0] = '\0',  _prefix_log.warn( string("gethostname(): ") + strerror( errno ) );
    _hostname = hostname;

    Command_processor cp ( this );
    cp.execute_file( _config_filename );
}

//-----------------------------------------------------------------------------------Spooler::start

void Spooler::start()
{
    assert( current_thread_id() == _thread_id );

    _state_cmd = sc_none;
    set_state( s_starting );

    _mail_on_error   = read_profile_bool           ( _factory_ini, "spooler", "mail_on_error"  , _mail_on_error );
    _mail_on_process = read_profile_mail_on_process( _factory_ini, "spooler", "mail_on_process", _mail_on_process );
    _mail_on_success = read_profile_bool           ( _factory_ini, "spooler", "mail_on_success", _mail_on_success );
    _mail_queue_dir  = read_profile_string         ( _factory_ini, "spooler", "mail_queue_dir" , _mail_queue_dir );
    _mail_encoding   = read_profile_string         ( _factory_ini, "spooler", "mail_encoding"  , "base64"        );      // "quoted-printable": Jmail braucht 1s pro 100KB daf�r
    _smtp_server     = read_profile_string         ( _factory_ini, "spooler", "smtp"           , _smtp_server );

    _log_mail_from      = read_profile_string( _factory_ini, "spooler", "log_mail_from"   );
    _log_mail_to        = read_profile_string( _factory_ini, "spooler", "log_mail_to"     );
    _log_mail_cc        = read_profile_string( _factory_ini, "spooler", "log_mail_cc"     );
    _log_mail_bcc       = read_profile_string( _factory_ini, "spooler", "log_mail_bcc"    );
    _log_mail_subject   = read_profile_string( _factory_ini, "spooler", "log_mail_subject");
    _log_collect_within = read_profile_uint  ( _factory_ini, "spooler", "log_collect_within", 0 );
    _log_collect_max    = read_profile_uint  ( _factory_ini, "spooler", "log_collect_max"   , 900 );

    _log.set_directory( _log_directory );
    _log.open_new();
    _log.info( string( "Spooler (" VER_PRODUCTVERSION_STR ) + ") startet mit " + _config_filename );


    _db.open( _db_name, _need_db );
    _db.spooler_start();

    if( !_manual )  _communication.start_or_rebind();

    _spooler_start_time = Time::now();

    if( _has_java  ) 
    {
        try
        {
            _java_vm.init();
        }
        catch( const exception& x )
        {
            _log.error( x.what() );
            _log.error( "Java kann nicht gestartet werden. Spooler startet ohne Java." );
        }

        set_ctrl_c_handler( false );     // Ctrl-C-Handler von Java �berschreiben (Suns Java beendet bei Ctrl-C den Prozess sofort)
        set_ctrl_c_handler( true );
    }


    {FOR_EACH( Thread_list, _thread_list, it )  if( !(*it)->empty() )  (*it)->init();}


    if( _module.set() )
    {
        _module_instance = _module.create_instance();
        _module_instance->init();

        _module_instance->add_obj( (IDispatch*)_com_spooler, "spooler"     );
        _module_instance->add_obj( (IDispatch*)_com_log    , "spooler_log" );

        _module_instance->load();
        _module_instance->start();

        bool ok = check_result( _module_instance->call_if_exists( "spooler_init()Z" ) );
        if( !ok )  throw_xc( "SPOOLER-127" );
    }

    FOR_EACH( Thread_list, _thread_list, it )  if( !(*it)->empty() )  (*it)->start_thread();
}

//------------------------------------------------------------------------------------Spooler::stop

void Spooler::stop()
{
    assert( current_thread_id() == _thread_id );

    set_state( _state_cmd == sc_let_run_terminate_and_restart? s_stopping_let_run : s_stopping );

    //_log.msg( "Spooler::stop" );

    signal_threads( "stop" );
    wait_until_threads_stopped( Time::now() + wait_for_thread_termination );

    FOR_EACH( Thread_list, _thread_list, it )  it = _thread_list.erase( it );


/*  interrupt() l�sst PerlScript abst�rzen
    FOR_EACH( Thread_list, _thread_list, it )
    {
        Job* job = (*it)->current_job();
        if( job )  try { job->interrupt_script(); } catch(const Xc& x){_log.error(x.what());}
    }

    wait_until_threads_stopped( Time::now() + wait_for_thread_termination_after_interrupt );
*/
    _object_set_class_list.clear();
    _thread_list.clear();

    if( _module_instance )  _module_instance->close();

    //_java_vm.close();  Erneutes _java.init() st�rzt ab, deshalb lassen wird Java stehen und schlie�en es erst am Schluss

    if( _state_cmd == sc_terminate_and_restart 
     || _state_cmd == sc_let_run_terminate_and_restart )  spooler_restart( &_log, _is_service );

    _db.spooler_stop();
    _db.close();

    set_state( s_stopped );     
    // Der Dienst ist hier beendet
}

//-------------------------------------------------------------------------------------Spooler::run

void Spooler::run()
{
    assert( current_thread_id() == _thread_id );

    set_state( s_running );

    while(1)
    {
        // Threads ohne Jobs und nach Fehler gestorbene Threads entfernen:
        //FOR_EACH( Thread_list, _thread_list, it )  if( (*it)->empty() )  THREAD_LOCK( _lock )  it = _thread_list.erase(it);
        bool valid_thread = false;
        FOR_EACH( Thread_list, _thread_list, it )  valid_thread |= !(*it)->_terminated;
        if( !valid_thread )  { _log.info( "Kein Thread vorhanden. Spooler wird beendet." ); break; }

        if( _state_cmd == sc_pause                 )  if( _state == s_running )  set_state( s_paused  ), signal_threads( "pause" );
        if( _state_cmd == sc_continue              )  if( _state == s_paused  )  set_state( s_running ), signal_threads( "continue" );
        if( _state_cmd == sc_load_config           )  break;
        if( _state_cmd == sc_reload                )  break;
        if( _state_cmd == sc_terminate             )  break;
        if( _state_cmd == sc_terminate_and_restart )  break;
        if( _state_cmd == sc_let_run_terminate_and_restart )  break;
        _state_cmd = sc_none;


#       ifdef SPOOLER_USE_THREADS
        {
            _wait_handles.wait_until( latter_day );
        }
#       else
        {
            Time            next_time = latter_day;
            Time            now       = Time::now();


            FOR_EACH( Thread_list, _thread_list, it )  
            {
                Time t = (*it)->next_start_time();
                if( t <= now )  (*it)->process();
                if( next_time > t )  next_time = t;
            }

            if( next_time > Time::now() )
            {
                sos_sleep( next_time - Time::now() );
            }
        }
#       endif


        _event.reset();

        if( ctrl_c_pressed )  _state_cmd = sc_terminate;
    }
}

//-------------------------------------------------------------------------Spooler::cmd_load_config
// Anderer Thread

void Spooler::cmd_load_config( const xml::Element_ptr& config, const Time& xml_mod_time, const string& source_filename )  
{ 
    THREAD_LOCK( _lock )  
    {
        _config_document_to_load = config.ownerDocument(); 
        _config_element_to_load  = config;
        _config_element_mod_time = xml_mod_time;
        _config_source_filename  = source_filename;
        _state_cmd = sc_load_config; 
    }

    if( current_thread_id() != _thread_id )  signal( "load_config" ); 
}

//----------------------------------------------------------------------------Spooler::cmd_continue
// Anderer Thread

void Spooler::cmd_continue()
{ 
    if( _state == s_paused )  _state_cmd = sc_continue; 
    signal( "continue" ); 
}

//------------------------------------------------------------------------------Spooler::cmd_reload
// Anderer Thread

void Spooler::cmd_reload()
{
    _state_cmd = sc_reload;
    signal( "reload" );
}

//--------------------------------------------------------------------------------Spooler::cmd_stop
// Anderer Thread

void Spooler::cmd_stop()
{
    _state_cmd = sc_stop;
    signal( "stop" );
}

//---------------------------------------------------------------------------Spooler::cmd_terminate
// Anderer Thread

void Spooler::cmd_terminate()
{
    //_log.msg( "Spooler::cmd_terminate" );

    _state_cmd = sc_terminate;
    signal( "terminate" );
}

//---------------------------------------------------------------Spooler::cmd_terminate_and_restart
// Anderer Thread

void Spooler::cmd_terminate_and_restart()
{
    //_log.msg( "Spooler::cmd_terminate_and_restart" );

    //if( _is_service )  throw_xc( "SPOOLER-114" );

    _state_cmd = sc_terminate_and_restart;
    signal( "terminate_and_restart" );
}

//-------------------------------------------------------Spooler::cmd_let_run_terminate_and_restart
// Anderer Thread

void Spooler::cmd_let_run_terminate_and_restart()
{
    _state_cmd = sc_let_run_terminate_and_restart;
    signal( "let_run_terminate_and_restart" );
}

//----------------------------------------------------------------------------------Spooler::launch

int Spooler::launch( int argc, char** argv )
{
    int rc;

    if( !SOS_LICENCE( licence_spooler ) )  throw_xc( "SOS-1000", "Spooler" );

    _argc = argc;
    _argv = argv;

    tzset();

    _thread_id = current_thread_id();

#   ifdef Z_WINDOWS
        SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
#   endif

    spooler_is_running = true;

    _event.set_name( "Spooler" );
    _event.add_to( &_wait_handles );

    _communication.init();  // F�r Windows

    do
    {
        if( _state_cmd != sc_load_config )  load();

        THREAD_LOCK( _lock )  
        {
            if( _config_element_to_load == NULL )  throw_xc( "SPOOLER-116", _spooler_id );

            load_config( _config_element_to_load, _config_element_mod_time, _config_source_filename );
    
            _config_element_to_load = NULL;
            _config_document_to_load = NULL;
        }

        start();
        run();
        stop();

    } while( _state_cmd == sc_reload || _state_cmd == sc_load_config );


    _java_vm.close();

    _log.info( "Spooler ordentlich beendet." );

    rc = 0;

    spooler_is_running = false;
    return rc;
}

//------------------------------------------------------------------------------------start_process
#ifdef Z_WINDOWS

static void start_process( const string& command_line )
{
    LOG( "start_process(\"" << command_line << "\")\n" );

    PROCESS_INFORMATION process_info; 
    STARTUPINFO         startup_info; 
    BOOL                ok;
    Dynamic_area        my_command_line;
    
    my_command_line.assign( command_line.c_str(), command_line.length() + 1 );

    memset( &process_info, 0, sizeof process_info );

    memset( &startup_info, 0, sizeof startup_info );
    startup_info.cb = sizeof startup_info; 

    ok = CreateProcess( NULL,                       // application name
                        my_command_line.char_ptr(), // command line 
                        NULL,                       // process security attributes 
                        NULL,                       // primary thread security attributes 
                        FALSE,                      // handles are inherited?
                        0,                          // creation flags 
                        NULL,                       // use parent's environment 
                        NULL,                       // use parent's current directory 
                        &startup_info,              // STARTUPINFO pointer 
                        &process_info );            // receives PROCESS_INFORMATION 

    if( !ok )  throw_mswin_error("CreateProcess");

    CloseHandle( process_info.hThread );
    CloseHandle( process_info.hProcess );
}

#endif
//----------------------------------------------------------------------------make_new_spooler_path
#ifdef Z_WINDOWS

static string make_new_spooler_path( const string& this_spooler )
{
    return directory_of_path(this_spooler) + DIR_SEP + basename_of_path( this_spooler ) + new_suffix + ".exe";
}

#endif
//----------------------------------------------------------------------------------spooler_restart

void spooler_restart( Log* log, bool is_service )
{
#ifdef Z_WINDOWS
    string this_spooler = program_filename();
    string command_line = GetCommandLine();
    string new_spooler  = make_new_spooler_path( this_spooler );

    if( GetFileAttributes( new_spooler.c_str() ) != -1 )      // spooler~new.exe vorhanden?
    {
        // Programmdateinamen aus command_line ersetzen
        int pos;
        if( command_line.length() == 0 )  throw_xc( "SPOOLER-COMMANDLINE" );
        if( command_line[0] == '"' ) {
            pos = command_line.find( '"', 1 );  if( pos == string::npos )  throw_xc( "SPOOLER-COMMANDLINE" );
            pos++;                
        } else {
            pos = command_line.find( ' ' );  if( pos == string::npos )  throw_xc( "SPOOLER-COMMANDLINE" );
        }

        command_line = new_spooler + command_line.substr(pos);
                     //+ " -renew-spooler=" + quoted_string(this_spooler);
    }
    else
    {
        //command_line += " -renew-spooler";
    }

    if( is_service )  command_line += " -renew-service";

    command_line += " -renew-spooler=" + quoted_string(this_spooler,'"','"');
    if( log )  log->info( "Restart Spooler  " + command_line );
    start_process( command_line );
#endif
}

//------------------------------------------------------------------------------------spooler_renew
#ifdef Z_WINDOWS

static void spooler_renew( const string& service_name, const string& renew_spooler, bool is_service, const string& command_line )
{
    string this_spooler = program_filename();
    BOOL   copy_ok      = true;
    double t            = renew_wait_time;

    if( is_service ) 
    {
        // terminate_and_restart: Erst SERVICE_STOP_PENDING, dann SERVICE_STOPPED
        // abort_immediately_and_restart: SERVICE_RUNNING, vielleicht SERVICE_PAUSED o.a.

        for( t; t > 0; t -= renew_wait_interval )
        {
            if( spooler::service_state(service_name) == SERVICE_STOPPED )  break;    
            sos_sleep( renew_wait_interval );
        }

        if( spooler::service_state(service_name) != SERVICE_STOPPED )  return;
    }

    if( renew_spooler != this_spooler )
    {
        for( t; t > 0; t -= renew_wait_interval )  
        {
            string msg = "CopyFile " + this_spooler + ", " + renew_spooler + '\n';
            if( !is_service )  fprintf( stderr, "%s", msg.c_str() );  // stderr, weil wir kein Log haben.
            LOG( msg );

            copy_ok = CopyFile( this_spooler.c_str(), renew_spooler.c_str(), FALSE );
            if( copy_ok )  break;

            int error = GetLastError();
            try 
            { 
                throw_mswin_error( error, "CopyFile" ); 
            }
            catch( const Xc& x ) { 
                if( !is_service )  fprintf( stderr, "%s\n", x.what() );
                LOG( x.what() << '\n' );
            }

            if( error != ERROR_SHARING_VIOLATION )  return;
            sos_sleep( renew_wait_interval );
        }

        if( !is_service )  fprintf( stderr, "Der Spooler ist ausgetauscht und wird neu gestartet\n\n" );
    }

    if( is_service )  spooler::service_start( service_name );
                else  start_process( quoted_string(renew_spooler,'"','"') + " " + command_line );
}

#endif
//----------------------------------------------------------------------------------------full_path
#ifdef Z_WINDOWS

static string full_path( const string& path )
{
    Sos_limited_text<MAX_PATH> full;

    char* p = _fullpath( full.char_ptr(), path.c_str(), full.size() );
    if( !p )  throw_xc( "fullpath", path );

    return full.char_ptr();
}

#endif
//-------------------------------------------------------------------------------delete_new_spooler
#ifdef Z_WINDOWS

void __cdecl delete_new_spooler( void* )
{
    string          this_spooler   = program_filename();
    string          basename       = basename_of_path( this_spooler );
    string          copied_spooler = make_new_spooler_path( this_spooler );
    struct _stat    ths;
    struct _stat    cop;
    int             err;

    if( full_path( this_spooler ) == full_path( copied_spooler ) )  return;

    err = _stat( this_spooler  .c_str(), &ths );  if(err)  return;
    err = _stat( copied_spooler.c_str(), &cop );  if(err)  return;

    if( ths.st_size == cop.st_size  &&  ths.st_mtime == cop.st_mtime )  // spooler.exe == spooler~new.exe?
    {
        for( double t = renew_wait_time; t > 0; t -= renew_wait_interval )  
        {
            string msg = "remove " + copied_spooler + '\n';
            fprintf( stderr, "%s", msg.c_str() );
            LOG( msg );

            int ret = _unlink( copied_spooler.c_str() );
            if( ret == 0  || errno != EACCES ) break;

            msg = "errno=" + as_string(errno) + ' ' + strerror(errno) + '\n';
            fprintf( stderr, "%s", msg.c_str() );
            LOG( msg.c_str() );
            
            sos_sleep( renew_wait_interval );
        }
    }
}

#endif
//-------------------------------------------------------------------------------------spooler_main

int spooler_main( int argc, char** argv )
{
    int ret;

    try
    {
        Ole_initialize ole;

        Spooler my_spooler;

        spooler = &my_spooler;

        ret = my_spooler.launch( argc, argv );

        spooler = NULL;
    }
    catch( const Xc& x )
    {
        SHOW_ERR( "Fehler " << x );
        ret = 9999;
    }

    return ret;
}

//-------------------------------------------------------------------------------------------------

} //namespace spooler

//-----------------------------------------------------------------------------------------sos_main

int sos_main( int argc, char** argv )
{
    int ret = 99;

    bool    is_service = false;
    bool    is_service_set = false;
    bool    do_install_service = false;
    bool    do_remove_service = false;
    string  id;
    string  service_name, service_display;
    string  service_description = "Hintergrund-Jobs der Document Factory";
    string  renew_spooler;
    string  command_line;
    bool    renew_service = false;
    string  log_filename;
    string  factory_ini = spooler::default_factory_ini;
    string  dependencies;

    for( Sos_option_iterator opt ( argc, argv ); !opt.end(); opt.next() )
    {
      //if( opt.flag      ( "renew-spooler"    ) )  renew_spooler = program_filename();
      //else
        if( opt.with_value( "renew-spooler"    ) )  renew_spooler = opt.value();
        else
        if( opt.flag      ( "renew-service"    ) )  renew_service = opt.set();
        else
        if( opt.flag      ( "V"                ) )  fprintf( stderr, "Spooler %s\n", VER_PRODUCTVERSION_STR );
        else
        {
            if( opt.flag      ( "install-service"  ) )  do_install_service = opt.set();
            else
            if( opt.with_value( "install-service"  ) )  do_install_service = true, service_name = opt.value();
            else
            if( opt.flag      ( "remove-service"   ) )  do_remove_service = opt.set();
            else
            if( opt.with_value( "service-name"     ) )  service_name = opt.value();
            else
            if( opt.with_value( "service-display"  ) )  service_display = opt.value();
            else
            if( opt.with_value( "service-descr"    ) )  service_description = opt.value();
            else
            if( opt.flag      ( "service"          ) )  is_service = opt.set(), is_service_set = true;
            else
            if( opt.with_value( "service"          ) )  is_service = true, is_service_set = true, service_name = opt.value();
            else
            if( opt.with_value( "need-service"     ) )  dependencies += opt.value(), dependencies += '\0';
            else
            {
                if( opt.with_value( "id"               ) )  id = opt.value();
                else
                if( opt.with_value( "ini"              ) )  factory_ini = opt.value();
                else
                if( opt.with_value( "log"              ) )  log_filename = opt.value();

                if( !command_line.empty() )  command_line += " ";
                command_line += opt.complete_parameter( '"', '"' );
            }
        }
    }

#   ifdef Z_WINDOWS
        if( service_name != "" ) 
        {
            if( service_display == "" )  service_display = service_name;
        }
        else
        {
            service_name = spooler::make_service_name(id);
            if( service_display == "" )  service_display = spooler::make_service_display(id);
        }
#   endif

    if( log_filename.empty() )  log_filename = read_profile_string( factory_ini, "spooler", "log" );
    if( !log_filename.empty() )  log_start( log_filename );

#   ifdef Z_WINDOWS

        if( !renew_spooler.empty() )  
        { 
            spooler::spooler_renew( service_name, renew_spooler, renew_service, command_line ); 
            ret = 0;
        }
        else
        if( do_remove_service | do_install_service )
        {
            if( do_remove_service  )  spooler::remove_service( service_name );
            if( do_install_service ) 
            {
                //if( !is_service )  command_line = "-service " + command_line;
                command_line = "-service=" + service_name + " " + command_line;
                dependencies += '\0';
                spooler::install_service( service_name, service_display, service_description, dependencies, command_line );
            }
            ret = 0;
        }
        else
        {
            _beginthread( spooler::delete_new_spooler, 50000, NULL );

          //if( !is_service_set )  is_service = spooler::service_is_started(service_name);

            if( is_service )
            {
                ret = spooler::spooler_service( service_name, argc, argv );
            }
            else
            {
                ret = spooler::spooler_main( argc, argv );
            }
        }

#    else

        ret = spooler::spooler_main( argc, argv );

#   endif

    return ret;
}

} //namespace sos

