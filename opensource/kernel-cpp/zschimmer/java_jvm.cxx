// $Id: java_jvm.cxx 13870 2010-06-03 15:29:33Z jz $

#include "zschimmer.h"
#include "java.h"
#include "z_com.h"

/*
    static_jvm liegt in einem eigenen Modul, damit java.cxx nicht eingebunden werden muss,
    nur weil irgendwo gepr�ft wird, ob JavaVM* da ist (f�r z_com.cxx beim Durchreichen der JavaVM).
*/

namespace zschimmer {
namespace javabridge {

//-------------------------------------------------------------------------------------------static

//JavaVM*                         Vm::static_foreign_jvm = NULL;
Mutex                           Vm::static_vm_mutex    ( "java_vm" );

//--------------------------------------------------------------------------------------Vm::get_jvm

JavaVM* Vm::request_jvm()
{
    if( com::static_com_context_ptr )
    {
        return com::static_com_context_ptr->request_javavm();
    }
    else
    {
        Z_MUTEX( static_vm_mutex )
        {
            ptr<Vm> vm = get_vm( true );
            assert( +vm == static_vm );
            static_vm->AddRef();
        }

        return static_vm->vm();
    }
}

//----------------------------------------------------------------------------------Vm::release_jvm

void Vm::release_jvm( JavaVM* jvm )
{
    if( com::static_com_context_ptr )
    {
        com::static_com_context_ptr->release_javavm( jvm );
    }
    else
    {
        Z_MUTEX( static_vm_mutex )
        {
            int ref_count = static_vm->Release();
            if( ref_count == 0 )  static_vm = NULL;
        }
    }
}

//-------------------------------------------------------------------------------------------------


} //namespace javabridge
} //namespace zschimmer
