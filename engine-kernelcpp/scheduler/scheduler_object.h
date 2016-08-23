// $Id: scheduler_object.h 14167 2010-12-30 15:11:58Z jz $        Joacim Zschimmer, Zschimmer GmbH, http://www.zschimmer.com

#ifndef __SCHEDULER_OBJECT_H
#define __SCHEDULER_OBJECT_H

#include "Event_code.h"

#include "../javaproxy/com__sos__scheduler__engine__data__event__KeyedEvent.h"
typedef javaproxy::com::sos::scheduler::engine::data::event::KeyedEvent KeyedEventJ;

namespace sos {
namespace scheduler {

//---------------------------------------------------------------------------------Scheduler_object

struct Scheduler_object

{
    enum Type_code
    {
        type_none,

        type_active_schedulers_watchdog,
        type_cluster_member,
        type_database,
        type_database_order_detector,
        type_directory_file_order_source,
        type_directory_observer,
        type_directory_tree,
        type_exclusive_scheduler_watchdog,
        type_folder_subsystem,
        type_folder,
        type_subfolder_folder,
        type_heart_beat,
        type_heart_beat_watchdog_thread,
        type_http_file_directory,
        type_http_server,
        type_java_subsystem,
        type_job,
        type_job_chain,
        type_job_chain_folder,
        type_job_chain_group,
        type_job_chain_node,
        type_job_folder,
        type_job_subsystem,
        type_internal_module,
        type_lock,
        type_lock_folder,
        type_lock_holder,
        type_lock_requestor,
        type_lock_subsystem,
        type_lock_use,
        type_monitor,
        type_monitor_folder,
        type_monitor_subsystem,
        type_web_service,
        type_web_service_operation,
        type_web_service_request,
        type_web_service_response,
        type_web_services,
        type_order,
        type_order_queue,
        type_order_subsystem,
        type_remote_configuration_observer,
        type_remote_scheduler,
        type_remote_schedulers_configuration,
        type_process,
        type_process_class,
        type_process_class_folder,
        type_process_class_subsystem,
        type_schedule,
        type_schedule_folder,
        type_schedule_subsystem,
        type_schedule_use,
        type_scheduler,
        type_scheduler_event_manager,
        type_scheduler_script,
        type_scheduler_script_folder,
        type_scheduler_script_subsystem,
        type_standing_order,
        type_standing_order_folder,
        type_standing_order_subsystem,
        type_subsystem_register,
        type_supervisor,
        type_supervisor_client,
        type_supervisor_client_connection,
        type_task,
        type_task_subsystem,
        type_xml_client_connection,
        type_scheduler_event2,
        type_event_subsystem,
    };

    static string               name_of_type_code           ( Type_code );

    virtual                    ~Scheduler_object            ();

    virtual Type_code           scheduler_type_code         () const = 0;
    virtual Spooler*            spooler                     () const = 0;
    virtual Prefix_log*         log                         () const = 0;

    virtual void                close                       ()                                      {}
    void                        report_event                (const KeyedEventJ&);
    void                        report_event                (const KeyedEventJ&, const ObjectJ& eventSource);
    void                        report_event_code           (Event_code event_code, const ObjectJ& eventSource);
    virtual string              obj_name                    () const                                { return name_of_type_code(scheduler_type_code()); }
};


struct Abstract_scheduler_object : Scheduler_object {
    Abstract_scheduler_object(Spooler*, IUnknown* me, Type_code);

    Type_code                   scheduler_type_code         () const                                { return _scheduler_object_type_code; }
    void                    set_mail_xslt_stylesheet_path   ( const string& path )                  { _mail_xslt_stylesheet.release();  _mail_xslt_stylesheet_path = path; }
    Spooler*                    spooler                     () const                                { return _spooler; }
    void                        assert_empty_attribute      ( const xml::Element_ptr&, const string& attribute_name );
    virtual void                self_check                  ()                                      {}
    virtual ptr<Xslt_stylesheet> mail_xslt_stylesheet       ();
    virtual void                print_xml_child_elements_for_event( String_stream*, Scheduler_event* )  {}
    virtual void                write_element_attributes    ( const xml::Element_ptr& ) const;
    Prefix_log*                 log                         () const                                { return _log; }
    Database*                   db                          () const;
    Job_subsystem*              job_subsystem               () const;
    Task_subsystem*             task_subsystem              () const;
    Order_subsystem*            order_subsystem             () const;

    Spooler* const             _spooler;
    IUnknown* const            _my_iunknown;
    Type_code const            _scheduler_object_type_code;
    ptr<Xslt_stylesheet>       _mail_xslt_stylesheet;
    string                     _mail_xslt_stylesheet_path;
    ptr<Prefix_log>            _log;
};

//-------------------------------------------------------------------------------------------------

} //namespace scheduler
} //namespace sos

#endif
