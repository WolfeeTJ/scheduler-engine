package com.sos.scheduler.engine.kernel.main;

import java.io.File;

import javax.annotation.Nullable;

import com.sos.scheduler.engine.kernel.Scheduler;
import com.sos.scheduler.engine.kernel.schedulerevent.SchedulerCloseEvent;
import com.sos.scheduler.engine.kernel.SchedulerException;
import com.sos.scheduler.engine.kernel.event.Event;
import com.sos.scheduler.engine.kernel.event.EventSubscriber;
import com.sos.scheduler.engine.kernel.main.event.SchedulerReadyEvent;
import com.sos.scheduler.engine.kernel.main.event.TerminatedEvent;
import com.sos.scheduler.engine.kernel.util.Time;
import com.sos.scheduler.engine.kernel.util.sync.ThrowableMailbox;

/** Steuert den {@link SchedulerThread}. */
public class SchedulerThreadController implements SchedulerController {
    //private static final Time terminationTimeout = Time.of(10);
    private final ThrowableMailbox<Throwable> throwableMailbox = new ThrowableMailbox<Throwable>();
    private final SchedulerThread thread;
    private final StateThreadBridge stateThreadBridge = new StateThreadBridge();
    private EventSubscriber eventSubscriber = EventSubscriber.empty;

    public SchedulerThreadController() {
        thread =  new SchedulerThread(new MyStateHandler());
    }

    @Override public final void subscribeEvents(EventSubscriber s) {
        assert s != null;
        eventSubscriber = s;
    }

    public final void loadModule(File cppModuleFile) {
        thread.loadModule(cppModuleFile);
    }

    @Override public final void startScheduler(String... args) {
        thread.startThread(args);
    }

    @Override public final Scheduler waitUntilSchedulerIsRunning() {
        try {
            Scheduler result = stateThreadBridge.waitWhileSchedulerIsStarting();
            throwableMailbox.throwUncheckedIfSet();
            if (result == null) {
                throw new SchedulerException("Scheduler aborted before startup");
            }
            return result;
        }
        catch (InterruptedException x) { throw new RuntimeException(x); }
    }

    @Override public final void terminateAndWait() {
        if (thread.isAlive()) {
            terminateScheduler();
            waitForTermination(Time.eternal);
        }
    }

    @Override public final void waitForTermination(Time timeout) {
        try {
            if (timeout == Time.eternal)  thread.join();
            else timeout.unit.timedJoin(thread, timeout.value);
            throwableMailbox.throwUncheckedIfSet();
        }
        catch (InterruptedException x) { throw new RuntimeException(x); }
    }

    @Override public final void terminateAfterException(Throwable x) {
        throwableMailbox.setIfFirst(x);
        terminateScheduler();
    }

    @Override public final void terminateScheduler() {
        stateThreadBridge.terminate();
    }

    @Override public final int exitCode() {
        return thread.exitCode();
    }

    private class MyStateHandler implements SchedulerStateHandler {
        private final MyEventSubscriber myEventSubscriber = new MyEventSubscriber();
        private Scheduler scheduler = null;

        @Override public final void onSchedulerStarted(Scheduler s) {
            this.scheduler = s;
            stateThreadBridge.onSchedulerStarted(scheduler);
            reportStrictlyEvent(new SchedulerReadyEvent(SchedulerThreadController.this));
        }

        @Override public final void onSchedulerActivated() {
            //stateThreadBridge.onSchedulerActivated(scheduler);
            scheduler.getEventSubsystem().subscribe(myEventSubscriber);
        }

        @Override public void onSchedulerTerminated(int exitCode, @Nullable Throwable t) {
            if (t != null) throwableMailbox.setIfFirst(t);
            stateThreadBridge.onSchedulerTerminated();
            reportStrictlyEvent(new TerminatedEvent(exitCode, t));
        }

        class MyEventSubscriber implements EventSubscriber {
            @Override public final void onEvent(Event e) throws Exception {
                if (e instanceof SchedulerCloseEvent)
                    stateThreadBridge.onSchedulerClosed();
                eventSubscriber.onEvent(e);
            }
        }
    }

    void reportStrictlyEvent(Event e) {
        try { 
            eventSubscriber.onEvent(e);
        }
        catch (Throwable x) {
            //boolean debugOnly = e instanceof TerminatedEvent  &&  x instanceof UnexpectedTerminatedEventException;
            throwableMailbox.setIfFirst(x); //, debugOnly? Level.DEBUG : Level.ERROR);
        }
    }
}
