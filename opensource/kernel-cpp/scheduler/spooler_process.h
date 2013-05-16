// $Id: spooler_process.h 13558 2008-05-09 14:11:16Z jz $

#ifndef __SPOOLER_PROCESS_H
#define __SPOOLER_PROCESS_H

#include "../zschimmer/com_remote.h"


namespace sos {
namespace scheduler {

//-------------------------------------------------------------------------------------------------

struct Process_class;
struct Process_class_folder;
struct Process_class_subsystem;

//------------------------------------------------------------------------------------------Process
// Ein Prozess, in dem ein Module oder eine Task ablaufen kann.
// Kann auch ein Thread sein.

struct Process : zschimmer::Object, Scheduler_object
{
    struct Close_operation : Async_operation
    {
        enum State { s_initial, s_closing_session, s_closing_remote_process, s_finished };


                                    Close_operation         ( Process*, bool run_independently );
                                   ~Close_operation         ();

        // Async_operation:
        bool                        async_continue_         ( Continue_flags );
        bool                        async_finished_         () const;
        string                      async_state_text_       () const;

        static string               string_from_state       ( State );

      private:
        friend struct               Process;

        Fill_zero                  _zero_;
        State                      _state;
        ptr<Process>               _process;
        Async_operation*           _close_session_operation;
        ptr<Close_operation>       _hold_self;              // Objekt h�lt sich selbst, wenn es selbstst�ndig, ohne Antwort, den Process schlie�en soll
    };


    struct Async_remote_operation : Async_operation
    {
        enum State
        {
            s_not_connected,
            s_connecting,
            s_starting,
            s_running,
            s_closing,
            s_closed
        };

        static string           state_name                  ( State );


                                Async_remote_operation      ( Process* );
                               ~Async_remote_operation      ();

        virtual bool            async_continue_             ( Continue_flags f )                    { return _process->async_remote_start_continue( f ); }
        virtual bool            async_finished_             () const                                { return _state == s_running  ||  _state == s_closed; }
        virtual string          async_state_text_           () const;

        void                    close_remote_task           ( bool kill = false );


        Fill_zero              _zero_;
        State                  _state;
        Process*               _process;
    };


    struct Com_server_thread : object_server::Connection_to_own_server_thread::Server_thread
    {
        typedef object_server::Connection_to_own_server_thread::Server_thread Base_class;

                                Com_server_thread           ( object_server::Connection_to_own_server_thread* );

        int                     thread_main                 ();

        Fill_zero              _zero_;
        ptr<Object_server>     _object_server;
    };



                                Process                     ( Spooler* );
    Z_GNU_ONLY(                 Process                     (); )
                               ~Process                     ();


    void                        close_async                 ();
    Async_operation*            close__start                ( bool run_independently = false );
    void                        close__end                  ();
    bool                     is_closing                     ()                                      { return _close_operation != NULL; }
    bool                        continue_close_operation    ( Process::Close_operation* );


    bool                        started                     ()                                      { return _connection != NULL; }

    void                    set_controller_address          ( const Host_and_port& h )              { _controller_address = h; }
    void                        start                       ();
    void                        start_local_process         ();
    void                        start_local_thread          ();
    void                        fill_connection             ( object_server::Connection* );
    void                        async_remote_start          ();
  //bool                        is_started                  ();
    bool                        async_remote_start_continue ( Async_operation::Continue_flags );
    object_server::Session*     session                     ()                                      { return _session; }
  //void                    set_event                       ( Event* e )                            { if( _connection )  _connection->set_event( e ); }
    bool                        async_continue              ();
    double                      async_next_gmtime           ()                                      { return _connection? _connection->async_next_gmtime() : time::never_double; }
    void                        add_module_instance         ( Module_instance* );
    void                        remove_module_instance      ( Module_instance* );
    int                         module_instance_count       ()                                      { return _module_instance_count; }
    void                    set_temporary                   ( bool t )                              { _temporary = t; }
    void                    set_job_name                    ( const string& job_name )              { _job_name = job_name; }
    void                    set_task_id                     ( int id )                              { _task_id = id; }
  //void                    set_server                      ( const string& hostname, int port )    { _server_hostname = hostname;  _server_port = port; }
    void                    set_priority                    ( const string& priority )              { _priority = priority; }
    void                    set_environment                 ( const Com_variable_set& env )         { _environment = new Com_variable_set( env ); }
  //void                    set_environment_string          ( const string& env )                   { _environment_string = env;  _has_environment = true; }
    void                    set_java_options                (const string& o)                       { _java_options = o; }
    void                    set_java_classpath              (const string& o)                       { _java_classpath = o; }
    void                    set_run_in_thread               ( bool b )                              { _run_in_thread = b; }
    void                    set_log_stdout_and_stderr       ( bool b )                              { _log_stdout_and_stderr = b; }
    void                    set_login                       (Login* o)                              { _login = o; }
    Process_id                  process_id                  () const                                { return _process_id; }
    int                         pid                         () const;                               // Bei kind_process die PID des eigentlichen Prozesses, �ber Connection_to_own_server_thread
    Process_id                  remote_process_id           () const                                { return _remote_process_id; }
    bool                     is_terminated                  ();
    void                        end_task                    ();
    bool                        kill                        ();
    int                         exit_code                   ();
    int                         termination_signal          ();
    File_path                   stderr_path                 ();
    File_path                   stdout_path                 ();
    bool                        try_delete_files            ( Has_log* );
    std::list<file::File_path>  undeleted_files             ();
    bool                        connected                   ()                                      { return _connection? _connection->connected() : false; }
    bool                        is_remote_host              () const;

  //void                    set_dom                         ( const xml::Element_ptr& );
    xml::Element_ptr            dom_element                 ( const xml::Document_ptr&, const Show_what& );
    string                      obj_name                    () const;
    string                      short_name                  () const;

    
//private:
    Fill_zero                  _zero_;
    string                     _job_name;
    int                        _task_id;
  //string                     _server_hostname;
  //int                        _server_port;
    Host_and_port              _controller_address;
    ptr<object_server::Connection> _connection;             // Verbindung zum Prozess
    ptr<object_server::Session>    _session;                // Wir haben immer nur eine Session pro Verbindung
    ptr<Com_server_thread>     _com_server_thread;
    Process_handle             _process_handle_copy;
    bool                       _is_killed;
    int                        _exit_code;
    int                        _termination_signal;
    Time                       _running_since;
    bool                       _temporary;                  // L�schen, wenn kein Module_instance mehr l�uft
    long32                     _module_instance_count;
    Module_instance*           _module_instance;
    ptr<Login>                 _login;
    Process_class*             _process_class;
    string                     _priority;
    ptr<Com_variable_set>      _environment;
    string                     _java_options;
    string                     _java_classpath;
    bool                       _run_in_thread;
    Host_and_port              _remote_scheduler;
    Process_id                 _remote_process_id;
    pid_t                      _remote_pid;
    //File                       _remote_stdout_file;
    //File                       _remote_stderr_file;
    ptr<Async_remote_operation> _async_remote_operation;
    ptr<Xml_client_connection>  _xml_client_connection;
    ptr<Close_operation>       _close_operation;
    const Process_id           _process_id;
    bool                       _log_stdout_and_stderr;      // Prozess oder Thread soll stdout und stderr selbst �ber COM/TCP protokollieren
};

//----------------------------------------------------------------------Process_class_configuration

struct Process_class_configuration : idispatch_implementation< Process_class, spooler_com::Iprocess_class >,
                                     file_based< Process_class_configuration, Process_class_folder, Process_class_subsystem >
{
                                Process_class_configuration ( Scheduler*, const string& name = "" );

    STDMETHODIMP_(ULONG)        AddRef                      ()                                      { return Idispatch_implementation::AddRef(); }
    STDMETHODIMP_(ULONG)        Release                     ()                                      { return Idispatch_implementation::Release(); }

    virtual void            set_max_processes               ( int );
    int                         max_processes               () const                                { return _max_processes; }
    virtual void          check_max_processes               ( int ) const                           {}

    virtual void            set_remote_scheduler            ( const Host_and_port& );
    const Host_and_port&        remote_scheduler            () const                                { return _remote_scheduler; }
    virtual void          check_remote_scheduler            ( const Host_and_port& ) const          {}

    bool                        is_remote_host              () const                                { return _remote_scheduler; }

    string                      obj_name                    () const;

    void                    set_dom                         ( const xml::Element_ptr& );
    xml::Element_ptr            dom_element                 ( const xml::Document_ptr&, const Show_what& );

    // spooler_com::Iprocess_class:
    STDMETHODIMP            get_Java_class_name             ( BSTR* result )                        { return String_to_bstr( const_java_class_name(), result ); }
    STDMETHODIMP_(char*)  const_java_class_name             ()                                      { return (char*)"sos.spooler.Process_class"; }
    STDMETHODIMP                Remove                      ();
    STDMETHODIMP            put_Name                        ( BSTR );
    STDMETHODIMP            get_Name                        ( BSTR* result )                        { return String_to_bstr( name(), result ); }
    STDMETHODIMP            put_Remote_scheduler            ( BSTR );
    STDMETHODIMP            get_Remote_scheduler            ( BSTR* result )                        { return String_to_bstr( _remote_scheduler.as_string(), result ); }
    STDMETHODIMP            put_Max_processes               ( int );
    STDMETHODIMP            get_Max_processes               ( int* result )                         { *result = _max_processes;  return S_OK; }

  protected: 
    Fill_zero                  _zero_;

    int                        _max_processes;
    Host_and_port              _remote_scheduler;
    // Neue Einstellungen in Process_class::set_configuration() ber�cksichtigen!


    static Class_descriptor     class_descriptor;
    static const Com_method    _methods[];
};

//------------------------------------------------------------------------------------Process_class
// <process_class>

struct Process_class : Process_class_configuration
{
                                Process_class               ( Scheduler*, const string& name = "" );
    Z_GNU_ONLY(                 Process_class               (); )
                               ~Process_class               ();


    // file_based<Process_class>
    void                        close                       ();
    bool                        on_initialize               ();
    bool                        on_load                     ();
    bool                        on_activate                 ();

    bool                        can_be_removed_now          ();

    void                        prepare_to_replace          ();
    bool                        can_be_replaced_now         ();
    Process_class*              on_replace_now              ();
    bool                        is_visible_requisite  () { return path() != ""; }   // default process_class will not shown

    void                    set_configuration               ( const Process_class_configuration& );
    void                  check_max_processes               ( int ) const;
    void                    set_max_processes               ( int );
    void                  check_remote_scheduler            ( const Host_and_port& ) const;

    void                        add_process                 ( Process* );
    void                        remove_process              ( Process* );

    Process*                    new_process                 ();
    Process*                    select_process_if_available ();                                     // Startet bei Bedarf. Bei _max_processes: return NULL
    bool                        process_available           ( Job* for_job );
    void                        enqueue_waiting_job         ( Job* );
    void                        remove_waiting_job          ( Job* );
    bool                        need_process                ();
    void                        notify_a_process_is_idle    ();

    xml::Element_ptr            dom_element                 ( const xml::Document_ptr&, const Show_what& );
  //xml::Element_ptr            execute_xml                 ( Command_processor*, const xml::Element_ptr&, const Show_what& );


  private:
    friend struct               Process_class_subsystem;

    Fill_zero                  _zero_;

    typedef list< ptr<Job> >    Job_list;
    Job_list                   _waiting_jobs;

  public:
    typedef stdext::hash_set< ptr<Process> >  Process_set;
    Process_set                _process_set;
    int                        _process_set_version;
};

//-----------------------------------------------------------------------------Process_class_folder

struct Process_class_folder : typed_folder<Process_class>
{
                                Process_class_folder        ( Folder* );
                               ~Process_class_folder        ();


    // Typed_folder:
    bool                        is_empty_name_allowed       () const                                { return true; }

  //void                        set_dom                     ( const xml::Element_ptr& );
    void                        add_process_class           ( Process_class* process_class )        { add_file_based( process_class ); }
    void                        remove_process_class        ( Process_class* process_class )        { remove_file_based( process_class ); }
    Process_class*              process_class               ( const string& name )                  { return file_based( name ); }
    Process_class*              process_class_or_null       ( const string& name )                  { return file_based_or_null( name ); }
  //xml::Element_ptr            execute_xml_process_class   ( Command_processor*, const xml::Element_ptr& );
  //xml::Element_ptr            dom_element                 ( const xml::Document_ptr&, const Show_what& );
    xml::Element_ptr            new_dom_element             ( const xml::Document_ptr& doc, const Show_what& ) { return doc.createElement( "process_classes" ); }
};

//--------------------------------------------------------------------------Process_class_subsystem

struct Process_class_subsystem : idispatch_implementation< Process_class_subsystem, spooler_com::Iprocess_classes>, 
                                 file_based_subsystem< Process_class >
{
                                Process_class_subsystem     ( Scheduler* );

    // Subsystem
    void                        close                       ();
    bool                        subsystem_initialize        ();
    bool                        subsystem_load              ();
    bool                        subsystem_activate          ();

    // file_based_subsystem< Process_class >
    string                      object_type_name            () const                                { return "Process_class"; }
    string                      filename_extension          () const                                { return ".process_class.xml"; }
    string                      xml_element_name            () const                                { return "process_class"; }
    string                      xml_elements_name           () const                                { return "process_classes"; }
  //string                      normalized_name             ( const string& name ) const            { return name; }
    ptr<Process_class>          new_file_based              (const string& source)                  { return Z_NEW( Process_class( spooler() ) ); }
    xml::Element_ptr            new_file_baseds_dom_element ( const xml::Document_ptr& doc, const Show_what& ) { return doc.createElement( "process_classes" ); }

    ptr<Process_class_folder>   new_process_class_folder    ( Folder* folder )                      { return Z_NEW( Process_class_folder( folder ) ); }
    Process_class*              process_class               ( const Absolute_path& path )           { return file_based( path ); }
    Process_class*              process_class_or_null       ( const Absolute_path& path )           { return file_based_or_null( path ); }
    Process*                    new_temporary_process       ();
    Process_class*              temporary_process_class     ();
    bool                        try_to_free_process         ( Job* for_job, Process_class*, const Time& now );
    bool                        async_continue              ();

  //xml::Element_ptr            execute_xml                 ( Command_processor*, const xml::Element_ptr&, const Show_what& );

    // spooler_com::Iprocess_classes
    STDMETHODIMP            get_Java_class_name             ( BSTR* result )                        { return String_to_bstr( const_java_class_name(), result ); }
    STDMETHODIMP_(char*)  const_java_class_name             ()                                      { return (char*)"sos.spooler.Process_classes"; }
    STDMETHODIMP            get_Process_class               ( BSTR, spooler_com::Iprocess_class** );
    STDMETHODIMP            get_Process_class_or_null       ( BSTR, spooler_com::Iprocess_class** );
    STDMETHODIMP                Create_process_class        ( spooler_com::Iprocess_class** );
    STDMETHODIMP                Add_process_class           ( spooler_com::Iprocess_class* );


  private:
    Fill_zero                  _zero_;

  public:
    static Class_descriptor     class_descriptor;
    static const Com_method     _methods[];
};

//-------------------------------------------------------------------------------------------------

} //namespace scheduler
} //namespace sos

#endif
