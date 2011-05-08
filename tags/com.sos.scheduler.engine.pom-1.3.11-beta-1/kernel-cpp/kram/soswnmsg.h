// soswnmsg.h                               � 1995 SOS GmbH Berlin

#ifndef __SOSWNMSG_H
#define __SOSWNMSG_H

struct SOS_CLASS Has_mswin_message_handler
{
/*  Registriert eine Fensterklasse und �ffnet ein Fenster dieser Klasse einmal f�r
    alle Has_mswin_message_handler.

    Has_mswin_message_handler::mswin_message_handler() wird f�r Botschaften mit dem Code msg
    aufgerufen. msg mu� eindeutig sein.
*/

    HWND                        hwnd                    () const;
    static const char*          mswin_window_class_name ();
    static const char*          mswin_window_name       ();

  protected:
                                Has_mswin_message_handler()             {}
                                Has_mswin_message_handler( uint msg );
    virtual                    ~Has_mswin_message_handler();

    void                        register_msg            ( uint msg );

    virtual long                mswin_message_handler   ( HWND, UINT, WPARAM, LPARAM, Bool* processed ) = 0;

    friend class                Mswin_msg_window_manager;
};

#endif
