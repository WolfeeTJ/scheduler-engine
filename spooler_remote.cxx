// $Id$

#include "spooler.h"

// com_remote.cxx �ndern: R4, R8, DATE rechner-unabh�ngig �bertragen!

/*
    Scheduler setzt einen Server auf. 


    VERBINDUNGSAUFBAU
        Verbindung �ber <config tcp_port=>, also mit spooler_communication koppeln.

        com_remote.cxx: Bei Verbindungsaufbau einen Header �bertragen, am besten ein XML-Dokument:

        L�nge in Bytes <LF> XML-Dokument

        Oder HTTP-POST.



    ANMELDUNG
        Client �bertr�gt Referenz auf Spooler, Server legt das in _spooler_proxy ab.
        Der Client �bergibt zugleich ein paar Standardinfos: id, version (mit set_property?).

        Server antwortet mit Referenz auf seinen Spooler und des Remote_scheduler-Objekts.


    ABMELDUNG
        Client ruft logoff() des Remote_scheduler-Objekts auf 


    VERBINDUNGSABBRUCH
        Wenn kein logoff(): _connection_lost = true;

        Remote_scheduler wird nie gel�scht, es bleibt f�r immer im Remote_scheduler_register.





    ASYNCHRONER BETRIEB
        TCP-Verbindung wird von spooler_communication.cxx (Channel) gehalten.
        
        Channel wird mit Remote_scheduler verkn�pft.
        Abstrakte Klasse f�r Remote_scheduler, Http_server, TCP-Kommando.

        Bei Verbindungsende wird Channel gel�scht, nicht aber Remote_scheduler.



    AUFRUF DES ANGEMELDETEN SCHEDULER ALS SERVER
        Eigentlich ist der angemeldete Scheduler der Client.

        Aufruf asynchron: Remote_schedler._remote_scheduler_proxy->call__begin();

        Gleichzeitige Aufrufe beider Seiten vermeiden.
        Oder: Ein Aufruf des anderen Schedulers darf nicht zu einem R�ckruf f�hren.
        Das w�rde bei gleichzeitigen Aufrufen �berkreuz com_remote.cxx nicht unterst�tzen. 
        Und w�re auch verwirrend. Wir brauchen das bestimmt nicht (versuch eine Sperre einzubauen)

*/

namespace sos {
namespace spooler {

Object_server_processor_channel::Object_server_processor_channel( Communication::Channel* ch )
: 
    Communication::Processor_channel( ch )
{
    //_session = Z_NEW( object_server::Session );
}

//-------------------------------------------------Object_server_processor::Object_server_processor

Object_server_processor::Object_server_processor( Object_server_processor_channel* ch )
:
    Communication::Processor(ch),
    _zero_(this+1),
    _processor_channel(ch),
    _input_message( ch->_session ),
    _input_message_builder( &_input_message ),
    _output_message( ch->_session )
{
}

//--------------------------------------------------------Object_server_processor::put_request_part

void Object_server_processor::put_request_part( const char* data, int length )
{ 
    _input_message_builder.add_data( (const Byte*)data, length );
}

//-----------------------------------------------------Object_server_processor::request_is_complete

bool Object_server_processor::request_is_complete()
{ 
    return _input_message.is_complete();
}

//-----------------------------------------------------------------Object_server_processor::process

void Object_server_processor::process()
{
    _processor_channel->_session->execute( &_input_message, &_output_message );
}

//----------------------------------------------------Object_server_processor::response_is_complete

bool Object_server_processor::response_is_complete()
{
    return true;
}

//-------------------------------------------------------Object_server_processor::get_response_part

string Object_server_processor::get_response_part()
{
    string result = _output_message._data;
    _output_message._data = "";
    return result;
}

//-------------------------------------------------Object_server_processor::should_close_connection

bool Object_server_processor::should_close_connection()
{
    return false;
}

//-------------------------------------------------------------------------------------------------

} //namespace spooler
} //namespace sos
