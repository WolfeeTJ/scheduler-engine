package com.sos.scheduler.engine.kernelcpptest.jira.js628;

import static org.junit.Assert.*;
import java.util.Iterator;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

import javax.jms.JMSException;
import javax.jms.Message;
import javax.jms.Session;
import javax.jms.TextMessage;
import javax.jms.Topic;
import javax.jms.TopicConnection;
import javax.jms.TopicSession;
import javax.jms.TopicSubscriber;

import org.junit.BeforeClass;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.sos.JSHelper.Logging.Log4JHelper;
import com.sos.scheduler.engine.kernel.test.SchedulerTest;
import com.sos.scheduler.engine.kernel.util.Time;
import com.sos.scheduler.engine.plugins.jms.Configuration;
import com.sos.scheduler.model.SchedulerObjectFactory;
import com.sos.scheduler.model.events.Event;
import com.sos.scheduler.model.events.EventOrderFinished;


/**
 * \file JS628.java
 * \brief 
 *  
 * \class JS628
 * \brief 
 * 
 * \details
 *
 * \code
  \endcode
 *
 * \author schaedi
 * \version 1.0 - 25.05.2011 19:07:16
 * <div class="sos_branding">
 *   <p>(c) 2011 SOS GmbH - Berlin (<a style='color:silver' href='http://www.sos-berlin.com'>http://www.sos-berlin.com</a>)</p>
 * </div>
 */
/**
 * \file JS628.java
 * \brief 
 *  
 * \class JS628
 * \brief 
 * 
 * \details
 *
 * \code
  \endcode
 *
 * \author schaedi
 * \version 1.0 - 25.05.2011 19:07:21
 * <div class="sos_branding">
 *   <p>(c) 2011 SOS GmbH - Berlin (<a style='color:silver' href='http://www.sos-berlin.com'>http://www.sos-berlin.com</a>)</p>
 * </div>
 */
public class JS628 extends SchedulerTest {
    /** Maven: mvn test -Dtest=JmsPlugInTest -DargLine=-Djms.providerUrl=tcp://localhost:61616 */
	
	/* start this module with -Djms.providerUrl=tcp://localhost:61616 to test with an external JMS server */
    private static final String providerUrl = System.getProperty("jms.providerUrl", Configuration.vmProviderUrl);

    private static final Time schedulerTimeout = Time.of(5);
    private static Configuration conf;

    private static Logger logger;
    private final Topic topic = conf.topic;
    private final TopicConnection topicConnection = conf.topicConnectionFactory.createTopicConnection();
    private final TopicSession topicSession = topicConnection.createTopicSession(false, Session.CLIENT_ACKNOWLEDGE);
    
    // Queue for collecting the fired events in the listener thread
    private final BlockingQueue<String> resultQueue = new ArrayBlockingQueue<String>(50);
    
    private final TopicSubscriber topicSubscriber;
    
    private final SchedulerObjectFactory objFactory;
    
    @BeforeClass
    public static void setUpBeforeClass () throws Exception {
		// this file contains appender for ActiveMQ logging
		new Log4JHelper("src/test/resources/log4j.properties");
		logger = LoggerFactory.getLogger(JS628.class);
		conf = Configuration.newInstance(providerUrl);
	}
    
    public JS628() throws Exception {
    	
    	topicSubscriber = newTopicSubscriber();
        topicSubscriber.setMessageListener(new MyListener());
        topicConnection.start();

        //TODO Connect with hostname & port from the scheduler intance
		objFactory = new SchedulerObjectFactory("localhost", 4444);
		objFactory.initMarshaller(com.sos.scheduler.model.events.Event.class);
    }


    /**
     * \brief TopicSubscriber erzeugen nur für OrderFinishedEvent erzeugen
     *
     * @return
     * @throws JMSException
     */
    private TopicSubscriber newTopicSubscriber() throws JMSException {
        String messageSelector = "eventName = 'EventOrderFinished'";
        boolean noLocal = false;
        logger.debug("eventFilter is: " + messageSelector);
        return topicSession.createSubscriber(topic, messageSelector, noLocal);
    }

    
    /**
     * \brief Test ausführen
     * \detail
     * Es werden 2 OrderFinishedEvents erwartet (und sonst nichts)
     *
     * @throws Exception
     */
    @Test 
    public void test() throws Exception {
        runScheduler(schedulerTimeout, "-e");
        assertState("success",1);										// one order has to end with 'success'
        assertState("error",3);											// three order has to end with 'error'
        assertEquals("total number of events",4,resultQueue.size());	// totaly 4 OrderFinishedEvents
    }
    
    private void assertState(String stateName, int exceptedHits) {
    	Iterator<String> it = resultQueue.iterator();
    	int cnt = 0;
    	while (it.hasNext()) {
    		String e = it.next();
    		if(e.equals(stateName)) cnt++;
    	}
        assertEquals("state '" + stateName + "'",exceptedHits,cnt);
    }


    
    private class MyListener implements javax.jms.MessageListener {

        // runs in an own thread
    	@Override
        public void onMessage(Message message) {
            String result = "<unknown event>";
            try {
                TextMessage textMessage = (TextMessage) message;
                String xmlContent = textMessage.getText();
                Event ev = (Event)objFactory.unMarshall(xmlContent);		// get the event object
               	assertNotNull(ev.getEventOrderFinished());
                assertEquals(getTopicname(textMessage), "com.sos.scheduler.engine.Event" );  // Erstmal ist der Klassenname vorangestellt.
                EventOrderFinished ov = ev.getEventOrderFinished();
                textMessage.acknowledge();
                result = ov.getOrder().getState();
                logger.info("order " + ov.getOrder().getId() + " ended with state " + ov.getOrder().getState() );
            }
            catch (JMSException x) { throw new RuntimeException(x); }
            finally {
                try {
					resultQueue.put(result);
				} catch (InterruptedException e) {
					logger.error(e.getMessage());
				}
            }
        }
    }
    
    private String getTopicname(Message m) throws JMSException {
    	Topic t = (Topic)m.getJMSDestination();
    	return (t.getTopicName()!=null) ? t.getTopicName() : "???";
    }

}
