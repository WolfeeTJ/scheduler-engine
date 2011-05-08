#include "precomp.h"
//#define MODULE_NAME "sosstrea"
//#define COPYRIGHT   "(c) 1995 SOS GmbH Berlin"
//#define AUTHOR      "Joacim Zschimmer"

//#pragma implementation

#include "../kram/sos.h"
#include "../kram/xception.h"
#include "../kram/sosstrea.h"

#if !defined SYSTEM_INCLUDE_TEMPLATES && defined JZ_TEST
#   include "../kram/sosstrea.tpl"
#endif

using namespace std;
namespace sos {

//---------------------------------------------------------------------------------write_iso_string

void write_iso_string( Sos_binary_ostream* s, const char* string, uint field_size )
/*
    Schreibt den String im Code ISO 8859-1 (ASCII).
    Der String wird im lolalen Code �bergeben.
    Der String wird auf die L�nge field_size abgeschnitten oder mit Blanks aufgef�llt.
    Beim Abschneiden kann eine Exception auftreten.
*/
{
    int l = min( strlen( string ), field_size );

    s->write_fixed( string, l );
    s->write_byte_repeated( 0x40, field_size - l );
}

//----------------------------------------------------------------------------------read_iso_string

void read_iso_string( Sos_binary_istream* s, char* string_buffer, uint field_size )
/*
    Konvertiert einen ISO 8859-1-String fester L�nge in einen 0-terminierten String im
    lokalen Code, bei dem die Blanks abgeschnitten worden sind.
    Der Puffer f�r den String mu� ein Byte gr��er sein als field_size.
*/
{
    const Byte* p0 = (const Byte*) s->read_bytes( 0 );
    const Byte* p  = p0 + field_size;

    while( p > p0  &&  *(p-1) == 0x20 )  p--;        // Blanks abschneiden

    uint l = p - p0;

    memcpy( string_buffer, p0, l );

    string_buffer[ l ] = '\0';

    s->skip_bytes( field_size );
}

//-------------------------------------------------------------------------------read_ebcdic_string

void read_ebcdic_string( Sos_binary_istream* s, char* string_buffer, uint field_size )
/*
    Konvertiert einen EBCDIC-String fester L�nge in einen 0-terminierten String im
    lokalen Code, bei dem die Blanks abgeschnitten worden sind.
    Der Puffer f�r den String mu� ein Byte gr��er sein als field_size.
*/
{
    const Byte* p0 = (const Byte*) s->read_bytes( 0 );
    const Byte* p  = p0 + field_size;

    while( p > p0  &&  *(p-1) == 0x40 )  p--;        // Blanks abschneiden

    uint l = p - p0;

    xlat( string_buffer, p0, l, ebc2iso );

    string_buffer[ l ] = '\0';

    s->skip_bytes( field_size );
}

//------------------------------------------------------------------------------write_ebcdic_string

void write_ebcdic_string( Sos_binary_ostream* s, const char* string, uint field_size )
/*
    Schreibt den String im Code ISO 8859-1 (EBCDIC).
    Der String wird im lolalen Code �bergeben.
    Der String wird auf die L�nge field_size abgeschnitten oder mit Blanks aufgef�llt.
    Beim Abschneiden kann eine Exception auftreten.
*/
{
    int l = min( strlen( string ), field_size );

    xlat( s->space( l ), string, l, iso2ebc );
    s->write_byte_repeated( 0x40, field_size - l );
}

//-------------------------------------------------------------------------------read_ebcdic_string

void read_ebcdic_string( Sos_binary_istream* s, Area* buffer_ptr, uint field_size )
/*
    Konvertiert einen EBCDIC-String fester L�nge in einen 0-terminierten String im
    lokalen Code, bei dem die Blanks abgeschnitten worden sind.
    Der Puffer f�r den String mu� ein Byte gr��er sein als field_size.
*/
{
    buffer_ptr->allocate_min( field_size + 1 );
    read_ebcdic_string( s, buffer_ptr->char_ptr(), field_size );
    buffer_ptr->length( strlen( buffer_ptr->char_ptr() ));
}

//------------------------------------------------------------------------------write_ebcdic_string

void write_ebcdic_string( Sos_binary_ostream* s, const Const_area& string, uint field_size )
/*
    Schreibt den String im Code ISO 8859-1 (EBCDIC).
    Der String wird im lolalen Code �bergeben.
    Der String wird auf die L�nge field_size abgeschnitten oder mit Blanks aufgef�llt.
    Beim Abschneiden kann eine Exception auftreten.
*/
{
    int l = min( length( string ), field_size );

    xlat( s->space( l ), string.char_ptr(), l, iso2ebc );
    s->write_byte_repeated( 0x40, field_size - l );
}



} //namespace sos
