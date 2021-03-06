package com.sos.scheduler.engine.data.scheduler

import com.sos.jobscheduler.base.sprayjson.JavaTimeJsonFormats.implicits._
import com.sos.jobscheduler.base.system.SystemInformation
import com.sos.jobscheduler.data.scheduler.SchedulerId
import com.sos.jobscheduler.data.system.JavaInformation
import com.sos.scheduler.engine.data.scheduler.SchedulerStates.SchedulerStateJsonFormat
import java.time.Instant
import spray.json.DefaultJsonProtocol._

final case class SchedulerOverview(
  version: String,
  startedAt: Instant,
  schedulerId: SchedulerId,
  httpPort: Option[String],
  httpsPort: Option[String],
  udpPort: Option[Int],
  supervisor: Option[SupervisorUri] = None,
  pid: Int,
  state: SchedulerState,
  system: SystemInformation,
  java: JavaInformation)

object SchedulerOverview {
  implicit val MyJsonFormat = jsonFormat11(apply)
}
