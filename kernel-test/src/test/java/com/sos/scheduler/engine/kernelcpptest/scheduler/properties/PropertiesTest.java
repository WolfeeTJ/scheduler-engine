package com.sos.scheduler.engine.kernelcpptest.scheduler.properties;

import com.sos.scheduler.engine.kernel.test.SchedulerTest;
import org.apache.log4j.*;
import org.junit.*;

import static com.google.common.base.Strings.nullToEmpty;

public class PropertiesTest extends SchedulerTest {
    private static final Logger logger = Logger.getLogger(PropertiesTest.class);

    @Test public void test1() throws Exception {
        try {
            startScheduler();
            System.out.println("port=" + getScheduler().getTcpPort());
            System.out.println("host=" + getScheduler().getHostname());
            System.out.println("host_complete=" + getScheduler().getHostnameLong());
            schedulerController.terminateAndWait();
//            fail("Exception expected");
        }
        catch (Exception x) {
            if (!nullToEmpty(x.getMessage()).contains("SOS-1300"))
                throw new RuntimeException("Exception enthält nicht SOS-1300", x);
            logger.debug("OK: " + x);
        }
    }
}
