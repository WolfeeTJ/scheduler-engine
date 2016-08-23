// $Id: scheduler_object.cxx 14131 2010-11-09 20:22:22Z jz $        Joacim Zschimmer, Zschimmer GmbH, http://www.zschimmer.com


#include "spooler.h"

namespace sos {
namespace scheduler {

//--------------------------------------------------------------Scheduler_object::name_of_type_code
    
string Scheduler_object::name_of_type_code( Scheduler_object::Type_code type_code )
{
    switch( type_code )
    {
        case type_none:                         return "none";

        case type_active_schedulers_watchdog:   return "Active_schedulers_watchdog";
        case type_cluster_member:               return "Cluster_member";
        case type_database:                     return "Database";
        case type_database_order_detector:      return "Database_order_detector";
        case type_directory_file_order_source:  return "Directory_file_order_source";
        case type_directory_observer:           return "Directory_observer";
        case type_directory_tree:               return "Directory_tree";
        case type_event_subsystem:              return "Event_subsystem";
        case type_exclusive_scheduler_watchdog: return "Exclusive_scheduler_watchdog";
        case type_folder:                       return "Folder";
        case type_subfolder_folder:             return "Subfolder_folder";
        case type_folder_subsystem:             return "Folder_subsystem";
        case type_heart_beat:                   return "Heart_beat";
        case type_heart_beat_watchdog_thread:   return "Heart_beat_watchdog_thread";
        case type_http_server:                  return "Http_server";
        case type_http_file_directory:          return "Http_file_directory";
        case type_java_subsystem:               return "Java_subsystem";
        case type_job:                          return "Job";
        case type_job_chain:                    return "Job_chain";
        case type_job_chain_folder:             return "Job_chain_folder";
        case type_job_chain_group:              return "Job_chain_group";
        case type_job_chain_node:               return "job_chain::Node";
        case type_job_folder:                   return "Job_folder";
        case type_job_subsystem:                return "Job_subsystem";
        case type_internal_module:              return "Internal_module";
        case type_lock:                         return "Lock";
        case type_lock_folder:                  return "Lock_folder";
        case type_lock_holder:                  return "Lock.Holder";
        case type_lock_requestor:               return "Lock.Use";
        case type_lock_subsystem:               return "Lock_subsystem";
        case type_lock_use:                     return "Use";
        case type_monitor:                      return "Monitor";
        case type_monitor_folder:               return "Monitor_folder";
        case type_monitor_subsystem:            return "Monitor_subsystem";
        case type_order:                        return "Order";
        case type_order_queue:                  return "Order_queue";
        case type_order_subsystem:              return "Job_chain_subsystem";
        case type_process:                      return "Process";
        case type_process_class:                return "Process_class";
        case type_process_class_folder:         return "Process_class_folder";
        case type_process_class_subsystem:      return "Process_class_subsystem";
        case type_remote_configuration_observer:return "Remote_configuration_observer";
        case type_remote_scheduler:             return "Remote_scheduler";
        case type_remote_schedulers_configuration: return "Remote_schedulers_configuration";
        case type_schedule:                     return "Schedule";
        case type_schedule_folder:              return "Schedule_folder";
        case type_schedule_subsystem:           return "Schedule_subsystem";
        case type_schedule_use:                 return "Schedule_use";
        case type_scheduler_event_manager:      return "Scheduler_event_manager";
        case type_scheduler_event2:             return "Scheduler_event2";
        case type_scheduler_script:             return "Scheduler_script";
        case type_scheduler_script_folder:      return "Scheduler_script_folder";
        case type_scheduler_script_subsystem:   return "Scheduler_script_subsystem";
        case type_scheduler:                    return "Scheduler";
        case type_standing_order:               return "Standing_order";
        case type_standing_order_folder:        return "Standing_order_folder";
        case type_standing_order_subsystem:     return "Standing_order_subsystem";
        case type_subsystem_register:           return "Subsystem_register";
        case type_supervisor:                   return "Supervisor";
        case type_supervisor_client:            return "Supervisor_client";
        case type_supervisor_client_connection: return "Supervisor_client_connection";
        case type_task:                         return "Task";
        case type_task_subsystem:               return "Task_subsystem";
        case type_web_service:                  return "Web_service";
        case type_web_service_operation:        return "Web_service_operation";
        case type_web_service_request:          return "Web_service_request";
        case type_web_service_response:         return "Web_service_response";
        case type_web_services:                 return "Web_services";
        case type_xml_client_connection:        return "Xml_client_connection";
        default:                                return S() << "Type_code(" << type_code << ")";
    }
}

//---------------------------------------------Abstract_scheduler_object::Abstract_scheduler_object

Abstract_scheduler_object::Abstract_scheduler_object( Spooler* spooler, IUnknown* me, Type_code code )
: 
    _spooler(spooler), 
    _my_iunknown(me), 
    _scheduler_object_type_code(code)
{
    if( this == spooler )
    {
        _log = Z_NEW( Prefix_log( 1 ) );       // Der Scheduler initialisiert beim Start sein Hauptprotokoll selbst
    }
    else
    {
        _log = Z_NEW( Prefix_log( this ) );
        _log->set_prefix( obj_name() );
    }
}

//--------------------------------------------------------------Scheduler_object::~Scheduler_object
    
Scheduler_object::~Scheduler_object()
{
}

//-------------------------------------------------------------------Scheduler_object::report_event

void Scheduler_object::report_event(const KeyedEventJ& e)
{
    if (Spooler* sp = spooler())
        if (Event_subsystem* s = sp->event_subsystem()) 
            s->report(e);
}

//-------------------------------------------------------------------Scheduler_object::report_event

void Scheduler_object::report_event(const KeyedEventJ& e, const ObjectJ& eventSource) {
    if (Spooler* sp = spooler())
        if (Event_subsystem* s = sp->event_subsystem()) 
            s->report(e, eventSource);
}

//-------------------------------------------------------------------Scheduler_object::report_event

void Scheduler_object::report_event_code(Event_code event_code, const ObjectJ& eventSource) {
    if (Spooler* sp = spooler())
        if (Event_subsystem* s = sp->event_subsystem()) 
            s->report_event_code(event_code, eventSource);
}

//----------------------------------------------Abstract_scheduler_object::write_element_attributes

void Abstract_scheduler_object::write_element_attributes( const xml::Element_ptr& ) const
{
    Z_DEBUG_ONLY( assert( !"Scheduler_object::write_element_attributes" ) );
}

//------------------------------------------------Abstract_scheduler_object::assert_empty_attribute

void Abstract_scheduler_object::assert_empty_attribute( const xml::Element_ptr& element, const string& attribute_name )
{
    if( element.getAttribute( attribute_name ) != "" )
    {
        z::throw_xc( "SCHEDULER-232", element.nodeName(), attribute_name, element.getAttribute( attribute_name ) );
    }
}

//--------------------------------------------------Abstract_scheduler_object::mail_xslt_stylesheet

ptr<Xslt_stylesheet> Abstract_scheduler_object::mail_xslt_stylesheet()
{ 
    if( !_mail_xslt_stylesheet )
    {
        if( _mail_xslt_stylesheet_path != "" )
        {
            ptr<Xslt_stylesheet> stylesheet = Z_NEW( Xslt_stylesheet );
            stylesheet->load_file( _mail_xslt_stylesheet_path );
            
            _mail_xslt_stylesheet = stylesheet;
        }
        else
        if( this != _spooler )
        {
            _mail_xslt_stylesheet = _spooler->mail_xslt_stylesheet();
        }
    }

    return _mail_xslt_stylesheet; 
}

//--------------------------------------------------------------------Abstract_scheduler_object::db

Database* Abstract_scheduler_object::db() const 
{
    return _spooler->_db;
}

//---------------------------------------------------------Abstract_scheduler_object::job_subsystem

Job_subsystem* Abstract_scheduler_object::job_subsystem() const
{
    return _spooler->job_subsystem();
}

//--------------------------------------------------------Abstract_scheduler_object::task_subsystem

Task_subsystem* Abstract_scheduler_object::task_subsystem() const
{
    return _spooler->task_subsystem();
}

//-------------------------------------------------------Abstract_scheduler_object::order_subsystem

Order_subsystem* Abstract_scheduler_object::order_subsystem() const
{
    return _spooler->order_subsystem();
}

//-------------------------------------------------------------------------------------------------

} //namespace spoooler
} //namespace sos
