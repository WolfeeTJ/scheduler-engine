package com.sos.scheduler.engine.tests.jira.js1191

import com.sos.scheduler.engine.data.jobchain.JobChainPath
import com.sos.scheduler.engine.data.order.OrderState
import com.sos.scheduler.engine.test.SchedulerTestUtils._
import com.sos.scheduler.engine.test.scalatest.ScalaSchedulerTest
import org.junit.runner.RunWith
import org.scalatest.FreeSpec
import org.scalatest.junit.JUnitRunner

/**
 * JS-1191 Order.last_error
 *
 * @author Joacim Zschimmer
 */
@RunWith(classOf[JUnitRunner])
final class JS1191IT extends FreeSpec with ScalaSchedulerTest {

  "Order.last_error" in {
    controller.toleratingErrorLogEvent(_ ⇒ true) {
      val result = runOrder(JobChainPath("/test") orderKey "1")
      assert(result.state == OrderState("END"))
    }
  }
}
