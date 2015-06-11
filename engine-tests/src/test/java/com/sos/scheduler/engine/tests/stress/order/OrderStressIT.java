package com.sos.scheduler.engine.tests.stress.order;

import com.sos.scheduler.engine.data.order.OrderTouchedEvent;
import com.sos.scheduler.engine.eventbus.EventHandler;
import com.sos.scheduler.engine.test.SchedulerTest;
import java.time.Duration;
import org.junit.Test;

public final class OrderStressIT extends SchedulerTest {
    // In Maven setzen mit -DargLine=-DOrderStressTest.limit=26 (Surefire plugin 2.6), 2010-11-28
    // Zum Beispiel: mvn test -Dtest=ExtractResourcesTest -DargLine=-DOrderStressTest.limit=26
    private static final int testLimit = Integer.parseInt(System.getProperty("ExtractResourcesTest.limit", "100"));
    private int touchedOrderCount = 0;

    @Test public void test() throws Exception {
        controller().activateScheduler();
        controller().waitForTermination(Duration.ofHours(1));
    }
    
    @EventHandler public void handleEvent(OrderTouchedEvent e) {
        // OrderFinishedEvent wird nicht ausgelöst, weil der Auftrag vorher mit add_or_replace() ersetzt wird.
        touchedOrderCount++;
        if (touchedOrderCount > testLimit)
            controller().terminateScheduler();
    }

//    public void main(String[] args) throws Exception {
//        int limit = Integer.parseInt(args[0]);
//        new ExtractResourcesTest().runTest(limit);
//    }
}
