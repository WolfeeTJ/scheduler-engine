// *** Generated by com.sos.scheduler.engine.cplusplus.generator ***

#include "_precompiled.h"

#include "com__sos__scheduler__engine__kernel__cppproxy__DatabaseC.h"
#include "com__sos__scheduler__engine__kernel__cppproxy__Variable_setC.h"
#include "java__lang__Object.h"
#include "java__lang__String.h"

namespace javaproxy { namespace com { namespace sos { namespace scheduler { namespace engine { namespace kernel { namespace cppproxy { 

struct DatabaseC__class : ::zschimmer::javabridge::Class
{
    DatabaseC__class(const string& class_name);
   ~DatabaseC__class();


    static const ::zschimmer::javabridge::class_factory< DatabaseC__class > class_factory;
};

const ::zschimmer::javabridge::class_factory< DatabaseC__class > DatabaseC__class::class_factory ("com.sos.scheduler.engine.kernel.cppproxy.DatabaseC");

DatabaseC__class::DatabaseC__class(const string& class_name) :
    ::zschimmer::javabridge::Class(class_name)
{}

DatabaseC__class::~DatabaseC__class() {}




DatabaseC::DatabaseC(jobject jo) { if (jo) assign_(jo); }

DatabaseC::DatabaseC(const DatabaseC& o) { assign_(o.get_jobject()); }

#ifdef Z_HAS_MOVE_CONSTRUCTOR
    DatabaseC::DatabaseC(DatabaseC&& o) { set_jobject(o.get_jobject());  o.set_jobject(NULL); }
#endif

DatabaseC::~DatabaseC() { assign_(NULL); }





::zschimmer::javabridge::Class* DatabaseC::java_object_class_() const { return _class.get(); }

::zschimmer::javabridge::Class* DatabaseC::java_class_() { return DatabaseC__class::class_factory.clas(); }


void DatabaseC::Lazy_class::initialize() const {
    _value = DatabaseC__class::class_factory.clas();
}


}}}}}}}
