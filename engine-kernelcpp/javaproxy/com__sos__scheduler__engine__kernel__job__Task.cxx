// *** Generated by com.sos.scheduler.engine.cplusplus.generator ***

#include "_precompiled.h"

#include "com__sos__scheduler__engine__kernel__job__Task.h"
#include "com__sos__scheduler__engine__cplusplus__runtime__Sister.h"
#include "com__sos__scheduler__engine__kernel__job__UnmodifiableTask.h"
#include "java__lang__Object.h"
#include "java__lang__String.h"

namespace javaproxy { namespace com { namespace sos { namespace scheduler { namespace engine { namespace kernel { namespace job { 

struct Task__class : ::zschimmer::javabridge::Class
{
    Task__class(const string& class_name);
   ~Task__class();

    ::zschimmer::javabridge::Method const _webServiceAccessTokenString____method;

    static const ::zschimmer::javabridge::class_factory< Task__class > class_factory;
};

const ::zschimmer::javabridge::class_factory< Task__class > Task__class::class_factory ("com.sos.scheduler.engine.kernel.job.Task");

Task__class::Task__class(const string& class_name) :
    ::zschimmer::javabridge::Class(class_name)
    ,_webServiceAccessTokenString____method(this, "webServiceAccessTokenString", "()Ljava/lang/String;"){}

Task__class::~Task__class() {}




Task::Task(jobject jo) { if (jo) assign_(jo); }

Task::Task(const Task& o) { assign_(o.get_jobject()); }

#ifdef Z_HAS_MOVE_CONSTRUCTOR
    Task::Task(Task&& o) { set_jobject(o.get_jobject());  o.set_jobject(NULL); }
#endif

Task::~Task() { assign_(NULL); }




::javaproxy::java::lang::String Task::webServiceAccessTokenString() const {
    ::zschimmer::javabridge::raw_parameter_list<0> parameter_list;
    Task__class* cls = _class.get();
    ::javaproxy::java::lang::String result;
    result.steal_local_ref(cls->_webServiceAccessTokenString____method.jobject_call(get_jobject(), parameter_list));
    return result;
}


::zschimmer::javabridge::Class* Task::java_object_class_() const { return _class.get(); }

::zschimmer::javabridge::Class* Task::java_class_() { return Task__class::class_factory.clas(); }


void Task::Lazy_class::initialize() const {
    _value = Task__class::class_factory.clas();
}


}}}}}}}
