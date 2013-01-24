package com.sos.scheduler.engine.tests.excluded.ss.simple;

import com.sos.scheduler.engine.eventbus.HotEventHandler;
import com.sos.scheduler.engine.data.job.TaskEndedEvent;
import com.sos.scheduler.engine.data.job.TaskStartedEvent;
import com.sos.scheduler.engine.data.order.OrderFinishedEvent;
import com.sos.scheduler.engine.kernel.job.UnmodifiableTask;
import com.sos.scheduler.engine.kernel.order.UnmodifiableOrder;
import com.sos.scheduler.engine.test.SchedulerTest;
import com.sos.scheduler.engine.test.util.CommandBuilder;
import com.sos.scheduler.engine.data.event.Event;
import org.junit.BeforeClass;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;

public class SimpleTest extends SchedulerTest {

	private static final Logger logger = LoggerFactory.getLogger(SimpleTest.class);
	
	private final CommandBuilder util = new CommandBuilder();
	
	@BeforeClass
    public static void setUpBeforeClass() throws Exception {
        logger.debug("starting test for " + SimpleTest.class.getName());
	}

	@Test
	public void test() throws InterruptedException {
        controller().activateScheduler();
        util.addOrder("jobchain1");
		controller().scheduler().executeXml( util.getCommand() );
        controller().waitForTermination(shortTimeout);
	}
	
	@HotEventHandler
	public void handleEvent(Event e) throws IOException {
		logger.debug("EVENT: " + e.getClass().getSimpleName());
	}
	
	@HotEventHandler
	public void handleTaskStartedEvent(TaskStartedEvent e, UnmodifiableTask t) throws IOException {
		logger.debug("TASKEVENT: " + t.getOrderOrNull().getId().asString());
	}
	
	/**
	 * Das Objekt t.getOrder() ist hier null.
	 *
	 * @param e
	 * @param t
	 * @throws IOException
	 */
	@HotEventHandler
	public void handleTaskEndedEvent(TaskEndedEvent e, UnmodifiableTask t) throws IOException {
		logger.debug("TASKEVENT: " + t.getJob().getName());
	}
	
	@HotEventHandler
	public void handleOrderEnd(OrderFinishedEvent e, UnmodifiableOrder order) throws IOException, InterruptedException {
		logger.debug("ORDERFINISHED: " + order.getId().asString());
		if (order.getId().asString().equals("jobchain1"))
			controller().terminateScheduler();
	}
}
