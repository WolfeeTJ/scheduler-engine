// $Id: Variable_set.java,v 1.7 2004/07/13 14:22:21 jz Exp $

package sos.spooler;

/**
 * Variablenmenge.
 * <p> 
 * Variablenmengen werden gebraucht f�r die scheduler-weiten Variablen und Task-Parameter.
 * Eine neue Variablenmenge wird mit {@link Spooler.create_variable_set()} angelegt.
 * <p>
 * Die Gro�schreibung der Variablennamen ist relevant.
 * <p>
 * In Java ist ein Variablenwert ein String.
 * <p>
 * In COM (JavaScript, VBScript, Perl) ist ein Variablenwert ein Variant. 
 * Weil die Variablen in der Regel in die Scheduler-Datenbank geschrieben werden, sollten nur
 * nach String konvertierbare Variant-Werte verwendet werden (d.h. es sollten keine Objekte verwendet werden).  
 *  
 *
 * @see Spooler#variables()
 * @see Task#params()
 * @see Spooler#create_variable_set()
 * @author Joacim Zschimmer
 * @version $Revision: 1.7 $
 */

public class Variable_set extends Idispatch
{
    private                 Variable_set        ( long idispatch )                  { super(idispatch); }

    /** Setzt eine Variable.
     * 
     * <p><br/><b>Beispiel</b>
     * <pre>
     *     Variable_set variable_set = spooler.create_variable_set();                                                  
     *     variable_set.set_var( "nachname", "M�ller" );
     * </pre>
     *
     * <p><br/><b>Beispiel in JavaScript</b>
     * <pre>
     *     var variable_set = spooler.create_variable_set();                                                  
     *     variable_set( "nachname" ) = "M�ller";
     * </pre>
     * 
     * 
     */ 
    public void         set_var                 ( String name, String value )       {                       com_call( ">var", name, value       ); }
    

    
    /** Liefert den Wert einer Variablen.
     * 
     * <p><br/><b>Beispiel</b>
     * <pre>
     *     spooler_log.debug( "nachname=" + spooler_task.params().var( "nachname" ) );                                                  
     * </pre>
     *
     * <p><br/><b>Beispiel in JavaScript</b>
     * <pre>
     *     spooler_log.debug( "nachname=" + spooler_task.params( "nachname" ) );                                                  
     * </pre>
     * 
     * @param name
     * @return Wenn die Variable nicht bekannt ist, wird "" (bei COM: ein Empty) zur�ckgegeben.
     */
    public String           var                 ( String name )                     { return (String)       com_call( "<var", name              ); }


    /** Liefert die Anzahl der Variablen.
     * <p>
     * Es gibt keine M�glichkeit, �ber Variablen �ber einen Index anzusprechen oder �ber sie zu iterieren. 
     * Dieser Aufruf ist also nicht so n�tzlich. 
     * 
     * <p><br/><b>Beispiel</b>
     * <pre>
     *     spooler_log.debug( "count=" + spooler_task.params().count() );                                                  
     * </pre>
     *
     * <p><br/><b>Beispiel in JavaScript</b>
     * <pre>
     *     spooler_log.debug( "count=" + spooler_task.params.count );                                                  
     * </pre>
     */

    public int              count               ()                                  { return            int_com_call( "<count"                  ); }
  
  //public Dom              dom                 ()
  
  
  //public Variable_set     clone               ()                                  { return (Variable_set) com_call( "clone"                   ); }
    
    
    /** Mischt die Variablen aus einer anderen Variablenmenge ein.
     * 
     * Bereits vorhandene Variablen werden bei gleichen Namen �berschrieben.
     * 
     */
    
    public void             merge               ( Variable_set vars )               {                       com_call( "merge", vars             ); }


    /** �bernimmt ein die Variablenmenge aus einem XML-Dokument.
     * Mit folgender DTD:
     * <p>
     * <pre>
     *     &lt;!ELEMENT variable_set ( variable* )>
     *     &lt;!ELEMENT variable EMPTY>
     *     &lt;!ATTLIST variable name CDATA #REQUIRED>
     *     &lt;!ATTLIST variable value CDATA #REQUIRED>
     * </pre>
     *
     * Die Variablen im XML-Dokument werden dem Variable_set hinzugef�gt. 
     * Vorhandene Variablen gleichen Namens werden �berschrieben.
     * 
     * <p><br/><b>Beispiel</b>
     * <pre>
     *     Variable_set variable_set = spooler.create_variable_set();
     *     String xml = "&lt;?xml version='1.0'?>&gt;variable                                                  
     *     variable_set.set_xml( xml );
     * </pre>
     *
     * <p><br/><b>Beispiel in JavaScript</b>
     * <pre>
     *     var variable_set = spooler.create_variable_set();                                                  
     *     variable_set( "name" ) = "M�ller";
     * </pre>
     */

    public void         set_xml                 ( String xml_text )                 {                       com_call( ">xml", xml_text          ); }


    /** Liefert die Variablenmenge als XML-Dokument, wie in {@link #set_xml(String)} beschrieben.
      * Das XML-Dokument kann {@link #set_xml(String)} �bergeben werden.
      */

    public String           xml                 ()                                  { return (String)       com_call( "<xml"                    ); }
}
