package com.sos.scheduler.engine.tests.scheduler.job.wake_when_not_in_period

import WakeWhenInPeriodIT._
import com.sos.scheduler.engine.data.folder.JobPath
import com.sos.scheduler.engine.data.job.TaskStartedEvent
import com.sos.scheduler.engine.eventbus.EventHandler
import com.sos.scheduler.engine.test.scala.ScalaSchedulerTest
import com.sos.scheduler.engine.test.scala.SchedulerTestImplicits._
import org.joda.time.DateTimeConstants.MILLIS_PER_DAY
import org.joda.time._
import org.joda.time.format.DateTimeFormat
import org.junit.runner.RunWith
import org.scalatest.FunSuite
import org.scalatest.Matchers._
import org.scalatest.junit.JUnitRunner
import scala.collection.mutable
import com.sos.scheduler.engine.common.time.ScalaJoda._

/** JS-948 */
@RunWith(classOf[JUnitRunner])
final class WakeWhenInPeriodIT extends FunSuite with ScalaSchedulerTest {

  private val startTimes = mutable.Buffer[LocalTime]()

  test("wake_when_in_period") {
    val now = new LocalTime()
    val minimumTimeUntilMidnight = 10000
    if (now isAfter now.withMillisOfDay(MILLIS_PER_DAY - minimumTimeUntilMidnight))
      sleep(minimumTimeUntilMidnight + 1000)

    val t = new LocalTime(now) plusMillis 1999 withMillisOfSecond 0
    val a = SchedulerPeriod(t plusSeconds 2, t plusSeconds 6)
    val b = SchedulerPeriod(t plusSeconds 8, t plusSeconds 10)
    scheduler executeXml jobElem(List(a, b))

    scheduler executeXml <modify_job job={jobPath.string} cmd="wake_when_in_period"/>   // Vor der Periode: unwirksam
    sleepUntil(a.begin plusMillis 100)
    scheduler executeXml <modify_job job={jobPath.string} cmd="wake_when_in_period"/>   // In der Periode: wirksam
    sleepUntil(a.begin plusMillis 2100)
    scheduler executeXml <modify_job job={jobPath.string} cmd="wake_when_in_period"/>   // In der Periode: wirksam
    sleepUntil(a.end plusMillis 100)
    scheduler executeXml <modify_job job={jobPath.string} cmd="wake_when_in_period"/>   // Nach der Periode: unwirksam
    sleepUntil(b.begin plusMillis 500)

    startTimes should have size 2
    assert(a contains startTimes(0))
    assert(a contains startTimes(1))
  }

  @EventHandler def handle(e: TaskStartedEvent) {
    if (e.jobPath == jobPath)
      startTimes += new LocalTime
  }
}

private object WakeWhenInPeriodIT {
  private val jobPath = JobPath.of("/a")
  private val hhmmssFormat = DateTimeFormat.forPattern("HH:mm:ss")

  private def jobElem(periods: Iterable[SchedulerPeriod]) =
    <job name={jobPath.getName}>
      <script language="shell">exit 0</script>
      <run_time>{ periods map { o => <period begin={hhmmssFormat.print(o.begin)} end={hhmmssFormat.print(o.end)}/>} }</run_time>
    </job>

  private case class SchedulerPeriod(begin: LocalTime, end: LocalTime) {
    def contains(t: LocalTime) = !(t isBefore begin) && (t isBefore end)
  }

  private def sleepUntil(t: LocalTime) {
    sleep(t.getMillisOfDay - (new LocalTime).getMillisOfDay)
  }
}
