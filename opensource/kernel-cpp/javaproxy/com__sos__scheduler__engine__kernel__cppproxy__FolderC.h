// *** Generated by com.sos.scheduler.engine.cplusplus.generator ***

#ifndef _JAVAPROXY_COM_SOS_SCHEDULER_ENGINE_KERNEL_CPPPROXY_FOLDERC_H_
#define _JAVAPROXY_COM_SOS_SCHEDULER_ENGINE_KERNEL_CPPPROXY_FOLDERC_H_

#include "../zschimmer/zschimmer.h"
#include "../zschimmer/java.h"
#include "../zschimmer/Has_proxy.h"
#include "../zschimmer/javaproxy.h"
#include "../zschimmer/lazy.h"
#include "java__lang__Object.h"

namespace javaproxy { namespace java { namespace lang { struct Object; }}}
namespace javaproxy { namespace java { namespace lang { struct String; }}}


namespace javaproxy { namespace com { namespace sos { namespace scheduler { namespace engine { namespace kernel { namespace cppproxy { 


struct FolderC__class;

struct FolderC : ::zschimmer::javabridge::proxy_jobject< FolderC >, ::javaproxy::java::lang::Object {
  private:
    static FolderC new_instance();  // Not implemented
  public:

    FolderC(jobject = NULL);

    FolderC(const FolderC&);

    #ifdef Z_HAS_MOVE_CONSTRUCTOR
        FolderC(FolderC&&);
    #endif

    ~FolderC();

    FolderC& operator=(jobject jo) { assign_(jo); return *this; }
    FolderC& operator=(const FolderC& o) { assign_(o.get_jobject()); return *this; }
    #ifdef Z_HAS_MOVE_CONSTRUCTOR
        FolderC& operator=(FolderC&& o) { set_jobject(o.get_jobject()); o.set_jobject(NULL); return *this; }
    #endif

    jobject get_jobject() const { return ::zschimmer::javabridge::proxy_jobject< FolderC >::get_jobject(); }

  protected:
    void set_jobject(jobject jo) {
        ::zschimmer::javabridge::proxy_jobject< FolderC >::set_jobject(jo);
        ::javaproxy::java::lang::Object::set_jobject(jo);
    }
  public:


    ::zschimmer::javabridge::Class* java_object_class_() const;

    static ::zschimmer::javabridge::Class* java_class_();


  private:
    struct Lazy_class : ::zschimmer::abstract_lazy<FolderC__class*> {
        void initialize() const;
    };

    Lazy_class _class;
};


}}}}}}}

#endif