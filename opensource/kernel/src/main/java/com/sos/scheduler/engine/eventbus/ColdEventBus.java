package com.sos.scheduler.engine.eventbus;

import com.sos.scheduler.engine.eventbus.annotated.ColdMethodEventSubscriptionFactory;
import com.sos.scheduler.engine.data.event.Event;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Collection;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

public class ColdEventBus extends AbstractEventBus {
    private static final Logger logger = LoggerFactory.getLogger(ColdEventBus.class);

    private final Queue<Call> callQueue = new ConcurrentLinkedQueue<Call>();

    public ColdEventBus() {
        super(ColdMethodEventSubscriptionFactory.singleton);
    }

    @Override public final void publish(Event e) {
        publish(calls(e));
    }

    final void publish(Collection<Call> c) {
        callQueue.addAll(c);
    }

    public final void dispatchEvents() {
        while (true) {
            Call call = callQueue.poll();
            if (call == null) break;
            logger.trace("dispatch "+call);
            dispatchCall(call);
        }
    }
}
