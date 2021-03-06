// *** Generated by com.sos.scheduler.engine.cplusplus.generator ***

#ifndef _JAVAPROXY_COM_SOS_SCHEDULER_ENGINE_KERNEL_CPPPROXY_SETTINGSC_H_
#define _JAVAPROXY_COM_SOS_SCHEDULER_ENGINE_KERNEL_CPPPROXY_SETTINGSC_H_

#include "../zschimmer/zschimmer.h"
#include "../zschimmer/java.h"
#include "../zschimmer/Has_proxy.h"
#include "../zschimmer/javaproxy.h"
#include "../zschimmer/lazy.h"
#include "java__lang__Object.h"

namespace javaproxy { namespace java { namespace lang { struct Object; }}}
namespace javaproxy { namespace java { namespace lang { struct String; }}}
namespace javaproxy { namespace java { namespace util { struct ArrayList; }}}


namespace javaproxy { namespace com { namespace sos { namespace scheduler { namespace engine { namespace kernel { namespace cppproxy { 


struct SettingsC__class;

struct SettingsC : ::zschimmer::javabridge::proxy_jobject< SettingsC >, ::javaproxy::java::lang::Object {
  private:
    static SettingsC new_instance();  // Not implemented
  public:

    SettingsC(jobject = NULL);

    SettingsC(const SettingsC&);

    #ifdef Z_HAS_MOVE_CONSTRUCTOR
        SettingsC(SettingsC&&);
    #endif

    ~SettingsC();

    SettingsC& operator=(jobject jo) { assign_(jo); return *this; }
    SettingsC& operator=(const SettingsC& o) { assign_(o.get_jobject()); return *this; }
    #ifdef Z_HAS_MOVE_CONSTRUCTOR
        SettingsC& operator=(SettingsC&& o) { set_jobject(o.get_jobject()); o.set_jobject(NULL); return *this; }
    #endif

    jobject get_jobject() const { return ::zschimmer::javabridge::proxy_jobject< SettingsC >::get_jobject(); }

  protected:
    void set_jobject(jobject jo) {
        ::zschimmer::javabridge::proxy_jobject< SettingsC >::set_jobject(jo);
        ::javaproxy::java::lang::Object::set_jobject(jo);
    }
  public:


    ::zschimmer::javabridge::Class* java_object_class_() const;

    static ::zschimmer::javabridge::Class* java_class_();


  private:
    struct Lazy_class : ::zschimmer::abstract_lazy<SettingsC__class*> {
        void initialize() const;
    };

    Lazy_class _class;
};


}}}}}}}

#endif
