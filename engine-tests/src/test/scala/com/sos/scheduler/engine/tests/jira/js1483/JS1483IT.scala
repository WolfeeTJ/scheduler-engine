package com.sos.scheduler.engine.tests.jira.js1483

import com.sos.scheduler.engine.common.soslicense.LicenseKeyParameterIsMissingException
import com.sos.scheduler.engine.common.time.ScalaTime._
import com.sos.scheduler.engine.common.time.WaitForCondition.waitForCondition
import com.sos.scheduler.engine.data.job.JobPath
import com.sos.scheduler.engine.kernel.job.JobState
import com.sos.scheduler.engine.test.SchedulerTestUtils._
import com.sos.scheduler.engine.test.agent.AgentWithSchedulerTest
import com.sos.scheduler.engine.test.scalatest.ScalaSchedulerTest
import com.sos.scheduler.engine.tests.jira.js1483.JS1483IT._
import org.junit.runner.RunWith
import org.scalatest.FreeSpec
import org.scalatest.junit.JUnitRunner

/**
 * JS-1483 Task start failure due to missing license key is stated in Job.state_text.
 * JS-1482 Without a license key, Agent runs one task at a time
 *
 * @author Joacim Zschimmer
 */
@RunWith(classOf[JUnitRunner])
final class JS1483IT extends FreeSpec with ScalaSchedulerTest with AgentWithSchedulerTest {

  "Without a license key, Agent runs one task at a time - JS-1482" in {
    runJobAndWaitForEnd(TestJobPath)
    sleep(100.ms)  // Sometimes the task has not been closed before the next start ???
    runJobAndWaitForEnd(TestJobPath)
    sleep(100.ms)
    runJobAndWaitForEnd(TestJobPath)
    sleep(100.ms)
  }

  "Task start failure due to missing license key is stated in Job.state_text" in {
    // Test does not work with external license keys as in ~/sos.ini or /etc/sos.ini
    val firstRun = runJobFuture(SleepJobPath)
    awaitSuccess(firstRun.started)
    controller.toleratingErrorCodes(_ ⇒ true) {
      runJobFuture(TestJobPath)
      waitForCondition(TestTimeout, 100.ms) { job(TestJobPath).state == JobState.stopped }
      assert(job(TestJobPath).stateText startsWith classOf[LicenseKeyParameterIsMissingException].getSimpleName)
      assert(job(TestJobPath).stateText contains "No license key provided by master to execute jobs in parallel")
      scheduler executeXml <kill_task job="/test-sleep" id={firstRun.taskId.string} immediately="true"/>
      awaitSuccess(firstRun.closed)
    }
  }
}

private object JS1483IT {
  private val TestJobPath = JobPath("/test")
  private val SleepJobPath = JobPath("/test-sleep")
}
