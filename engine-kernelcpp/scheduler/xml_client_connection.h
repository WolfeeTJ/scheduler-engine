// $Id: xml_client_connection.h 13198 2007-12-06 14:13:38Z jz $        Joacim Zschimmer, Zschimmer GmbH, http://www.zschimmer.com

namespace sos {
namespace scheduler {

//----------------------------------------------------------------------------Xml_client_connection

struct Xml_client_connection : Async_operation, Abstract_scheduler_object
{
    enum State
    {
        s_not_connected,
        s_connecting,
        s_connected,
        s_sending,
        s_waiting,
        s_receiving,
        s_closed
    };

    static string               state_name                  ( State );


                                Xml_client_connection       ( Spooler*, const Host_and_port& );
                               ~Xml_client_connection       ();

    void                        close                       ();
    virtual string              obj_name                    () const;

    void                    set_wait_for_connection         ( int seconds )                         { _wait_for_connection = seconds; }
    State                       state                       () const                                { return _state; }

    void                        connect                     ();
    bool                        is_send_possible            ();
    void                        send                        ( const string& );
    xml::Document_ptr           fetch_received_dom_document ();                                     // NULL, wenn noch nichts empfangen
    bool send_keep_alive_space();      

  protected:
    string                      async_state_text_           () const;
    bool                        async_continue_             ( Continue_flags );
    bool                        async_finished_             () const                                { return _state == s_not_connected 
                                                                                                          || _state == s_connected
                                                                                                          || _state == s_closed; }
    bool                        async_signaled_             ()                                      { return _socket_operation && _socket_operation->async_signaled(); }

  private:
    Fill_zero                  _zero_;
    State                      _state;
    Host_and_port              _host_and_port;
    int                        _wait_for_connection;
    ptr<Buffered_socket_operation>  _socket_operation;
    Xml_end_finder             _xml_end_finder;
    string                     _send_data;
    String_list                _received_data;
};

//-------------------------------------------------------------------------------------------------

} //namespace scheduler
} //namespace sos
