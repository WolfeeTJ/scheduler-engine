package com.sos.scheduler.engine.tests.jira.js401;

import com.sos.scheduler.engine.data.event.Event;
import com.sos.scheduler.engine.data.event.KeyedEvent;
import com.sos.scheduler.engine.data.job.TaskEnded;
import com.sos.scheduler.engine.eventbus.EventHandler;
import com.sos.scheduler.engine.test.SchedulerTest;
import com.sos.scheduler.engine.test.util.CommandBuilder;
import org.junit.Test;

public final class JS401IT extends SchedulerTest {

    private int countEndedTasks = 0;

    @Test
    public void test() {
        CommandBuilder util = new CommandBuilder();
        controller().activateScheduler();
        controller().scheduler().executeXml("<add_order job_chain='chain1' id='1'/>");
        controller().scheduler().executeXml(util.startJobImmediately("job2").getCommand());
        controller().scheduler().executeXml(util.startJobImmediately("inc_max_non_exclusive").getCommand());
        controller().waitForTermination();
    }

    @EventHandler
    public void handleEvent(KeyedEvent<Event> e) {
        if (TaskEnded.class.isAssignableFrom(e.event().getClass())) {
            countEndedTasks++;
            if (countEndedTasks == 3) {
                controller().terminateScheduler();
            }
        }
    }
}
