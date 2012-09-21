package com.sos.scheduler.engine.test;

import com.sos.scheduler.engine.common.time.Time;

class SchedulerRunningAfterTimeoutException extends RuntimeException {
    SchedulerRunningAfterTimeoutException(Time timeout) {
        super("Scheduler has not been terminated within "+timeout);
    }
}
