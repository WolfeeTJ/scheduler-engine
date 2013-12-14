package com.sos.scheduler.engine.newkernel.job

import NewJobTest._
import com.sos.scheduler.engine.common.async.{CallRunner, StandardCallQueue}
import com.sos.scheduler.engine.common.scalautil.Logger
import com.sos.scheduler.engine.data.folder.JobPath
import com.sos.scheduler.engine.data.job.{TaskStartedEvent, TaskEndedEvent}
import com.sos.scheduler.engine.eventbus.{EventHandlerAnnotated, EventHandler, SchedulerEventBus}
import javax.xml.stream.XMLInputFactory
import org.joda.time.DateTimeZone
import org.junit.runner.RunWith
import org.scalatest.FunSuite
import org.scalatest.junit.JUnitRunner
import com.sos.scheduler.engine.newkernel.utils.ThreadService
import com.sos.scheduler.engine.newkernel.utils.Service.withService

@RunWith(classOf[JUnitRunner])
class NewJobTest extends FunSuite with EventHandlerAnnotated {
  lazy val inputFactory = XMLInputFactory.newInstance()
  val callQueue = new StandardCallQueue
  val callRunner = new CallRunner(callQueue)

  ignore("xx") {
    val xml =
      <new_job>
        <run_time>
          <period begin="12:42" end="24:00" repeat="5"/>
        </run_time>
        <script language="shell">
          ping -n 3 127.0.0.1
          exit 0
        </script>
      </new_job>
      .toString()
    val eventBus = new SchedulerEventBus
    eventBus.registerAnnotated(this)
    withService(new ThreadService(eventBus)) {
      val jobConfiguration = JobConfigurationXMLParser.parse(xml, inputFactory, DateTimeZone.getDefault)
      val job = new NewJob(jobPath, jobConfiguration, eventBus, callQueue)
      job.activate()
      callRunner.run()
    }
  }

  @EventHandler def handle(e: TaskStartedEvent) {
    logger info e.toString
  }

  @EventHandler def handle(e: TaskEndedEvent) {
    logger info e.toString
    callRunner.end()
  }
}

private object NewJobTest {
  private val logger = Logger(getClass)
  private val jobPath = JobPath.of("/a")
}
