// $Id: java_subsystem.h 14094 2010-10-20 11:28:15Z jz $        Joacim Zschimmer, Zschimmer GmbH, http://www.zschimmer.com

#ifndef __SCHEDULER_JAVA_SUBSYSTEM_H
#define __SCHEDULER_JAVA_SUBSYSTEM_H

#include "javaproxy.h"

namespace sos {
namespace scheduler {

//-----------------------------------------------------------------------Java_subsystem_interface

struct Java_subsystem_interface : Object, Subsystem
{
                                Java_subsystem_interface    ( Scheduler* scheduler, Type_code t )   : Subsystem( scheduler, this, t ) {}

    virtual void                initialize_java_sister      ()                                      = 0;
    virtual javabridge::Vm*     java_vm                     ()                                      = 0;
    virtual InjectorJ&          injectorJ                   ()                                      = 0;
    virtual SchedulerJ&         schedulerJ                  ()                                      = 0;
  //virtual const PlatformJ&    platformJ                   () const                                = 0;
    virtual xml::Element_ptr    dom_element                 (const xml::Document_ptr&)              = 0;
    virtual void                on_scheduler_activated      ()                                      = 0;

    static string               classname_of_scheduler_object(const string&);
    static ptr<javabridge::Java_idispatch>  instance_of_scheduler_object( IDispatch*, const string&);
};

//-------------------------------------------------------------------------------------------------

ptr<Java_subsystem_interface>   new_java_subsystem          (Scheduler*);

//-------------------------------------------------------------------------------------------------

} //namespace scheduler
} //namespace sos

#endif
