package com.sos.scheduler.engine.tests.jira.js653

import com.sos.scheduler.engine.common.time.ScalaTime._
import com.sos.scheduler.engine.data.event.KeyedEvent
import com.sos.scheduler.engine.data.jobchain.{JobChainPath, NodeId}
import com.sos.scheduler.engine.data.order.{OrderFinished, OrderId, OrderKey, OrderStarted}
import com.sos.scheduler.engine.test.SchedulerTestUtils._
import com.sos.scheduler.engine.test.scalatest.ScalaSchedulerTest
import com.sos.scheduler.engine.tests.jira.js653.JS653IT._
import java.lang.System.currentTimeMillis
import java.lang.Thread.sleep
import org.junit.runner.RunWith
import org.scalatest.FreeSpec
import org.scalatest.junit.JUnitRunner
import scala.collection.mutable

/**
  * Ticket JS-653.
  * <a href='http://www.sos-berlin.com/jira/browse/JS-653'>JS-653</a>
  */
@RunWith(classOf[JUnitRunner])
final class JS653IT extends FreeSpec with ScalaSchedulerTest {

  private val orderStarts = mutable.Set[(OrderId, NodeId)]()
  private val orderEnds = mutable.Set[(OrderKey, NodeId)]()
  private var lastActivity = 0L

  "test" in  {
    waitUntilOrdersAreNotStarted()
    assert(orderStarts == ExpectedOrderStarts)
    assert(orderEnds == ExpectedOrderEnds)
  }

  /* Warten bis wir einigermaßen sicher sind, dass Aufträge, die nicht starten sollen, nicht gestartet werden. **/ @throws[InterruptedException]
  private def waitUntilOrdersAreNotStarted() {
    lastActivity = currentTimeMillis
    while (true) {
      val remaining = lastActivity + IdleTimeout.toMillis - currentTimeMillis
      if (remaining < 0) return
      sleep(remaining)
    }
  }

  eventBus.onHot[OrderStarted.type] {
    case KeyedEvent(orderKey, _) ⇒
      orderStarts.add(orderKey.id → orderOverview(orderKey).nodeId)
  }

  eventBus.onHot[OrderFinished] {
    case KeyedEvent(orderKey, e) ⇒
      orderEnds.add(orderKey → e.nodeId)
      lastActivity = currentTimeMillis
  }
}

private object JS653IT {
  private val IdleTimeout = 10.s
  private val ExpectedOrderStarts = Set(
    OrderId("simpleShouldRun")          → NodeId("state.job1"),
    OrderId("simpleWithStateShouldRun") → NodeId("state.job1"),
    OrderId("superShouldRun")           → NodeId("state.nestedA.job1"),
    OrderId("superWithStateBShouldRun") → NodeId("state.nestedB.job1"))
  private val ExpectedOrderEnds = Set(
    (JobChainPath("/simple") orderKey "simpleShouldRun")          → NodeId("end"),
    (JobChainPath("/simple") orderKey "simpleWithStateShouldRun") → NodeId("end"),
    (JobChainPath("/b") orderKey "superShouldRun")                → NodeId("end"),
    (JobChainPath("/b") orderKey "superWithStateBShouldRun")      → NodeId("end"))
}
