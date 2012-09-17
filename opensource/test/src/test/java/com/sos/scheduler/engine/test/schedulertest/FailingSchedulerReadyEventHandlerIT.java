package com.sos.scheduler.engine.test.schedulertest;

import org.junit.Test;

import com.sos.scheduler.engine.eventbus.HotEventHandler;
import com.sos.scheduler.engine.main.event.SchedulerReadyEvent;
import com.sos.scheduler.engine.test.SchedulerTest;

/** Testet {@link com.sos.scheduler.engine.test.SchedulerTest} */
public final class FailingSchedulerReadyEventHandlerIT extends SchedulerTest {
    @Test(expected=TestError.class) public void activateTest() {
        controller().activateScheduler();
        controller().close();
    }

    @Test(expected=TestError.class) public void startTest() {
        controller().startScheduler();
        controller().close();
    }

    @HotEventHandler public void handleEvent(SchedulerReadyEvent e) {
        throw new TestError();
    }
}
