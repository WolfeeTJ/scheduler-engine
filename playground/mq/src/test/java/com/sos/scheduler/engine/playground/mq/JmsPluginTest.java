package com.sos.scheduler.engine.playground.mq;

import com.sos.scheduler.engine.common.sync.Gate;
import com.sos.scheduler.engine.test.SchedulerTest;
import org.apache.log4j.Logger;
import org.junit.Test;

import javax.jms.*;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;

public final class JmsPluginTest extends SchedulerTest {
    /** Maven: mvn test -Dtest=JmsPluginTest -DargLine=-Djms.providerUrl=tcp://localhost:61616 */
    private static final String providerUrl = System.getProperty("jms.providerUrl", Configuration.vmProviderUrl);

    private static final Configuration conf = Configuration.newInstance(providerUrl);
    private static final Logger logger = Logger.getLogger(JmsPluginTest.class);

    private final Topic topic = conf.topic;
    private final TopicConnection topicConnection = conf.topicConnectionFactory.createTopicConnection();
    private final TopicSession topicSession = topicConnection.createTopicSession(false, Session.AUTO_ACKNOWLEDGE);
    private final Gate<Boolean> resultGate = new Gate<Boolean>();

    public JmsPluginTest() throws JMSException {
        newTopicSubscriber().setMessageListener(new MyListener());
        topicConnection.start();
    }

    private TopicSubscriber newTopicSubscriber() throws JMSException {
        String messageSelector = null;
        boolean noLocal = false;
        return topicSession.createSubscriber(topic, messageSelector, noLocal);
    }

    @Test public void test() throws InterruptedException {
        controller().activateScheduler();
        assertThat(resultGate.poll(shortTimeout), equalTo(true));
    }

    private final class MyListener implements MessageListener {
        @Override public void onMessage(Message message) {
            // Läuft in einem Thread von JMS
            boolean result = false;
            try {
                TextMessage textMessage = (TextMessage) message;
                logger.debug("onMessage: " + textMessage.getText());
                //assertThat(textMessage.getText(), startsWith("com.sos.scheduler.engine."));  // Erstmal ist der Klassenname vorangestellt.
                result = true;
            }
            catch (JMSException x) { throw new RuntimeException(x); }
            finally {
                resultGate.offer(result);
            }
        }
    }
}
