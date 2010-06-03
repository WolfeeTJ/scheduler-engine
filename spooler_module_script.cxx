/**
 * \file spooler_module_script
 * \brief script-Verarbeitung �ber JAVA interface
 * \details 
 * Verarbeitet Jobs, die in einer Scriptsprache geschrieben sind �ber ein JAVA-Modul
 *
 * \author SS
 * \version 2.1.1 - 2010-05-07
 * \copyright � 2010 by SS
 * <div class="sos_branding">
 *    <p>� 2010 SOS GmbH - Berlin (<a style='color:silver' href='http://www.sos-berlin.com'>http://www.sos-berlin.com</a>)</p>
 * </div>
 */
#include "spooler.h"

using namespace sos::scheduler::scheduler_java;

namespace sos {
namespace scheduler {

//-----------------------------------------------------------------Script_module::Script_module
    
Script_module::Script_module( Spooler* spooler, Prefix_log* log )
:
    Module( spooler, (File_based*)NULL, "::NO-INCLUDE-PATH::", log )
{
    _kind = kind_scripting_engine_java;
    _set = true;
}

//---------------------------------------------------Internal_instance_base::Script_module_instance
    
Script_module_instance::Script_module_instance( Module* module, const string& servicename )             
: 
    Module_instance(module),
    _zero_(this+1),
//    _scriptconnector(servicename),
    _scriptinterface(servicename)                   // instantiert ScriptInterface
{
    Z_LOG2("scheduler","Script_module_instance::Script_module_instance service=" << servicename << "\n");
    _scriptinterface.init(_module->read_source_script());    // Initialisierung und �bergabe des Scriptcodes
    //_servicename = servicename;
    //_scriptinterface = _scriptconnector.getService();
}

//----------------------------------------------------------------Script_module_instance::add_obj
    
void Script_module_instance::add_obj( IDispatch* idispatch, const string& name )
{
    Z_LOG2("scheduler","Script_module_instance::add_obj name=" << name << "\n");

    ptr<Java_idispatch> java_idispatch = Java_subsystem_interface::instance_of_scheduler_object(idispatch, name);
    _added_jobjects.push_back( java_idispatch );
    _scriptinterface.add_obj( java_idispatch->get_jobject(), name );            // registriert das Object in ScriptInterface
}

//-------------------------------------------------------------------Script_module_instance::call

Variant Script_module_instance::call( const string& name )
{
    string f = replace_regex_ext(name,"^(.*).$","\\1");         // nach Java auslagern
    Z_LOG2("scheduler","Script_module_instance::call name=" << f << "\n");
    return _scriptinterface.call(f);
//    _log->info("Script_module_instance::call" );
//    return false;
}

//-------------------------------------------------------------------Script_module_instance::call

Variant Script_module_instance::call( const string&, const Variant&, const Variant& )
{
    Z_LOG2("scheduler","Script_module_instance::call\n");
//    _log->info("Script_module_instance::call");
    assert(0), z::throw_xc( Z_FUNCTION );
}

//------------------------------------------------------------Script_module_instance::name_exists

bool Script_module_instance::name_exists( const string& name )
{
    //TODO in Java implementiren
    // Da erst derzeit keine Funktionalit�t zum �berpr�fen des Vorhandenseins einer Methode gibt,
    // wird erstmal immer true geliefert.
    Z_LOG2("scheduler","Script_module_instance::name_exists name=" << name << "\n");
    return true;
}

//-------------------------------------------------------------------------------------------------

} //namespace scheduler
} //namespace sos

