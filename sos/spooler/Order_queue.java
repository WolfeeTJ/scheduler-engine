// $Id: Order_queue.java,v 1.4 2004/07/12 17:59:49 jz Exp $

package sos.spooler;

/**
 * Auftragswarteschlange eines auftragsgesteuerten Jobs.
 * 
 * <p>
 * Ein auftragsgesteuerter Job (&lt;job order="yes"> hat eine Auftragswarteschlange, die die vom Job zu verarbeitenden Jobs
 * aufnimmt.
 * Die Auftr�ge sind nach Priorit�t und Zeitpunkt des Eintreffens geordnet.
 * <p>
 * Verarbeiten bedeutet, dass der Scheduler die Methode {@link Job_impl#spooler_process()} einer Task des Jobs aufruft.
 * Die Methode kann �ber spooler_task.order() auf den Auftrag zugreifen.
 * Endet spooler_process() ohne Fehler (ohne Exception), entfernt der Scheduler den Auftrag aus der Auftragswarteschlange.
 * Ist der Auftrag in einer Jobkette, dann r�ckt der r�ckt der Auftrag an die n�chste Position der Jobkette.  
 * 
 * @see Job_chain
 * 
 * @author Joacim Zschimmer
 * @version $Revision: 1.4 $
 */

public class Order_queue extends Idispatch
{
    private                 Order_queue         ( long idispatch )                  { super(idispatch); }

    /** Liefert die Anzahl der Auftr�ge in der Auftragswarteschlange. */
    public int              length              ()                                  { return        int_com_call( "<length"             ); }
    
    
    
    /** F�gt einen Auftrag der Auftragswarteschlange hinzu.
     * <p>
     * Der Aufruf gilt nur f�r den Fall, dass der Auftrag nicht in einer Jobkette enthalten ist.
     * 
     * <p>
     * Die Priorit�t des Auftrags (@link Order#set_priority(int)}) wird dabei ber�cksichtigt.
     * <p>
     * 
     * @param order Der Auftrag
     * @see Job_chain#add_order(Order)
     */ 
    public Order            add_order           ( Order order )                     { return (Order)    com_call( "add_order", order    ); }
}
