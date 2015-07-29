// $Id: spooler_module_com.h 12552 2007-01-20 23:14:22Z jz $        Joacim Zschimmer, Zschimmer GmbH, http://www.zschimmer.com

#ifndef __SPOOLER_MODULE_COM_H
#define __SPOOLER_MODULE_COM_H

namespace sos {
namespace scheduler {

//-------------------------------------------------------------------------------------------------

bool                            check_result                ( const Variant& vt );

//-------------------------------------------------------------------------Com_module_instance_base
// Oberklasse

struct Com_module_instance_base : Module_instance
{
                                Com_module_instance_base    ( Module* module )                      : Module_instance(module) {}

    void                        init                        ();
    IDispatch*                  dispatch                    () const                                { return _idispatch; }
    using Module_instance::add_obj;
    void                        close__end                  ();
    Variant                     call                        ( const string& name );
    Variant                     call                        ( const string& name, const Variant& param, const Variant& );
    bool                        name_exists                 ( const string& name );
    bool                        loaded                      ()                                      { return _idispatch != NULL; }
    bool                        callable                    ()                                      { return _idispatch != NULL; }


    ptr<IDispatch>             _idispatch;
};

//------------------------------------------------------------------------------Com_module_instance
// Für COM-Objekte
#ifdef Z_WINDOWS

struct Com_module_instance : Com_module_instance_base
{
                                Com_module_instance         ( Module* );
                               ~Com_module_instance         ();

    void                        init                        ();
    virtual void                add_obj                     ( IDispatch*, const string& name );
    bool                        load                        ();
    void                        close__end                  ();


    typedef HRESULT (WINAPI *DllGetClassObject_func)(CLSID*,IID*,void**);

    Fill_zero                  _zero_;
    ptr<Com_context>           _com_context;

    HMODULE                    _com_module;                 // Für _module->_filename != ""
    DllGetClassObject_func     _DllGetClassObject;          // Für _module->_filename != ""

    Fill_end                   _end_;
};

#endif
//-----------------------------------------------------------------Scripting_engine_module_instance
// Für Scripting Engines

struct Scripting_engine_module_instance : Com_module_instance_base
{
                                Scripting_engine_module_instance( Module* script )                  : Com_module_instance_base(script) {}
                               ~Scripting_engine_module_instance();

    void                        close__end                  ();
    void                        init                        ();
    bool                        load                        ();
    void                        start                       ();
    virtual void                add_obj                     ( IDispatch*, const string& name );
    Variant                     call                        ( const string& name );
    Variant                     call                        ( const string& name, const Variant&, const Variant& );


    ptr<Script_site>           _script_site;
};

//-------------------------------------------------------------------------------------------------

} //namespace scheduler
} //namespace sos

#endif
