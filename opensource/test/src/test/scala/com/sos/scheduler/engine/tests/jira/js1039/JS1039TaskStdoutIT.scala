package com.sos.scheduler.engine.tests.jira.js1039

import JS1039TaskStdoutIT._
import com.sos.scheduler.engine.common.scalautil.AutoClosing.autoClosing
import com.sos.scheduler.engine.common.utils.FreeTcpPortFinder
import com.sos.scheduler.engine.data.folder.JobPath
import com.sos.scheduler.engine.data.job.TaskClosedEvent
import com.sos.scheduler.engine.kernel.variable.VariableSet
import com.sos.scheduler.engine.test.configuration.TestConfiguration
import com.sos.scheduler.engine.test.scala.ScalaSchedulerTest
import com.sos.scheduler.engine.test.scala.SchedulerTestImplicits._
import java.util.regex.Pattern
import org.junit.runner.RunWith
import org.scalatest.FunSpec
import org.scalatest.Matchers._
import org.scalatest.junit.JUnitRunner
import scala.util.matching.Regex

/** JS-1039 FIXED: API functions stdout_text and stderr_text return empty strings when used in monitor of shell-job.
  * Prüft, ob Task.stdout_text und Task.stderr_text die Ausgaben vom Shell-Prozess enthalten
  * und ob die Ausgaben des Shell-Prozesses und des Monitors im Task-Log erscheinen.
  *
  * 32 Tests: Shell oder API, mit oder ohne Monitor, lokal oder fern, stdout oder stderr:
  * Skript schreibt, spooler_task_after() schreibt, spooler_task_after() liest spooler_task.stdxxx_text. */
@RunWith(classOf[JUnitRunner])
final class JS1039TaskStdoutIT extends FunSpec with ScalaSchedulerTest {

  private lazy val tcpPort = FreeTcpPortFinder.findRandomFreeTcpPort()
  protected override lazy val testConfiguration = TestConfiguration(mainArguments = List(s"-tcp-port=$tcpPort"))
  private lazy val schedulerVariables = scheduler.instance[VariableSet]

  private lazy val jobResults: Map[JobPath, JobResult] = {
    def runJob(jobPath: JobPath) {
      autoClosing(controller.newEventPipe()) { eventPipe =>
        scheduler executeXml <start_job job={jobPath.string}/>
        eventPipe.nextWithCondition[TaskClosedEvent] { _.jobPath == jobPath }
      }
    }

    (jobSettings map { _.jobPath } map { jobPath =>
      for (o <- stdOutErrList) schedulerVariables(o) = ""
      runJob(jobPath)
      jobPath -> JobResult(
        taskLog = controller.environment.taskLogFileString(jobPath),
        variableMap = Map() ++ schedulerVariables)
    }).toMap
  }

  protected override def onSchedulerActivated() {
    scheduler executeXml <process_class name="test-remote" remote_scheduler={s"127.0.0.1:$tcpPort"}/>
  }

  checkJobs(s"Task log should contain output from script") { (result, outOrErr) =>
    shouldOccurExactlyOnce(in = result.taskLog, what = s"/script $outOrErr/")
  }

  checkJobs(s"Task log should contain output from spooler_task_after()", _.hasMonitor) { (result, outOrErr) =>
    shouldOccurExactlyOnce(in = result.taskLog, what = s"/spooler_task_after $outOrErr/")
  }

  checkJobs(s"In spooler_task_after(), spooler_task.stdxxx_text contains scripts output", _.hasMonitor) { (result, outOrErr) =>
    result.variableMap(outOrErr) should include (s"/script $outOrErr/")
  }

  private def checkJobs(testName: String, predicate: JobSetting => Boolean = _ => true)(f: (JobResult, String) => Unit) {
    describe(testName) {
      for (jobSetting <- jobSettings if predicate(jobSetting)) {
        describe(s"Job ${jobSetting.jobPath.name}") {
          for (outOrErr <- stdOutErrList) {
            it(outOrErr.toLowerCase) {
              f(jobResults(jobSetting.jobPath), outOrErr)
            }
          }
        }
      }
    }
  }

  private def shouldOccurExactlyOnce(in: String, what: String) {
    in should include (what)
    withClue(s"'$what' should occur only once:") {
      (new Regex(Pattern.quote(what)) findAllIn in).size should equal(1)
    }
  }
}


private object JS1039TaskStdoutIT {

  private val stdOutErrList = List("STDOUT", "STDERR")  // Nach stdout und stderr geschriebene Strings und zugleich Namen globaler Scheduler-Variablen.

  private case class JobSetting(jobPath: JobPath, hasMonitor: Boolean = false)

  private case class JobResult(taskLog: String, variableMap: Map[String, String])

  private val jobSettings = List(
    JobSetting(JobPath("/test-local-shell")),
    JobSetting(JobPath("/test-local-shell-monitor"), hasMonitor = true),
    JobSetting(JobPath("/test-local-api")),
    JobSetting(JobPath("/test-local-api-monitor"), hasMonitor = true),
    JobSetting(JobPath("/test-remote-shell")),
    JobSetting(JobPath("/test-remote-shell-monitor"), hasMonitor = true),
    JobSetting(JobPath("/test-remote-api")),
    JobSetting(JobPath("/test-remote-api-monitor"), hasMonitor = true))
}