// $Id: spooler_module_internal.h 13328 2008-01-28 10:12:01Z jz $

#ifndef __SPOOLER_MODULE_INTERNAL_H
#define __SPOOLER_MODULE_INTERNAL_H

namespace sos {
namespace scheduler {

//----------------------------------------------------------------------------------Internal_module

struct Internal_module : Module
{
                                Internal_module             ( Spooler*, Prefix_log* = NULL );
    virtual ptr<Module_instance> create_instance_impl       (const Host_and_port& remote_scheduler) = 0;
};

//-------------------------------------------------------------------------Internal_module_instance

struct Internal_module_instance : Module_instance
{
                                Internal_module_instance    ( Module* );

  //void                        init                        ();
    bool                        load                        ()                                      { _loaded = true; return Module_instance::load(); }
  //void                        start                       ();
    virtual void                add_obj                     ( IDispatch*, const string& name );
  //void                        close__end                  ();
    Variant                     call                        ( const string& name );
    Variant                     call                        ( const string& name, const Variant& param, const Variant& );      
    bool                        name_exists                 ( const string& name );
    bool                        loaded                      ()                                      { return _loaded; }
    bool                        callable                    ()                                      { return true; }

    virtual bool                spooler_process             () = 0;

    Fill_zero                  _zero_;
    Task*                      _task;
    Has_log*                   _log;
    bool                       _loaded;
};

//-------------------------------------------------------------------------------------------------

} //namespace scheduler
} //namespace sos

#endif
