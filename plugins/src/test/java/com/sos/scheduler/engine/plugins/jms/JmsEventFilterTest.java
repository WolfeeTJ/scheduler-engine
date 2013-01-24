package com.sos.scheduler.engine.plugins.jms;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.sos.scheduler.engine.data.order.OrderFinishedEvent;
import com.sos.scheduler.engine.data.order.OrderTouchedEvent;
import com.sos.scheduler.engine.eventbus.HotEventHandler;
import com.sos.scheduler.engine.kernel.order.UnmodifiableOrder;
import com.sos.scheduler.engine.kernel.scheduler.SchedulerException;
import com.sos.scheduler.engine.test.util.CommandBuilder;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.TextMessage;
import java.io.IOException;
import java.util.List;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

import static java.util.Arrays.asList;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;


public class JmsEventFilterTest extends JMSConnection {
	
	/* start this module with -Djms.providerUrl=tcp://localhost:61616 to test with an external JMS server */
    /** Maven: mvn test -Dtest=JmsPlugInTest -DargLine=-Djms.providerUrl=tcp://localhost:61616 */
    private static final String providerUrl = System.getProperty("jms.providerUrl", ActiveMQConfiguration.vmProviderUrl);
//  private static final String providerUrl = "tcp://w2k3.sos:61616";  // in scheduler.xml einstellen
    private static final Logger logger = LoggerFactory.getLogger(JmsEventFilterTest.class);
    private static final List<String> eventsToListen = asList("OrderTouchedEvent");
    private static final String jobchain = "jmstest";

    private final CommandBuilder util = new CommandBuilder();
    private int orderFinished = 0;
    
    // Queue for collecting the fired eventsToListen in the listener thread
    private final BlockingQueue<String> resultQueue = new ArrayBlockingQueue<String>(50);
    
    // This object is needed for serializing and deserializing of the event objects
    private final ObjectMapper mapper = new ObjectMapper();

    public JmsEventFilterTest() throws Exception {
    	super(providerUrl,eventsToListen);
    	setMessageListener( new MyListener() );
    }

    @Test
    public void test() throws Exception {
    	try {
//	        controller().activateScheduler("-e -log-level=debug"));
	        controller().activateScheduler();
	        controller().scheduler().executeXml( util.addOrder(jobchain, "order1").getCommand() );
	        controller().scheduler().executeXml( util.addOrder(jobchain, "order2").getCommand() );
	        controller().waitForTermination(shortTimeout);
	        assertEquals("two eventsToListen of " + eventsToListen.get(0) + " expected",2,resultQueue.size());
	        assertTrue("'order1' is not in result queue",resultQueue.contains("order1"));
	        assertTrue("'order2' is not in result queue",resultQueue.contains("order2"));
		} finally {
			close();
		}
    }

    @HotEventHandler
    public void handleOrderEnd(OrderFinishedEvent e, UnmodifiableOrder o) throws Exception {
        logger.debug("ORDERFINISHED: " + o.getId().asString());
        orderFinished++;
        if (orderFinished == 2)
            controller().scheduler().terminate();
    }

    private class MyListener implements javax.jms.MessageListener {

        // runs in an own thread
    	@Override
        public void onMessage(Message message) {
            String result = "<unknown event>";
            String jsonContent = null;
            
            // deserialize the event from JSON into the TaskStartEvent object
            try {
                TextMessage textMessage = (TextMessage) message;
                showMessageHeader(textMessage);
                jsonContent = textMessage.getText();
                textMessage.acknowledge();
                mapper.registerSubtypes(OrderTouchedEvent.class);
                OrderTouchedEvent ev = mapper.readValue(jsonContent, OrderTouchedEvent.class);
                assertEquals(getTopicname(textMessage), "com.sos.scheduler.engine.Event" );  // Erstmal ist der Klassenname vorangestellt.
                result = ev.getKey().getId().toString();
            } catch (IOException e1) {
                String msg = "could not deserialize " + jsonContent;
                logger.error(msg);
                throw new SchedulerException(msg,e1);
            } catch (JMSException e2) {
                String msg = "error getting content from JMS.";
                logger.error(msg);
                throw new SchedulerException(msg,e2);
            } finally {
                try {
					resultQueue.put(result);
				} catch (InterruptedException e) {
					logger.error(e.getMessage());
				}
            }
        }
    }
    
}
