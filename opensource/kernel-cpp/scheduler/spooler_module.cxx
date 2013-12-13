// $Id: spooler_module.cxx 14536 2011-05-31 09:47:32Z ss $

#include "spooler.h"
#include "../file/anyfile.h"
#include "../kram/sos_java.h"

using namespace std;

namespace sos {
namespace scheduler {

//--------------------------------------------------------------------------------------------const

extern const string spooler_init_name           = "spooler_init()Z";
extern const string spooler_exit_name           = "spooler_exit()V";
extern const string spooler_open_name           = "spooler_open()Z";
extern const string spooler_close_name          = "spooler_close()V";
extern const string spooler_process_name        = "spooler_process()Z";
extern const string spooler_on_error_name       = "spooler_on_error()V";
extern const string spooler_on_success_name     = "spooler_on_success()V";
extern const string spooler_api_version_name    = "spooler_api_version()Ljava.lang.String;";
const string        default_monitor_name        = "scheduler";

const string shell_language_name           = "shell";
const string shell_variable_prefix_default = "SCHEDULER_PARAM_";
const int encoding_code_page_none          = -1;

//-----------------------------------------------------------Text_with_includes::Text_with_includes

Text_with_includes::Text_with_includes( Spooler* spooler, File_based* file_based, const File_path& include_path, const xml::Element_ptr& e ) 
: 
    _zero_(this+1),
    _spooler(spooler),
    _file_based(file_based),
    _include_path(include_path)
{ 
    initialize(); 
    if( e )  append_dom( e ); 
}

//-------------------------------------------------------------------Text_with_includes::initialize

void Text_with_includes::initialize()
{
    _dom_document.load_xml_string( "<source/>" );
}

//--------------------------------------------------------------------Text_with_includes::append_dom

void Text_with_includes::append_dom( const xml::Element_ptr& element )
{
    size_t linenr_base = element.line_number();

    for( xml::Node_ptr node = element.firstChild(); node; node = node.nextSibling() )
    {
        string text;

        if( int n = node.line_number() ) 
            if( linenr_base < n )  linenr_base = n;     // libxml2 liefert bei Textknoten keine neue Zeilennummer, deshalb z�hlen wir selbst.

        switch( node.nodeType() )
        {
            case xml::CDATA_SECTION_NODE:
            {
                xml::CDATASection_ptr c = node;
                text = c.data();
                goto TEXT;
            }

            case xml::TEXT_NODE:
            {
                xml::Text_ptr t = node;
                text = t.data();
                goto TEXT;
            }

            TEXT:
            {
                const char* p = text.c_str();   
                while( *p  &&  isspace( (unsigned char)*p ) )  p++;     // Jira JS-60: Die SOS schreibt gerne <script> </script>, was dasselbe sein soll wie <script/>.

                if( *p )    // Nur nicht leeren Text ber�cksichtigen
                {
                    xml::Element_ptr e = _dom_document.documentElement().append_new_cdata_or_text_element( "source_part", text );

                    e.setAttribute( "linenr", linenr_base );
                }

                linenr_base += count( text.begin(), text.end(), '\n' );
                break;
            }

            case xml::COMMENT_NODE:
            {
                xml::Comment_ptr t = node;
                text = t.data();
                linenr_base += count( text.begin(), text.end(), '\n' );
                break;
            }

            case xml::ELEMENT_NODE:     // <include file="..."/>
            {
                xml::Element_ptr e = node;

                if( e.nodeName_is( "include" ) )
                {
                    xml::Element_ptr include_element = _dom_document.createElement( "include" );
                    include_element.setAttribute( "file"     , e.getAttribute( "file"      ) );
                    include_element.setAttribute( "live_file", e.getAttribute( "live_file" ) );

                    _dom_document.documentElement().appendChild( include_element );
                }
                else
                    z::throw_xc( Z_FUNCTION, e.nodeName() );

                break;
            }

            default: ;
        }
    }
}

//------------------------------------------------------------Text_with_includes::includes_resolved

xml::Document_ptr Text_with_includes::includes_resolved() const
{
    // L�st die <include> auf und macht sie zu <source_part>

    xml::Document_ptr result;

    result.create();
    result.importAndAppendChild(_dom_document.documentElement());

    DOM_FOR_EACH_ELEMENT( result.documentElement(), element )
    {
        if( element.nodeName_is( "include" ) )
        {
            Include_command include_command ( _spooler, _file_based, element, _include_path );

            try
            {
                xml::Element_ptr part_element = result.createElement( "source_part" );
                string xml_bytes = include_command.read_content_bytes();
                string xml_string = xml::Document_ptr::from_xml_bytes(xml_bytes).xml_string();
                part_element.appendChild( result.createTextNode(xml_string));
                element.replace_with( part_element );
            }
            catch( exception& x )  { z::throw_xc( "SCHEDULER-399", include_command.obj_name(), x ); }
        }
    }

    return result;
}

//--------------------------------------------------------------------Text_with_includes::read_text

string Text_with_includes::read_text(bool xml_expected)
{
    String_list result;

    DOM_FOR_EACH_ELEMENT( dom_element(), element )
    {
        result.append( read_text_element( element, xml_expected ) );
    }

    return result;
}

//------------------------------------------------------------Text_with_includes::read_text_element

string Text_with_includes::read_text_element( const xml::Element_ptr& element, bool xml_expected )
{
    if( element.nodeName_is( "source_part" ) )
        return element.text();
    else
    if( element.nodeName_is( "include" ) )
    {
        Include_command include_command ( _spooler, _file_based, element, _include_path );

        try {
            string bytes = include_command.read_content_bytes();
            if (xml_expected)
                return xml::Document_ptr::from_xml_bytes(bytes).xml_string();
            else {
                try {
                    return xml::Document_ptr::from_xml_bytes(bytes).xml_string();  // Vielleicht ist es doch ein XML? (JS-898, <script><include file=".xml">)
                }
                catch (exception&) {
                    return bytes;  // Datei wird in string_encoding codiert erwartet
                }
            }
        }
        catch( exception& x )  { z::throw_xc( "SCHEDULER-399", include_command.obj_name(), x ); }
    }
    else
        z::throw_xc( Z_FUNCTION, element.nodeName() );
}

//----------------------------------------------------------Text_with_includes::text_element_linenr

int Text_with_includes::text_element_linenr( const xml::Element_ptr& element )
{
    return element.int_getAttribute( "linenr", 1 );
}

//--------------------------------------------------------Text_with_includes::text_element_filepath

string Text_with_includes::text_element_filepath( const xml::Element_ptr& element )
{
    string result = element.getAttribute( "file" );
    if( result == "" )  result = "<script>";
    return result;
}

//---------------------------------------------------------------------Text_with_includes::is_empty

bool Text_with_includes::is_empty() const
{ 
    if( !_dom_document )  return true;
    if( !_dom_document.documentElement().firstChild() )  return true;

    return !_dom_document.documentElement().first_child_element();

    // Jira JS-60: Die SOS schreibt gerne <script> </script>, was dasselbe sein soll wie <script/>.
    //string text = _dom_document.documentElement().text();
    //const char* p = text.c_str();
    //while( *p  &&  isspace( (unsigned char)*p ) )  p++;
    //return *p == '\0';
}

//-----------------------------------------------------------------------------------Module::Module

Module::Module( Spooler* sp, File_based* file_based, const string& include_path, Has_log* log )
: 
    _zero_(_end_), 
    _spooler(sp), 
    _file_based(file_based),
    _log(log),
    _process_environment( new Com_variable_set() ),
    _include_path(include_path),
    _text_with_includes(sp,file_based,include_path)
{
    init0();
}

//-----------------------------------------------------------------------------------Module::Module
    
Module::Module( Spooler* sp, File_based* file_based, const xml::Element_ptr& e, const string& include_path )  
: 
    _zero_(_end_),
    _spooler(sp),
    _process_environment( new Com_variable_set() ),
    _include_path(include_path),
    _text_with_includes(sp,file_based,include_path)
{ 
    init0();
    set_dom( e ); 
}

//-------------------------------------------------------------------------------------------------

void Module::init0()
{
#   ifndef Z_WINDOWS
        _process_environment->_ignore_case = false;
#   endif

    #ifdef Z_WINDOWS        
        _encoding_code_page = CP_OEMCP;
    #endif

    string prefix = "";
    _process_shell_variable_prefix_is_configured = false;
    if (_spooler && _spooler->variables()) {
      prefix = _spooler->variables()->get_string( "scheduler.variable_name_prefix" );
      if (prefix.empty())        // try the old name ...
         prefix = _spooler->variables()->get_string( "SCHEDULER_VARIABLE_NAME_PREFIX" );
      if (!prefix.empty())
         _process_shell_variable_prefix_is_configured = true;     // this flag controls the transfer to remote process (see Remote_module_instance_proxy::continue_async_operation)
    }
    _process_shell_variable_prefix = (prefix.empty()) ? shell_variable_prefix_default : (prefix == "*NONE") ? "" : prefix;

    _monitors = Z_NEW( Module_monitors( this ) );
}

//-----------------------------------------------------------------------------Module::set_priority
    
void Module::set_priority( const string& priority )
{
    if( priority != "" )
    {
#       ifdef Z_WINDOWS
            windows::priority_class_from_string( priority );    // Pr�fen
#        else
            posix::priority_from_string( priority );            // Pr�fen
#       endif
    }

    _priority = priority;
}

//----------------------------------------------------------------------------set_checked_attribute

void Module::set_checked_attribute( string* variable, const xml::Element_ptr& element, const string& attribute_name, bool modify_allowed )
{
    if( !_initialized  ||  ( modify_allowed && *variable != "" ) ) 
    {
        *variable = element.getAttribute( attribute_name, *variable );
    }
    else
    if( element.hasAttribute( attribute_name )  &&  element.getAttribute( attribute_name ) != *variable )  
        z::throw_xc( "SCHEDULER-234", attribute_name + "=\"" + *variable + '"' );
}

//------------------------------------------------------------------------------Module::set_process
// <process> hat kein <script>, deshalb dieser Aufruf
// Besser w�re, <process> durch <script language="shell"> zu ersetzen

void Module::set_process()
{
    _language = shell_language_name;
    //_source.clear();
    _set = true;
}

//----------------------------------------------------------------------------------Module::set_dom

void Module::set_dom( const xml::Element_ptr& element )  
{ 
    if( !element )  return;

    _text_with_includes.append_dom( element );

    _recompile = element.bool_getAttribute( "recompile", true );

    set_checked_attribute( &_language          , element, "language"         );
    set_checked_attribute( &_com_class_name    , element, "com_class" , true );
    set_checked_attribute( &_filename          , element, "filename"         );
    set_checked_attribute( &_java_class_name   , element, "java_class", true );
    set_checked_attribute( &_java_class_path   , element, "java_class_path", true );  // JS-540

    #ifdef Z_64
        string language_lcase = lcase( _language );
        if (language_lcase == "javascript" || language_lcase == "spidermonkey" ) 
            z::throw_xc( "SCHEDULER-482", _language );  // Unter 64bit stellen wir Spidermonkey nicht mehr bereit.
    #endif 

    if( element.hasAttribute( "encoding" ) )
    {
      #ifdef Z_WINDOWS        
        string code_page_string = lcase( element.getAttribute( "encoding" ) );
        
        // Das XML-Schema scheduler.xsd schr�nkt die Codierungen auf die bekannten ein.

        if( lcase( code_page_string ) ==          "oem"         )  _encoding_code_page = CP_OEMCP;
        else
        if( string_begins_with( code_page_string, "cp"        ) )  _encoding_code_page = as_int( code_page_string.substr( 2 ) );   // 437, 850 usw.
        else
        if( string_begins_with( code_page_string, "windows-"  ) )  _encoding_code_page = as_int( code_page_string.substr( 8 ) );   // 1252 usw.
        else
        if( string_begins_with( code_page_string, "iso-8859-" ) )  _encoding_code_page = 28591 - 1 + as_int( code_page_string.substr( 8 ) );   // iso-8859-1 usw.
        else
        if( code_page_string ==                   "latin1"      )  _encoding_code_page = 28591;     // iso-8859-1
        else
        if( code_page_string ==                   "none"        )  _encoding_code_page = encoding_code_page_none;
        else
      #endif
            z::throw_xc( "SCHEDULER-441", element.nodeName(), "encoding", element.getAttribute( "encoding" ) );
    }

    //if( _use_process_class )
    //{
        //set_checked_attribute( &_process_class_string, element, "process_class"    );
        //if( _process_class_string != "" )  _process_class_path = Absolute_path( _folder_path, _process_class_string );
    //}

    _set = true;
}

//--------------------------------------------------------Module::set_xml_string_text_with_includes

void Module::set_xml_string_text_with_includes(const string& x)
{
    _text_with_includes.set_xml_string( x );
    _set = true;
}

//-------------------------------------------------------------------------------------Module::init

void Module::init()
{
    if( _initialized )  return;
    
    _monitors->initialize();


    if( _kind == kind_none )    // Wenn nicht, dann kind_internal
    {
#       ifdef Z_WINDOWS
        if( _com_class_name != "" )
        {
            _kind = kind_com;
        
            if( _language        != "" )  z::throw_xc( "SCHEDULER-145" );
            if( _java_class_name != "" )  z::throw_xc( "SCHEDULER-168" );
        }
        else
#       endif
        if( _java_class_name != ""  ||  lcase(_language) == "java" )
        {
            _kind = kind_java;
        
            if( _language == "" )  _language = "Java";

            if( lcase(_language) != "java" )  z::throw_xc( "SCHEDULER-166" );
            if( _com_class_name  != ""     )  z::throw_xc( "SCHEDULER-168" );
        }
        else
        if( _process_filename != ""  || _language == shell_language_name )  //   Z_POSIX_ONLY( || _language == ""  &&  string_begins_with( _source, "#!" ) ) )
        {
            _kind = kind_process;
        }
        else
        {
            _kind = kind_scripting_engine;
            if( _language == "" )  _language = SPOOLER_DEFAULT_LANGUAGE;
            if (string_begins_with(_language, "javax.script:") || string_begins_with(_language, "java:")) {
                // "javax.script:rhino" f�r Java-Methoden-Schnittstelle, z.B. spooler_task.set_exit_code( 0 );
                // "java:rhino" f�r Beans-Schnittstelle (Property-Schnittstelle), z.B. spooler_task.exit_code = 0;
                Z_LOG2("scheduler", "Using java interface for scripting language " << _language << "\n");
                _kind = kind_scripting_engine_java;
            }
        }
    }


    if( _kind != kind_internal )
    {
        if( _spooler )  _use_process_class = !_spooler->_ignore_process_classes;  //process_class_subsystem()->has_process_classes();
        if( _dont_remote )  _use_process_class = false, _process_class_path.clear();
    }


    switch( _kind )
    {
        case kind_internal:             if( _process_class_path != ""  )
                                            if( Process_class* process_class = process_class_or_null() )
                                                if( process_class->remote_scheduler() )  z::throw_xc( "SCHEDULER-REMOTE-INTERNAL?" );
                                        break;

        case kind_remote:               break;

        case kind_java:                 break;

// JS-498: Vorhandensein von Scriptcode pr�fen
        case kind_scripting_engine_java: break;
        
        case kind_scripting_engine:     if( !has_source_script() )  z::throw_xc( "SCHEDULER-173" );
                                        break;

        case kind_process:              if( !has_source_script()  &&  _process_filename.empty() )  z::throw_xc( "SCHEDULER-173" );
                                        break;

#       ifdef Z_WINDOWS
            case kind_com:              if( has_source_script() )  z::throw_xc( "SCHEDULER-167" );
                                        break;
#       endif

        default:                        assert(0), z::throw_xc( Z_FUNCTION );
    }

    _initialized = true;
}

//--------------------------------------------------------------------------Module::create_instance

ptr<Module_instance> Module::create_instance()
{
    ptr<Module_instance> result = create_instance_impl();

    if( !_monitors->is_empty() )
    {
        if( _kind == kind_internal )  z::throw_xc( "SCHEDULER-315", "Internal job" );
        
        if( !_use_process_class )  
        {
            result->_monitor_instances.create_instances();
        }
    }

    return result;
}

//---------------------------------------------------------------------Module::create_instance_impl

ptr<Module_instance> Module::create_instance_impl()
{
    ptr<Module_instance> result;


    Kind kind = _kind;
    
    if( _use_process_class  &&
        ( has_api() || process_class()->is_remote_host() ) )     // Nicht-API-Tasks (einfache Prozesse) nicht �ber Prozessklasse abwickeln
    {
        kind = kind_remote;                 
    }

    switch( kind )
    {
        case kind_java:              
        {
            if( _spooler )  if( !_spooler->java_subsystem()->java_vm()  ||  !_spooler->java_subsystem()->java_vm()->running() )  z::throw_xc( "SCHEDULER-177" );

            _java_vm = get_java_vm( false );
            _java_vm->set_destroy_vm( false );   //  Nicht DestroyJavaVM() rufen, denn das h�ngt manchmal

            if( !_java_vm->running() )
            {
                init_java_vm( _java_vm );     // Native Java-Methoden (Callbacks) bekannt machen
            }
            
            ptr<Java_module_instance> p = Z_NEW( Java_module_instance( this ) );
            result = +p;
            break;
        }

        // JS-498: neue Instanz f�r java-script via Java-Interface 
        case kind_scripting_engine_java:
        {
            if( _spooler )  if( !_spooler->java_subsystem()->java_vm()  ||  !_spooler->java_subsystem()->java_vm()->running() )  z::throw_xc( "SCHEDULER-177" );

            _java_vm = get_java_vm( false );
            _java_vm->set_destroy_vm( false );   //  Nicht DestroyJavaVM() rufen, denn das h�ngt manchmal

            if( !_java_vm->running() )
            {
                init_java_vm( _java_vm );     // Native Java-Methoden (Callbacks) bekannt machen
            }
            
            ptr<Java_module_script_instance> p = Z_NEW( Java_module_script_instance( this ) );
            result = +p;
            break;
        }

        case kind_scripting_engine:  
        {
            ptr<Scripting_engine_module_instance> p = Z_NEW( Scripting_engine_module_instance( this ) );
            result = +p;
            break;
        }

#     ifdef Z_WINDOWS
        case kind_com:               
        {
            ptr<Com_module_instance> p = Z_NEW( Com_module_instance( this ) );
            result = +p;
            break;
        }
#     endif

        case kind_process:
        {
            ptr<Process_module_instance> p = Z_NEW( Process_module_instance( this ) );
            result = +p;
            break;
        }

        case kind_remote:
        {
            ptr<Remote_module_instance_proxy> p = Z_NEW( Remote_module_instance_proxy( this ) );
            result = +p;
            break;
        }

        case kind_internal:
        {
            ptr<Internal_module_instance> p = Z_NEW( Internal_module_instance( this ) );
            result = +p;
            break;
        }

        default:                     
            z::throw_xc( "SCHEDULER-173" );
    }

    result->_kind = kind;

    return result;
}

//----------------------------------------------------------------------------------Module::has_api

bool Module::has_api() const
{ 
    return _kind != kind_process || !_monitors->is_empty(); 
}

//--------------------------------------------------------------------Module::process_class_or_null

Process_class* Module::process_class_or_null() const
{ 
    Process_class* result = NULL;

    if( _use_process_class )
    {
        result = _spooler->process_class_subsystem()->process_class_or_null( _process_class_path );
    }

    return result;
}

//----------------------------------------------------------------------------Module::process_class

Process_class* Module::process_class() const
{ 
    //kind_process darf das (f�r remote_scheduler)  if( !_use_process_class )  assert(0), z::throw_xc( "NO_PROCESS_CLASS", Z_FUNCTION );

    // F�r kind_process (ohne kind_remote) wird die Prozessklasse nicht wirklich benutzt, d.h. das Prozesslimit wirkt nicht.
    // Besser w�re das Limit zu ber�cksichtigen (�ber Dummy-Process?).

    return _spooler->process_class_subsystem()->process_class( _process_class_path );
}

//-------------------------------------------------------------------------------Module::needs_java

bool Module::needs_java() 
{
    bool result = _kind == Module::kind_java  &&  has_source_script();
    if( !result )  result = _kind == Module::kind_scripting_engine_java  &&  has_source_script();           // JS-498

    if( !result )  result = _monitors->needs_java();

    return result;
}

//----------------------------------------------------------------Module_instance::In_call::In_call

Module_instance::In_call::In_call( Module_instance* module_instance, const string& name, const string& extra ) 
: 
    _module_instance(NULL),
    _result_set(false)
{ 
    if( !module_instance->_in_call )       // In_call kann doppelt gerufen werden (explizit und implizit). Der zweite Aufruf wirkt nicht.
    {
        _module_instance = module_instance;

        size_t pos = name.find( '(' );
        _name = pos == string::npos? name : name.substr( 0, pos );

        _module_instance->set_in_call( this, extra ); 
        Z_LOG2( "scheduler.call", *_module_instance << '.' << _name << "() begin\n" );

        Z_WINDOWS_ONLY( _ASSERTE( _CrtCheckMemory() ); )
    }
}

//---------------------------------------------------------------Module_instance::In_call::~In_call

Module_instance::In_call::~In_call()
{ 
    if( _module_instance )
    {
        _module_instance->set_in_call( NULL ); 

        if( z::Log_ptr log = "scheduler.call" )
        {
            *log << *_module_instance << '.' << _name << "() end";
            if( _result_set )  *log << "  result=" << ( _result? "true" : "false" );
            *log << '\n';
        }

        Z_WINDOWS_ONLY( _ASSERTE( _CrtCheckMemory() ); )
    }
}

//-----------------------------------------------------------------Module_instance::Module_instance

Module_instance::Module_instance( Module* module )
: 
    _zero_(_end_), 
    _spooler(module->_spooler),
    _module(module),
    _log(module?module->_log:NULL),
    _monitor_instances( &_log, _module->_monitors )
{
    _com_task    = new Com_task;
    _com_log     = new Com_log;
    _process_environment = new Com_variable_set();
    _process_environment->merge( _module->_process_environment );
    _spooler_exit_called = false;

  //_close_instance_at_end;         // Das verhindert aber use_engine="job". Aber vielleicht braucht das keiner.
}

//----------------------------------------------------------------Module_instance::~Module_instance

Module_instance::~Module_instance()
{
    try
    {
        detach_process();
    }
    catch( exception& x ) { Z_LOG2( "scheduler", Z_FUNCTION << "  ERROR  " << x.what() << "\n" ); }

    if( _com_log  )  _com_log ->set_log ( NULL );
    if( _com_task )  _com_task->set_task( NULL );
}

//----------------------------------------------------------------------------Module_instance::init

void Module_instance::init()
{
    _initialized = true;
    //_spooler = _module->_spooler;

    if( !_module->set() )  z::throw_xc( "SCHEDULER-146" );

    _monitor_instances.init();
}

//---------------------------------------------------------------------------Module_instance::clear

void Module_instance::clear()
{ 
    _object_list.clear(); 
    _monitor_instances.clear_instances();
}

//--------------------------------------------------------------------Module_instance::set_job_name

void Module_instance::set_job_name( const string& job_name )
{
    _job_name = job_name; 
    _monitor_instances.set_job_name( job_name );
}

//---------------------------------------------------------------------Module_instance::set_task_id

void Module_instance::set_task_id( int id )
{ 
    _task_id = id; 
    _monitor_instances.set_task_id( id );
}

//-------------------------------------------------------------------------Module_instance::set_log

void Module_instance::set_log( Prefix_log* log )
{ 
    _com_log->set_log( log );

    set_log( (Has_log*)log );
}

//-------------------------------------------------------------------------Module_instance::set_log

void Module_instance::set_log( Has_log* log )
{ 
    _log = log; 
    _monitor_instances.set_log( log );
}

//--------------------------------------------------------------------------------Task::set_in_call

void Module_instance::set_in_call( In_call* in_call, const string& extra )
{
    _in_call = in_call;

    if( in_call  &&  _spooler  &&  _spooler->_debug )
    {
        _log.debug( in_call->_name + "()  " + extra );
    }
}

//---------------------------------------------------------------------Module_instance::attach_task

void Module_instance::attach_task( Task* task, Prefix_log* log )
{
    _task = task;

    set_log( log );
    _com_task->set_task( task );

    _task_id = task->id();
    //_title = task->obj_name();          // Titel f�r Prozess
    _monitor_instances.attach_task( task, log );

    _has_order = task->order() != NULL;      // R�ckgabe von Auftragsparametern �ber Datei erm�glichen

    fill_process_environment();
}

//--------------------------------------------------------Module_instance::fill_process_environment

void Module_instance::fill_process_environment()
{
    //if( _module->kind() == Module::kind_process )
    //{
    //    fill_process_environment_with_params();
    //    // JS-147: <environment> kommt nach <params>, deshalb Rest von attach_task() erst jetzt ausf�hren.
    //}


    // Environment, eigentlich nur bei einem Prozess n�tig, also nicht bei <process_classes ignore="yes"> und <monitor>)
    if( _task->environment_or_null() )  _process_environment->merge( _task->environment_or_null() );


    if( _module->kind() == Module::kind_process )
    {
        string dir = _spooler->_configuration_directories[ confdir_local ];
        
        if( string_ends_with( dir, "/" ) ||
            string_ends_with( dir, Z_DIR_SEPARATOR ) )  dir.erase( dir.length() - 1 );

        _process_environment->set_var( "SCHEDULER_CONFIGURATION_DIRECTORY", dir );      // Path of the Configuration Directory with hot folders
        _process_environment->set_var( "SCHEDULER_JOB_CONFIGURATION_DIRECTORY", 
            _task->job()->has_base_file()? _task->job()->base_file_info()._path.directory() : File_path() );  // Directory for the job configuration file should dynamic configuration from hot folders be used
        _process_environment->set_var( "SCHEDULER_HOST"              , _spooler->_short_hostname );
        _process_environment->set_var( "SCHEDULER_TCP_PORT"          , _spooler->tcp_port()? as_string( _spooler->tcp_port() ) : "" );
        _process_environment->set_var( "SCHEDULER_UDP_PORT"          , _spooler->udp_port()? as_string( _spooler->udp_port() ) : "" );

        Host_and_port supervisor_host_and_port;
        if( _spooler->_supervisor_client )  supervisor_host_and_port = _spooler->_supervisor_client->host_and_port();
        _process_environment->set_var( "SCHEDULER_SUPERVISOR_HOST"   , supervisor_host_and_port.host().name_or_ip() );
        _process_environment->set_var( "SCHEDULER_SUPERVISOR_PORT"   , supervisor_host_and_port.string_port() );

        _process_environment->set_var( "SCHEDULER_JOB_NAME"          , _task->job()->name() );
        _process_environment->set_var( "SCHEDULER_TASK_ID"           , S() << _task->id() );
        _process_environment->set_var( "SCHEDULER_TASK_TRIGGER_FILES", _task->trigger_files() );

        if( Order* order = _task->order() )
        {
            _process_environment->set_var( "SCHEDULER_JOB_CHAIN", order->job_chain_path().name() );
            _process_environment->set_var( "SCHEDULER_JOB_CHAIN_CONFIGURATION_DIRECTORY", 
                order->job_chain() && order->job_chain()->has_base_file()? order->job_chain()->base_file_info()._path.directory() : File_path()  );  // Directory for the job_chain configuration file should dynamic configuration from hot folders be used (for jobs with order=yes)
            _process_environment->set_var( "SCHEDULER_ORDER_ID" , order->string_id() );
        }
    }
}

//---------------------------------------------------------------------Module_instance::detach_task

void Module_instance::detach_task()
{
    close_monitor();

    _com_task->set_task( NULL );
    _com_log ->set_log ( NULL );
    
    _task = NULL;
    _task_id = 0;
    //_title = "";
    _monitor_instances.detach_task();
}

//-------------------------------------------------------------------------Module_instance::add_obj

void Module_instance::add_obj( IDispatch* object, const string& name )
{
    _monitor_instances.add_obj( object, name );
}

//--------------------------------------------------------------------------Module_instance::object

IDispatch* Module_instance::object( const string& name )
{
    IDispatch* result = object( name, NULL );
    if( !result )  assert(0), throw_xc( "Module_instance::object", name );
    return result;
}

//--------------------------------------------------------------------------Module_instance::object

IDispatch* Module_instance::object( const string& name, IDispatch* deflt )
{
    Z_FOR_EACH( Object_list, _object_list, o )
    {
        if( o->_name == name )  return o->_object;
    }
    
    return deflt;
}

//----------------------------------------------------------------------------Module_instance::load

bool Module_instance::load()
{
    bool ok = _monitor_instances.load();
    _load_called = true;
    return ok;
}

//--------------------------------------------------------------Module_instance::try_to_get_process

bool Module_instance::try_to_get_process()
{
    if( !_process )
    {
        if( _module->_process_class_path.empty()  
            &&  !_spooler->process_class_subsystem()->process_class_or_null( _module->_process_class_path ) )   
        {
            // Namenlose Prozessklasse nicht bekannt? Dann tempor�re Prozessklasse verwenden
            _process = _spooler->process_class_subsystem()->new_temporary_process();
        }
        else
        {
            _process = _spooler->process_class_subsystem()->process_class( _module->_process_class_path ) -> select_process_if_available();
        }

        if( _process )
        {
            _process->add_module_instance( this );
            _process->set_login(_module->_login);
            assert( !_process->started() );

            _process->set_job_name( _job_name );
            _process->set_task_id ( _task_id  );
        }

        // _process wird nur von Remote_module_instance_procy benutzt. 
        // Sonst ist _process ein Dummy, um die Zahl der Prozesse gegen max_processes der Prozessklasse zu pr�fen.
    }

    return true;
}

//---------------------------------------------------------------------------Module_instance::start

void Module_instance::start()
{
    //Schon in implicit_load_and_start() erledigt: if( _monitor_instance ) ...
}

//------------------------------------------------------------------Module_instance::call_if_exists

Variant Module_instance::call_if_exists( const string& name )
{
    if( name_exists(name) )  return call( name );
                       else  return Variant( Variant::vt_error, DISP_E_UNKNOWNNAME );
}

//------------------------------------------------------------------Module_instance::call_if_exists

Variant Module_instance::call_if_exists( const string& name, const Variant& param )
{
    if( name_exists(name) )  return call( name, param );
                       else  return Variant( Variant::vt_error, DISP_E_UNKNOWNNAME );
}

//---------------------------------------------------------------------------Module_instance::close

void Module_instance::close()
{
    Async_operation* op = close__start();
    if( !op->async_finished() )  _log.warn( message_string( "SCHEDULER-293" ) );        // "Warten auf Schlie�en der Modulinstanz ..."
    close__end();
}

//--------------------------------------------------------------------Module_instance::close__start

Async_operation* Module_instance::close__start()
{ 
    return &dummy_sync_operation; 
}

//----------------------------------------------------------------------Module_instance::close__end

void Module_instance::close__end()
{
    close_monitor();
}

//------------------------------------------------------------------Module_instance::detach_process

void Module_instance::detach_process()
{
    if( _process )
    {
        _process->remove_module_instance( this );
        _process = NULL;
    }
}

//-------------------------------------------------------------------Module_instance::close_monitor

void Module_instance::close_monitor()
{
    _monitor_instances.close_instances();
    _monitor_instances.clear_instances();
}

//--------------------------------------------------------------------Module_instance::begin__start

Async_operation* Module_instance::begin__start()
{
    return &dummy_sync_operation;
}

//---------------------------------------------------------Module_instance::implicit_load_and_start

bool Module_instance::implicit_load_and_start()
{
    if( !_initialized )  init();

    FOR_EACH_CONST( Object_list, _object_list, o )  add_obj( o->_object, o->_name );

    bool ok = load();
    if( !ok )  return false;

    start();
    return true;
}

//----------------------------------------------------------------------Module_instance::begin__end

bool Module_instance::begin__end()
{
    // Wird nach Task.Call_me_again_when_locks_available() wiederholt aufgerufen.
    // Dann soll der letzte Aufruf spooler_open() wiederholt werden.

    if( !_load_called )
    {
        bool ok = implicit_load_and_start();
        if( !ok )  return false;
    }

    if( !_spooler_init_called )
    {
        _spooler_init_called = true;
        bool ok = check_result( call_if_exists( spooler_init_name ) );
        if( !ok )  return ok;
    }

    return true;
    //_spooler_open_called = true;
    //return check_result( call_if_exists( spooler_open_name ) );
}

//----------------------------------------------------------------------Module_instance::end__start

Async_operation* Module_instance::end__start( bool )
{
    return &dummy_sync_operation;
}

//------------------------------------------------------------------------Module_instance::end__end

void Module_instance::end__end()
{
    if( !loaded() )  return;

    if( _spooler_open_called  &&  !_spooler_close_called )
    {
        _spooler_close_called = true;
        call_if_exists( spooler_close_name );
    }
}

//---------------------------------------------------------------------Module_instance::step__start

Async_operation* Module_instance::step__start()
{
    return &dummy_sync_operation;
}

//-----------------------------------------------------------------------Module_instance::step__end

Variant Module_instance::step__end()
{
    Variant result;

    if( _monitor_instances.is_empty() )
    {
        result = call_if_exists( spooler_process_name );
    }
    else
    {
        result = _monitor_instances.spooler_process_before();

        if( check_result( result ) )
        {
            try
            {
                result = call_if_exists( spooler_process_name );
            }
            catch( Xc& x )
            {
                com_call( object( "spooler_task", _com_task ), "Set_error_code_and_text", x.code(), x.what() );
                result = false;
            }
            catch( zschimmer::Xc& x )
            {
                com_call( object( "spooler_task", _com_task ), "Set_error_code_and_text", x.code(), x.what() );
                result = false;
            }
            catch( exception& x )
            {
                com_call( object( "spooler_task", _com_task ), "Set_error_code_and_text", "", x.what() );
                result = false;
            }

            result = _monitor_instances.spooler_process_after( result );
        }
    }

    return result;
}

//---------------------------------------------------------------------Module_instance::call__start

Async_operation* Module_instance::call__start( const string& method )
{
    _call_method = method;
    return &dummy_sync_operation;
}

//-----------------------------------------------------------------------Module_instance::call__end

Variant Module_instance::call__end()
{
    if( _call_method == spooler_exit_name  &&  !loaded() )  return true;

  //if( _call_method == wait_for_subprocesses_name )        // Keine Methode des Jobs.
  //{
  //    return _com_task->_task->wait_for_subprocesses();   // Siehe auch Com_remote_module_instance_server
  //}
  //else
    if( _call_method == spooler_open_name )
    {
        _spooler_open_called = true;
    }
    else
    if( _call_method == spooler_on_success_name   
     || _call_method == spooler_on_error_name )
    {
        if( !_spooler_open_called )  return true;
    }
    else
    if( _call_method == spooler_exit_name )  
    {
        if( _spooler_exit_called )  return true;
        _spooler_exit_called = true;
    }

    return call_if_exists( _call_method );
}

//------------------------------------------------------------------Module_instance::release__start

Async_operation* Module_instance::release__start()
{
    return &dummy_sync_operation;
}

//--------------------------------------------------------------------Module_instance::release__end

void Module_instance::release__end()
{
    //close();
}

//------------------------------------------------------------------------Module_instance::end_task

void Module_instance::end_task()
{
    assert( _task );
    if( _task )  _task->cmd_end();
}

//-------------------------------------------------------------------------Module_monitors::set_dom

void Module_monitors::set_dom( const xml::Element_ptr& element )
{
    if( !element.nodeName_is( "monitor" ) )  assert(0), z::throw_xc( "SCHEDULER-409", "monitor", element.nodeName() );
    
    string name = element.getAttribute( "name", default_monitor_name );

    ptr<Module_monitor> monitor = monitor_or_null( name );

    if( !monitor )
    {
        monitor = Z_NEW( Module_monitor() );
        monitor->_name   = name;
        monitor->_module = Z_NEW( Module( _main_module->_spooler, _main_module->_file_based, _main_module->_spooler->include_path(), &_main_module->_log ) );
        add_monitor( monitor );
    }

    monitor->_ordering = element.int_getAttribute( "ordering", monitor->_ordering );

    DOM_FOR_EACH_ELEMENT( element, e )
    {
        if( e.nodeName_is( "script" ) )  
        {
            monitor->_module->set_dom( e );
        }
    }
}

//----------------------------------------------------------------------Module_monitors::needs_java

bool Module_monitors::needs_java() 
{
    bool result = false;

    Z_FOR_EACH( Module_monitors::Monitor_map, _monitor_map, m )
    {
        Module_monitor* monitor = m->second;
        result = monitor->_module->needs_java();
        if( result )  break;
    }

    return result;
}
//-----------------------------------------------------------------Module_monitors::monitor_or_null

Module_monitor* Module_monitors::monitor_or_null( const string& name )
{
    Module_monitor* result = NULL;

    Monitor_map::iterator m = _monitor_map.find( name );
    if( m != _monitor_map.end() )  result = m->second;

    return result;
}

//----------------------------------------------------------------------Module_monitors::initialize

void Module_monitors::initialize()
{
    vector<Module_monitor*> ordered_monitors = this->ordered_monitors();

    Z_FOR_EACH( vector<Module_monitor*>, ordered_monitors, m )
    {
        Module_monitor* monitor = *m;
        monitor->_module->init();
    }
}

//-------------------------------------------------------------Module_monitors::ordered_module_list

vector<Module_monitor*> Module_monitors::ordered_monitors()
{
    vector<Module_monitor*> result;

    result.reserve( _monitor_map.size() );
    Z_FOR_EACH( Monitor_map, _monitor_map, m )  result.push_back( m->second );
    sort( result.begin(), result.end(), Module_monitor::less_ordering );

    return result;
}

//-------------------------------------------------Module_monitor_instance::Module_monitor_instance

Module_monitor_instance::Module_monitor_instance( Module_monitor* monitor, Module_instance* module_instance )
:
    _module_instance(module_instance),
    _obj_name( "Script_monitor " + monitor->name() )
{
}

//-------------------------------------------------------------------------------------------------

} //namespace scheduler
} //namespace sos
