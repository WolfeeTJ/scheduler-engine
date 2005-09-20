// $Id$

package sos.spooler;

/*+ Eine Task ist eine laufende Instanz eines Jobs.
 * 
 * <p>
 * Eine Task kann wartend in der Task-Warteschlange des Jobs sein oder laufen.
 * <p>
 * Implementiert wird eine Task durch {@link Job_impl}.
 * 
 * @author Joacim Zschimmer
 * @version $Revision$
 */

public class Task extends Idispatch
{
    private                 Task                ( long idispatch )                  { super(idispatch); }
    
  //public Object_set       object_set

    
    /*+ Liefert den {@link Job}, zu der die Task geh�rt. */
    public Job              job                 ()                                  { return (Job)          com_call( "<job"                            ); }
    
    

    /*+ Liefert die Kennung der Task. */
    public int              id                  ()                                  { return            int_com_call( "<id"                             ); }
    
    /*+ Liefert die Parameter der Task.
     * 
     * <p>
     * Eine Task kann Parameter haben. Die Parameter k�nnen in der Konfiguration mit &lt;parameter>, 
     * mit {@link Job#start(Variable_set)} oder mit &lt;start_job> gesetzt werden. 
     */
    public Variable_set     params              ()                                  { return (Variable_set) com_call( "<params"                         ); }
    
    
    /*+ Hier kann die Task ein Ergebnis speichern, das von einer anderen, solange wartenden Task
     * abgeholt werden kann.
     * 
     * <p>
     * Das Ergebnis kann mit {@link #result()} abgeholt werden.
     * @param value
     * @see Job#start(Variable_set)
     */
    public void         set_result              ( String value )                    {                       com_call( ">result", value                  ); }
    
    
    
    /*+ Liefert das mit set_result() gesetzte Ergebnis. */
    public String           result              ()                                  { return (String)       com_call( "<result"                         ); }
    
    
    
    /*+ Startet erneut eine Task nach der eingestellten Zeit.
     * 
     * <p>
     * Diese Methode geh�rt eigentlich in die Klasse Job. Sie hat nichts mit der gerade laufenden Task zu tun.
     * <p>
     * Wenn nach Ablauf der angegebenen Zeit keine Task des Jobs l�uft, startet der Scheduler eine Task.
     * <p>
     * Die Einstellung wirkt nicht, wenn der Job einen Fehler geliefert hat und
     * {@link sos.spooler.Job#set_delay_after_error(int,double)} aufgerufen worden ist.
     *  
     * @param seconds
     */
    public void         set_repeat              ( double seconds )                  {                       com_call( ">repeat", seconds                ); }
    
    
    
    /*+ Veranlasst den Scheduler, nicht mehr spooler_process() zu rufen.
     * 
     * <p>
     * Der n�chste Aufruf wird spooler_close() sein.
     */
    public void             end                 ()                                  {                       com_call( "end"                             ); }
    
    
    
    /*+ Setzt einen Fehler und stoppt den Job.
     *
     * @param text Fehlertext
     */ 
    public void         set_error               ( String text )                     {                       com_call( ">error", text                    ); }
    
    
    
    /*+ Liefert den Fehler der Task als {@link Error}.
     * 
     * <p>
     * Wenn kein Fehler vorliegt, wird ein Error-Objekt zur�ckgeliefert, dessen Aufruf is_error false liefert.
     */
    public Error            error               ()                                  { return (Error)        com_call( "<error"                          ); }

    
    
    /*+ Wartet aufs Ende einer anderen Task.
     * 
     * <p>
     * In Kombination mit {@link Job#start()} kann eine Task aufs Ende einer anderen warten.
     * <p>
     * Wenn die andere Task nicht starten kann, weil nicht gen�gend Ressourcen bereitstehen, kann dieser Aufruf
     * zur Verklemmung f�hren.
     */
    public boolean          wait_until_terminated()                                 { return        boolean_com_call( "wait_until_terminated"           ); }
    
    
    
    /*+ Wie {@link #wait_until_terminated()}, mit Begrenzung der Wartezeit. 
     * 
     * @return true, wenn Task geendet; false, wenn Zeit abgelaufen. */
    public boolean          wait_until_terminated( double wait_seconds )            { return ( (Boolean)    com_call( "wait_until_terminated", new Double(wait_seconds) ) ).booleanValue(); }
    
    
    
  //public Thread           thread              ()                                  { return (Thread)       com_call( "<thread"                         ); }

    
    
    /*+ Setzt ein Feld in der Task-Historie.
     * 
     * <p>
     * Die Datenbanktabelle muss eine Spalte mit dem Namen haben
     * und diese Spalte muss in der Datei factory.ini mit history_columns deklariert sein.
     */ 
    public void         set_history_field       ( String name, String value )       {                       com_call( ">history_field", name, value     ); }
    
    
    
    /*+ Verz�gert den n�chsten Aufruf von spooler_process().
     */
    public void         set_delay_spooler_process( double seconds )                 {                       com_call( ">delay_spooler_process", seconds ); }

    
    /*+ Verz�gert den n�chsten Aufruf von spooler_process().
     * 
     * @param hhmm_ss "HH:MM:SS" oder "HH:MM", die Dauer in Stunde, Minute, Sekunde.
     */
    public void         set_delay_spooler_process( String hhmm_ss )                 {                       com_call( ">delay_spooler_process", hhmm_ss ); }

    

    /*+ @deprecated Die Methode galt f�r use_engine="job", was es nicht mehr gibt. */    
    public void         set_close_engine        ( boolean close_after_task )        {                       com_call( ">close_engine", close_after_task ); }

    

    /*+ Liefert den zu verarbeitenden Auftrag oder null.
     */
    public Order            order               ()                                  { return (Order)        com_call( "<order"                          ); }

    
    
    /*+ Liefert die Verzeichnisse, deren �nderung den Start der Task veranlasst haben.
     * 
     * Mehrere Verzeichnisnamen sind durch Semikolon getrennt.
     * 
     * @see Job#start_when_directory_changed(String)
     * @see Job#start_when_directory_changed(String,String)
     */
    public String           changed_directories ()                                  { return (String)       com_call( "<changed_directories"            ); }

    
    /*+ Macht dem Scheduler einen abh�ngigen Prozess bekannt.
     * 
     * <p>
     * Wenn die Task endet, bricht der Scheduler die evtl. noch laufenden Prozesse ab.
     * <p>
     * Der Aufruf kann f�r mehrere Prozesse wiederholt werden.
     *  
     * @param pid Die Prozess-Id
     */
    public void             add_pid             ( int pid )                         {                       com_call( "add_pid", pid                    ); }
    
    

    /*+ Macht dem Scheduler einen abh�ngigen, befristeten Prozess bekannt.
     * 
     * <p>
     * Wie {@link #add_pid(int)}, mit dem Zusatz, dass der Scheduler den Prozess nach der
     * angegebenen Zeit abbricht, sollte er noch laufen.
     * <p>
     * Mit dem Aufruf kann die Laufzeit von abh�ngigen Prozessen eingegeschr�nkt werden.
     * Bei Task-Ende bricht der Scheduler noch laufende abh�ngigen Prozesse in jeden Fall ab.
     * <p>
     * Wenn der Scheduler einen Prozess abbricht, gibt es einen Eintrag ins Protokoll.
     * Die Task erf�hrt davon nichts.  
     *  
     * @param pid Die Prozess-Id
     * @param timeout_seconds Die Frist f�r den Prozess
     */
    public void             add_pid             ( int pid, double timeout_seconds ) {                       com_call( "add_pid", new Integer(pid), new Double(timeout_seconds) ); }

    
    
    /*+ Nimmt ein add_pid() zur�ck.
     * 
     * <p>
     * Wenn die Pid nicht bekannt ist, gibt es keinen Fehler.
     * 
     * @param pid Die Prozess-Id
     * @see #add_pid(int)
     * @see #add_pid(int,double)
     */
    public void             remove_pid          ( int pid )                         {                       com_call( "remove_pid", pid                 ); }

    
    
    /*+ Der bisher nach stdout geschriebene Text.
     * 
     * <p>
     * Nur, wenn die Task in einem separaten Prozess (&lt;process_classes/>) l�uft. Sonst "".
     * 
     * @see #stderr_text()
     */
    public String           stdout_text         ()                                  { return (String)       com_call( "<stdout_text"                    ); }


    /*+ Pfadname f�r stdout.
     * 
     * <p>
     * Nur, wenn die Task in einem separaten Prozess (&lt;process_classes/>) l�uft. Sonst "".
     * 
     * @see #stdout_text()
     */
    public String           stdout_path         ()                                  { return (String)       com_call( "<stdout_path"                    ); }

    
    
    /*+ Der bisher nach stderr geschriebene Text.
     * 
     * <p>
     * Nur, wenn die Task in einem separaten Prozess (&lt;process_classes/>) l�uft. Sonst "".
     * 
     * @see #stderr_text()
     */
    public String           stderr_text         ()                                  { return (String)       com_call( "<stderr_text"                    ); }
    
    /*+ Pfadname f�r stderr.
     * 
     * <p>
     * Nur, wenn die Task in einem separaten Prozess (&lt;process_classes/>) l�uft. Sonst "".
     * 
     * @see #stderr_text()
     */
    public String           stderr_path         ()                                  { return (String)       com_call( "<stderr_path"                    ); }
    
    public Subprocess       create_subprocess   ()                                  { return (Subprocess)   com_call( "create_subprocess" ); }

    public Subprocess       create_subprocess   ( String command_line )             { return (Subprocess)   com_call( "create_subprocess", command_line ); }

    public Subprocess       create_subprocess   ( String filename_and_arguments[] ) { return (Subprocess)   com_call( "create_subprocess", filename_and_arguments ); }
}
