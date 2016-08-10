package com.sos.scheduler.engine.tests.jira.js628;

import com.sos.scheduler.engine.eventbus.HotEventHandler;
import com.sos.scheduler.engine.data.order.OrderFinished;
import com.sos.scheduler.engine.kernel.order.UnmodifiableOrder;
import com.sos.scheduler.engine.test.SchedulerTest;
import com.sos.scheduler.engine.test.util.CommandBuilder;
import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * js-628: Order successfull when a pre/postprocessing script is used
 *
 * This test contain four jobchains for various combinations of the result from spooler_process and
 * spooler_process_before:
 *
 * spooler_process      spooler_process_before      result              job_chain
 * ==================   ========================    =================   =====================
 * true                 false                       error               js628_chain_fail_1
 * false                false                       error               js628_chain_fail_2
 * false                true                        error               js628_chain_fail_3
 * true                 true                        success             js628_chain_success
 *
 * It should be clarify that the job ends only if the tesult of spooler_process AND spooler_process_before
 * is true.
 *
 * The test estimate that one job_chain ends with success and three job_chains ends with a failure.
 */
public class JS628IT extends SchedulerTest {

    private static final String[] JOB_CHAINS =
            {
                    "js628-chain-success-1",
                    "js628-chain-success-2",
                    "js628-chain-fail-1",
                    "js628-chain-fail-2",
                    "js628-chain-fail-3",
                    "js628-chain-fail-4"
            };

    private static final int expectedErrorCount = 4;
    private static final int expectedSuccessCount = 2;

    private int finishedOrderCount = 0;
    private int errorCount = 0;
    private int successCount = 0;

    @Test
    public void test() throws Exception {
        CommandBuilder commandBuilder = new CommandBuilder();
        controller().activateScheduler();
        for(String jobChain : JOB_CHAINS) {
        String cmd = commandBuilder.addOrder(jobChain).getCommand();
            controller().scheduler().executeXml(cmd);
        }
        controller().waitForTermination();
        assertEquals("total number of events", JOB_CHAINS.length, finishedOrderCount);
        assertEquals("successfull orders", expectedSuccessCount, successCount);
        assertEquals("unsuccessfull orders", expectedErrorCount, errorCount);
    }

    @HotEventHandler
    public void handleEvent(OrderFinished e, UnmodifiableOrder order) throws InterruptedException {
        String endState = order.nodeId().string();
        if (endState.equals("error")) errorCount++;
        if (endState.equals("success")) successCount++;
        finishedOrderCount++;
        if (finishedOrderCount == JOB_CHAINS.length)
            controller().scheduler().terminate();
    }

}
