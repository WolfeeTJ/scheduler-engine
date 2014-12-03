package com.sos.scheduler.engine.eventbus;

import com.sos.scheduler.engine.eventbus.annotated.HotMethodEventSubscriptionFactory;
import com.sos.scheduler.engine.kernel.event.EventSubsystem;
import com.sos.scheduler.engine.data.event.Event;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.annotation.Nullable;
import java.util.Collection;

public class HotEventBus extends AbstractEventBus {
    private static final Logger logger = LoggerFactory.getLogger(HotEventBus.class);

    @Nullable private Event currentEvent = null;

    public HotEventBus() {
        super(HotMethodEventSubscriptionFactory.singleton);
    }

    @Override public final void publish(Event e) {
        publish(e, calls(e));
    }

    final void publish(Event e, Collection<Call> calls) {
        if (currentEvent != null)
            handleRecursiveEvent(e);
        else
            dispatchNonrecursiveEvent(e, calls);
    }

    //@Override public final void dispatchEvents() {}

    private void handleRecursiveEvent(Event e) {
        try {
            // Kein log().error(), sonst gibt es wieder eine Rekursion
            throw new Exception(EventSubsystem.class.getSimpleName() + ".publish("+e+"): ignoring the event triggered by handling the event '"+currentEvent+"'");
        }
        catch (Exception x) {
            logger.error("Ignored", x);
        }
    }

    private void dispatchNonrecursiveEvent(Event e, Collection<Call> calls) {
        currentEvent = e;
        try {
            for (Call c: calls)
                dispatchCall(c);
        }
        finally {
            currentEvent = null;
        }
    }
}
