// $Id: spooler_module_remote.h 13372 2008-02-03 22:24:12Z jz $

#ifndef __SPOOLER_MODULE_REMOTE_H
#define __SPOOLER_MODULE_REMOTE_H

#include "../zschimmer/com_remote.h"

namespace sos {
namespace scheduler {

//---------------------------------------------------------------------Remote_module_instance_proxy

struct Remote_module_instance_proxy : Com_module_instance_base
{

    enum Call_state
    {
        c_none,

        c_begin,            // call__begin()
        c_connect,          //   Mit Server verbinden
        c_create_instance,  
        c_construct,        
        c_call_begin,       //   spooler_open() 

        c_release_begin,    // Wenn Module_monitor.spooler_task_before() false geliefert hat
        c_release,

        c_finished,
    };


    struct Operation : Async_operation
    {
                                Operation                   ( Remote_module_instance_proxy*, Call_state first_state );
                               ~Operation                   ();

        Async_operation*        begin__start                ();
        bool                    begin__end                  ();

        virtual bool            async_finished_             () const;
        virtual bool            async_continue_             ( Continue_flags flags )                    { return _proxy->continue_async_operation( this, flags ); }
        virtual string          async_state_text_           () const;

        string                  state_name                  () const;


        Fill_zero              _zero_;
        Remote_module_instance_proxy* _proxy;
        Call_state             _call_state;
        Multi_qi               _multi_qi;
        bool                   _bool_result;
    };



                                Remote_module_instance_proxy( Module* module, const Host_and_port& remote_scheduler) : Com_module_instance_base(module), _zero_(_end_) { _remote_scheduler = remote_scheduler; }
                               ~Remote_module_instance_proxy();

    void                        init                        ();
    bool                        load                        ();
    void                        close                       ();
    bool                        kill                        ();
    bool                        is_remote_host              () const                                { return !_remote_scheduler.is_empty(); }
  
    void                        add_obj                     ( IDispatch*, const string& name );
    bool                        name_exists                 ( const string& name );
    Variant                     call                        ( const string& name );

    bool                        try_to_get_process          (const Process_configuration*);
    void                        detach_process              ();

            Async_operation*    close__start                ();
            void                close__end                  ();

    virtual Async_operation*    begin__start                ();
    virtual bool                begin__end                  ();

    virtual Async_operation*    end__start                  ( bool success );
    virtual void                end__end                    ();

    virtual Async_operation*    step__start                 ();
    virtual Variant             step__end                   ();

    virtual Async_operation*    call__start                 ( const string& method );
    virtual Variant             call__end                   ();

    virtual Async_operation*    release__start              ();
    virtual void                release__end                ();

    bool                        continue_async_operation    ( Operation*, Async_operation::Continue_flags );
    void                        check_connection_error      ();
    int                         exit_code                   ();
    int                         termination_signal          ();
    File_path                   stdout_path                 ();
    File_path                   stderr_path                 ();
    bool                        try_delete_files            ( Has_log* );
    std::list<File_path>        undeleted_files             ();
    string                      process_name                () const;

    Fill_zero                  _zero_;

    ptr<object_server::Session> _session;
    ptr<object_server::Proxy>   _remote_instance;
    ptr<Async_operation>        _operation;
    bool                        _end_success;               // Für end__start()
    int                         _exit_code;
    int                         _termination_signal;

    Fill_end                   _end_;
};

//-------------------------------------------------------------------------------------------------

} //namespace scheduler
} //namespace sos

#endif
