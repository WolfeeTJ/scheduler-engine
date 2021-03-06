package com.sos.scheduler.engine.data.jobchain

import com.sos.scheduler.engine.data.filebased.FileBasedState
import com.sos.scheduler.engine.data.queries.QueryableJobChain
import spray.json.DefaultJsonProtocol._

/**
  * @author Joacim Zschimmer
  */
final case class JobChainOverview(
  path: JobChainPath,
  fileBasedState: FileBasedState,
  isDistributed: Boolean = false,
  orderLimit: Option[Int] = None,
  orderIdSpaceName: Option[String] = None,
  obstacles: Set[JobChainObstacle] = Set())
extends QueryableJobChain

object JobChainOverview {
  private implicit val fileBasedStateJsonFormat = FileBasedState.MyJsonFormat
  implicit val MyJsonFormat = jsonFormat6(apply)
}
