// deldir.h

// dirname mu� nat�rlich ein korrekter DOS-Name sein (relativ o. absolut)
// Exceptions: - DOS-Errors ( LOCKED == EACCESS von NetWare! )
//             - DIRNAME (D1??) f�r nicht korrektem Directory-Namen

void deldir ( const char* dirname );
// L�scht rekursiv dirname (OHNE R�ckfrage!!!)