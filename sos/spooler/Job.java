// $Id: Job.java,v 1.6 2004/07/12 17:59:49 jz Exp $

package sos.spooler;

/**
 * 
 * @author Joacim Zschimmer
 * @version $Revision: 1.6 $
 */

public class Job extends Idispatch
{
    private                 Job                 ( long idispatch )                  { super(idispatch); }
    
    /**
     * Erzeugt eine neue Task und reiht sie in die Task-Warteschlange ein.
     */
    
    public Task             start               ()                                  { return (Task)       com_call( "start"                         ); }
    
    
    /**
     * Erzeugt eine neue Task mit Parametern und reiht sie in die Task-Warteschlange des Jobs ein.
     * 
     * <p>
     * Die Parameter stehen der Task mit {@link Task#params()} zur Verf�gung.
     * <p>
     * Zwei besondere Parameter k�nnen angegeben werden:
     * <p>
     * "spooler_task_name": Gibt der Task einen Namen, der in den Statusanzeigen erscheint.
     * <p>
     * "spooler_start_after": Gibt ein Zeit in Sekunden (reelle Zahl) an, nach dessen Ablauf die Task zu starten ist. 
     * Dabei wird die Konfiguration von &lt;run_time> au�er Kraft gesetzt.
     */
    
    public Task             start               ( Variable_set variables )          { return (Task)       com_call( "start", variables              ); }

    
    
    /**
     * Startet eine Task des Jobs gem�� &lt;run_time>, wenn nicht schon eine l�uft und &lt;run_time> dies zul�sst.
     */
    
    
    public void             wake                ()                                  {                     com_call( "wake"                          ); }
    
    
    
    /**
     * L�sst eine Task starten, sobald sich ein Verzeichnis �ndert.
     * 
     * <p>
     * Wenn keine Task des Jobs l�uft und sich das Verzeichnis ge�ndert hat 
     * (eine Datei hinzukommt, umbenannt oder entfernt wird), 
     * startet der Scheduler innerhalb der &lt;run_time> eine Task.
     * <p>
     * Der Aufruf kann f�r verschiedene und gleiche Verzeichnisse wiederholt werden.
     * 
     * @param directory_name
     */
    
    public void             start_when_directory_changed( String directory_name )                           { com_call( "start_when_directory_changed", directory_name ); }

    
    /**
     * L�sst eine Task starten, sobald sich ein Verzeichnis �ndert, mit Angabe eines Regul�ren Ausdrucks.
     * 
     * <p>
     * Wie {@link #start_when_directory_changed(String)}, mit der Einschr�nkung, dass nur Dateien beachtet werden, 
     * deren Name dem angegebenen Regul�ren Ausdruck entsprechen. 
     * 
     * @param directory_name
     * @param filename_pattern
     */
    
    public void             start_when_directory_changed( String directory_name, String filename_pattern )  { com_call( "start_when_directory_changed", directory_name, filename_pattern ); }

    
    /**
     * Nimmt alle Aufrufe von start_when_directory_changed() zur�ck.
     *
     */
    public void             clear_when_directory_changed()                          {                     com_call( "clear_when_directory_changed"  ); }

    
    
  //public Thread           thread              ()                                  { return (Thread)     com_call( "<thread"                       ); }
    
    
    
    /**
     * Dasselbe wie spooler().include_path().
     * 
     * @see Spooler#include_path()
     */
    
    public String           include_path        ()                                  { return (String)     com_call( "<include_path"                 ); }
    
    
    
    /**
     * Liefert den Jobnamen. 
     * @return Der Name des Jobs.
     */
    
    public String           name                ()                                  { return (String)     com_call( "<name"                         ); }
    
    
    
    /**
     * Setzt f�r die Status-Anzeige einen Text.
     * @param line Eine Textzeile
     */
    
    public void         set_state_text          ( String line )                     {                     com_call( ">state_text", line             ); }
    
    
    
    /**
     * Liefert den in der Konfiguration eingestellten Titel des Jobs.
     * <p>
     * Aus &lt;job title="...">
     * 
     */
    public String           title               ()                                  { return (String)     com_call( "<title"                        ); }

    
    /**
     * Liefert die Auftragswarteschlange des Jobs oder null.
     * @return Die {@link Order_queue} oder null, wenn der Job nicht auftragsgesteuert ist (&lt;job order="no">).
     */
    
    public Order_queue      order_queue         ()                                  { return (Order_queue)com_call( "<order_queue"                  ); }

    
    /** Stellt die Fehlertoleranz ein.
     * <p>
     * F�r verschiedene Anzahlen aufeinanderfolgender Fehler kann eine Verz�gerung eingestellt werden.
     * Der Job wird dann nicht gestoppt, sondern die angegebene Zeit verz�gert und erneut gestartet.
     * <p>
     * Der Aufruf kann f�r verschiedene Anzahlen wiederholt werden. 
     * Man wird jeweils eine l�ngere Verz�gerung angeben.
     * <p>
     * Beispiel siehe {@link #set_delay_after_error(int,String)}
     * 
     * @param error_steps Anzahl der aufeinanderfolgenden Fehler, ab der die Verz�gerung gilt
     * @param seconds Verz�gerung als reele Zahl
     */
    
    public void         set_delay_after_error   ( int error_steps, double seconds ) {                     com_call( ">delay_after_error", new Integer(error_steps), new Double(seconds)   ); }
    
    
    /**
     * Wie {@link #set_delay_after_error(int,double)}, "HH:MM:SS" und "STOP" k�nnen angegeben werden.
     * 
     * <p>
     * Die Verz�gerung kann als String "HH:MM:SS" oder "HH:MM:SS" (Stunde, Minute, Sekunde) eingestellt werden.
     * <p>
     * Statt einer Zeit kann auch "STOP" angegeben werden. 
     * Wenn der Job die angegebene Anzahl aufeinanderfolgende Fehler erreicht hat,
     * stoppt der Scheduler den Job.
     * <p>
     * Eine gute Stelle f�r die Aufrufe ist {@link Job_impl#spooler_init()}.
     * <p>
     * Beispiel:
     * <pre>
     *      spooler_job.set_delay_after_error(  2,  10 );
     *      spooler_job.set_delay_after_error(  5, "01:00" );
     *      spooler_job.set_delay_after_error( 10, "24:00" );
     *      spooler_job.set_delay_after_error( 20, "STOP" );
     * </pre>
     * Nach einem Fehler wiederholt der Scheduler den Job sofort.<br/>
     * Nach dem zweiten bis zum vierten Fehler verz�gert der Scheduler den Job um 10 Sekunden,<br/>
     * nach dem f�nften bis zum neunten Fehler um eine Minute,
     * nach dem zehnten bis zum neunzehnten um 24 Stunden,
     * nach dem zwanzigsten aufeinanderfolgenden Fehler schlie�lich stoppt der Job.
     *  
     * @param error_steps
     * @param hhmm_ss
     */
    
    public void         set_delay_after_error   ( int error_steps, String hhmm_ss ) {                     com_call( ">delay_after_error", new Integer(error_steps), hhmm_ss   ); }
    
    
    /**
     * Nimmt alle Aufrufe von set_delay_after_error() zur�ck. 
     */
    public void             clear_delay_after_error()                               {                     com_call( "clear_delay_after_error"       ); }
}
