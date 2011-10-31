//
// $Id: folder.cxx 14329 2011-05-12 13:43:35Z ss $        Joacim Zschimmer, Zschimmer GmbH, http://www.zschimmer.com

#include "spooler.h"

namespace sos {
namespace scheduler {
namespace folder {



//  VERSUCH EINER KL�RUNG            Joacim Zschimmer 2008-05-01
//
//  Wie das Modell der File_based, Dependant, Requisiten, Subsysteme usw. aussehen sollte, 
//  sch�lt sich erst nach und nach heraus.
//
//
//
//  GENERISCHE ZUST�NDE
//
//  Au�er den File_based gibt es noch andere Objekte, die die generischen Zust�nde von File_based haben sollten,
//  z.B. Job_chain_node und Schedule_use.
//
//  Ein Objekt mit generischem Zustand sollte ein In_generic_state sein. Dependant sollte ein In_generic_state sein.
//  initialized(), load(), activate() geh�ren hierher.
//
//  s_undefined,            // Anf�nglicher Zustand
//  s_not_initialized,      // set_dom() noch nicht aufgerufen (oder war fehlerhaft)
//  s_initialized,          // on_initialized() aufgerufen. Replacement kann jetzt das alte Objekt ersetzen
//  s_loaded,               // Mit Daten gef�llt: bei Jobs die Task-Warteschlange, bei Jobketten die Auftragswarteschlangen. Viele Objekte brauchen das nicht.
//  s_incomplete,           // Requisite fehlt (beim Versuch zu aktivieren)
//  s_active,               // kann wieder zu s_incomplete wechseln
//  s_closed                // Hier gibt's kein Zur�ck. Es sollte gleich darauf zerst�rt werden.
//
//  Zu jedem file_based<> gibt es ein typed_folder<> und ein file_based_subsystem<>.
//
//  F�r Dependant und Schedule_use brauchen wir kein typed_folder() und statt einem Subsystem 
//  w�rde ein Object_descriptor oder so etwas einfacheres gen�gen. Der lieferte den XML-Namen, dom_element() usw.
//
//  Vielleicht kann jedes Scheduler_object in einem generischen Zustand sein?
//
//
//
//  SPEZIELLE ZUST�NDE (Nebenzust�nde)
//
//  Manche Objekte brauchen weitere Zust�nde:
//
//  Job:        s_stopping, s_stopped, s_error,
//              s_pending und s_running ist vielleicht unerheblich und k�nnten wie das generische s_active sein.
//  Job_chain:  s_stopped
//  Scheduler:  s_stopped, s_loading, s_starting, s_waiting_for_activation, s_paused, s_stopping, s_stopping_let_run
//  Task:       zahlose Zust�nde
//
//  s_paused (s_halted, s_stopped) k�nnte verallgemeinert werden. 
//  Eigentlich kein eigener Zustand, sondern eine Eigenschaft parallel zu den generischen Zustand
//  (auch ein nicht initialisiertes Objekt k�nnte angehalten werden. Lediglich die Aktvierung soll verhindert werden).
//
//
//
//  REQUISITEN
//
//  Dependants k�nnen Requisiten (Requisite_path) haben.
//  Ein Requisit wird von einem Dependant zum Betrieb (s_active) gebraucht.
//  Wenn ein Requisit fehlt, sollte der Zustand s_incomplete statt s_active sein (das ist neu und nicht f�r alle realisiert).
//  
//  Wenn ein Requisite geladen ist (s_loaded), kann ein Dependant es benutzen: on_requisite_loaded().
//  Die Standard-Implementierung sollte den Dependant aktivieren (activate()). Das ist jetzt leider nicht m�glich, weil Dependant die Zust�nde nicht kennt.
//
//  add_requisite() sollte vielleicht erst von on_load() gerufen werden.
//  Nur Objekte im Zustand s_loaded oder weiter sollten Requisiten haben.
//
//  Es sollte registriert sein, ob ein Objekt alle Requisiten hat.
//  Wenn eine fehlt, sollte das Objekt im Zustand s_incomplete und nicht aktivierbar sein.
//  Sobald alle Requisiten da sind, sollte das Objekt automatisch aktiviert werden.
//
//
//  Eine Requisite fehlt nicht, wenn sie mindestens im Zustand s_loaded ist.
//  Damit w�ren aber indirekte Abh�ngigkeiten nicht erkennbar (denn eine Requisite in s_incomplete gilt jetzt als vorhanden).
//  Das jetzt vielleicht nicht ganz sauber codiert und k�nnte gekl�rt, vielleicht verallgemeinert werden.
//
//  Oder so: Der Scheduler k�nnte die Aktivierungen so ordnen, dass erst die Requisiten dann die Depandents aktiviert werden.
//  Wir brauchen also die transitive H�lle.
//
//
//
//  USE
//
//  Wenn ein Objekt (Dependant) ein anderes Objekt (die Requisite) braucht, kann dazwischen ein Gebrauchobjekt sein, der Use.
//  Z.B. lock::Use und Schedule_use. 
//
//  M�glichweise l�sst sich das verallgemeinern:
//      Job_chain_node ist ein Use
//
//  Dependant hat mehrere Requisiten
//  Use hat genau eine Requisite
//  Also: Dependant hat mehrere Use, haben je eine Requiste. Oder so. Noch unklar.
//
//
//
//  INLAY
//
//  Damit Objekte austauschbar sind, ohne das Dependant davon gro� Notiz nehmen m�ssen (also den Verweis erneuern),
//  k�nnen Objekte ein austauschbares Inlay habe. 
//  Bei �nderung des Objekts bleibt das Objekt erhalten, aber das Inlay wird ausgetauscht: Schedule::Inlay.
//
//  Das k�nnte ein allgemeines Verfahren f�r das Ersetzen eines Objekts sein.
//      replace_with()
//      prepare_replacement(),
//      on_replace_now(): Tauscht nur Inlay aus, statt bisher remove_file_based(), add_file_based().
//
//  set_inlay() verallgemeinern?
//
//  Vielleicht ein neuer Aufruf: on_requisite_replaced()
//
//
//
//  FEHLER-ZUST�NDE
//
//  _base_file_xc ist f�r Fehler beim Laden der Datei (inkl. set_dom()).
//  Die Objekte haben noch ihre eigenen Fehler, was verallgemeinert werden k�nnte.
//  Ein Objekt kann immer nur einen Fehler haben?
//  Ein fehlerhaftes Objekt kann nicht aktiv sein. Was ist es dann? Neuer Zustand s_error?
//  Was sind das �berhaupt f�r Fehler? Unerwartete Exception? Nur bei Job?
//  Wenn kein Fehler vorliegt, aber eine Requisite fehlt, liefert die Fehler-Methde die fehlenden Requisiten.
//
//  Ein XML-Kommando k�nnte die fehlerhaften, unvollst�ndigen, nicht aktiven Objekte liefern.
//
//
//
//  HTML-OBERFL�CHE
//
//  Wenn das alles so h�bsch verallgemeinert sein sollte,
//  kann auch die HTML-Oberfl�che verallgemeinert werden:
//  Zu jedem Objekt k�nnten die fehlenden Requisiten, Fehler, generischer Zustand usw. gezeigt werden.
//
//

//--------------------------------------------------------------------------------------------const

const char                      folder_separator            = '/';
const char                      folder_name_separator       = ',';                                  // F�r mehrteilige Dateinamen: "jobchain,orderid.order.xml"

//-------------------------------------------------------------------------------------------------

} //namespace folder
} //namespace scheduler
} //namespace sos
