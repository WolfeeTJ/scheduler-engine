// *** Generated by com.sos.scheduler.engine.cplusplus.generator ***

#include "spooler.h"
#include "../zschimmer/java.h"
#include "../zschimmer/Has_proxy.h"
#include "../zschimmer/javaproxy.h"
#include "../zschimmer/lazy.h"

using namespace ::zschimmer;
using namespace ::zschimmer::javabridge;

namespace zschimmer { namespace javabridge { 

    template<> const class_factory<Proxy_class> has_proxy< ::sos::scheduler::order::Order_queue >::proxy_class_factory("com.sos.scheduler.engine.kernel.cppproxy.Order_queueCImpl");

}}

namespace javaproxy { namespace com { namespace sos { namespace scheduler { namespace engine { namespace kernel { namespace cppproxy { 

static void JNICALL close(JNIEnv* jenv, jobject, jlong cppReference)
{
    Env env = jenv;
    try {
        ::sos::scheduler::order::Order_queue* o_ = has_proxy< ::sos::scheduler::order::Order_queue >::of_cpp_reference(cppReference,"::sos::scheduler::order::Order_queue::close()");
        (o_->close());
    }
    catch(const exception& x) {
        env.set_java_exception(x);
    }
}

}}}}}}}

namespace javaproxy { namespace com { namespace sos { namespace scheduler { namespace engine { namespace kernel { namespace cppproxy { 

static jboolean JNICALL is_1distributed_1order_1requested__J(JNIEnv* jenv, jobject, jlong cppReference, jlong p0)
{
    Env env = jenv;
    try {
        ::sos::scheduler::order::Order_queue* o_ = has_proxy< ::sos::scheduler::order::Order_queue >::of_cpp_reference(cppReference,"::sos::scheduler::order::Order_queue::is_distributed_order_requested()");
        return (o_->is_distributed_order_requested(p0));
    }
    catch(const exception& x) {
        env.set_java_exception(x);
        return jboolean();
    }
}

}}}}}}}

namespace javaproxy { namespace com { namespace sos { namespace scheduler { namespace engine { namespace kernel { namespace cppproxy { 

static jint JNICALL java_1order_1count(JNIEnv* jenv, jobject, jlong cppReference)
{
    Env env = jenv;
    try {
        ::sos::scheduler::order::Order_queue* o_ = has_proxy< ::sos::scheduler::order::Order_queue >::of_cpp_reference(cppReference,"::sos::scheduler::order::Order_queue::java_order_count()");
        return (o_->java_order_count());
    }
    catch(const exception& x) {
        env.set_java_exception(x);
        return jint();
    }
}

}}}}}}}

const static JNINativeMethod native_methods[] = {
    { (char*)"close__native", (char*)"(J)V", (void*)::javaproxy::com::sos::scheduler::engine::kernel::cppproxy::close },
    { (char*)"is_distributed_order_requested__native", (char*)"(JJ)Z", (void*)::javaproxy::com::sos::scheduler::engine::kernel::cppproxy::is_1distributed_1order_1requested__J },
    { (char*)"java_order_count__native", (char*)"(J)I", (void*)::javaproxy::com::sos::scheduler::engine::kernel::cppproxy::java_1order_1count }
};

namespace zschimmer { namespace javabridge { 

    template<> void has_proxy< ::sos::scheduler::order::Order_queue >::register_cpp_proxy_class_in_java() {
        Env env;
        Class* cls = has_proxy< ::sos::scheduler::order::Order_queue >::proxy_class_factory.clas();
        int ret = env->RegisterNatives(*cls, native_methods, sizeof native_methods / sizeof native_methods[0]);
        if (ret < 0)  env.throw_java("RegisterNatives", "com.sos.scheduler.engine.kernel.cppproxy.Order_queueCImpl");
    }

}}