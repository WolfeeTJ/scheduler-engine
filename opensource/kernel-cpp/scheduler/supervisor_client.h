// $Id: supervisor_client.h 13550 2008-05-06 09:42:05Z jz $

namespace sos {
namespace scheduler {
namespace supervisor {

//----------------------------------------------------------------------Supervisor_client_interface

struct Supervisor_client_interface: idispatch_implementation< Supervisor_client_interface, spooler_com::Isupervisor_client >,
                                    Subsystem
{
                                Supervisor_client_interface ( Scheduler*, Type_code, Class_descriptor* );

    virtual bool                is_ready                    () const                                = 0;
    virtual bool                connection_failed           () const                                = 0;
    virtual void                start_update_configuration  ()                                      = 0;
    virtual void                set_using_central_configuration()                                   = 0;
    virtual bool                is_using_central_configuration() const                              = 0;
    virtual Host_and_port       host_and_port               () const                                = 0;
  //virtual void                change_to_old_connector     ()                                      = 0;
};


ptr<Supervisor_client_interface> new_supervisor_client      ( Scheduler*, const Host_and_port& );

//-------------------------------------------------------------------------------------------------

} //namespace supervisor
} //namespace scheduler
} //namespace sos
