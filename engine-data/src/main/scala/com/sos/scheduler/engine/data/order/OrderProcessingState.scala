package com.sos.scheduler.engine.data.order

import com.sos.jobscheduler.base.sprayjson.JavaTimeJsonFormats.implicits._
import com.sos.jobscheduler.base.sprayjson.typed.{Subtype, TypedJsonFormat}
import com.sos.jobscheduler.data.agent.AgentAddress
import com.sos.jobscheduler.data.job.TaskId
import com.sos.scheduler.engine.data.processclass.ProcessClassPath
import com.sos.scheduler.engine.data.scheduler.ClusterMemberId
import java.time.Instant
import spray.json.DefaultJsonProtocol._

/**
  * @author Joacim Zschimmer
  */
sealed trait OrderProcessingState {
  def isDueOrStarted = false
  def isStarted = false
  def isWaiting = false
  def isInProcess = false
}

object OrderProcessingState {

  case object NotPlanned
  extends OrderProcessingState

  final case class Planned(at: Instant)
  extends OrderProcessingState

  sealed trait Waiting
  extends OrderProcessingState {
    override def isWaiting = true
  }

  final case class Due(at: Instant)
  extends Waiting {
    override def isDueOrStarted = true
  }

  sealed trait Started
  extends OrderProcessingState{
    override def isStarted = true
    override def isDueOrStarted = true
  }

  sealed trait InTask
  extends Started {
    def taskId: TaskId
    def processClassPath: ProcessClassPath
  }

  final case class WaitingInTask(
    taskId: TaskId,
    processClassPath: ProcessClassPath)
  extends InTask with Waiting

  final case class InTaskProcess(
    taskId: TaskId,
    processClassPath: ProcessClassPath,
    since: Instant,
    agentUri: Option[AgentAddress])
  extends InTask {
    override def isInProcess = true
  }

  final case class OccupiedByClusterMember(clusterMemberId: ClusterMemberId)
  extends OrderProcessingState with Started

  final case class Setback(until: Instant)
  extends Started with Waiting

  case object WaitingForResource
  extends Started with Waiting

  case object Blacklisted
  extends OrderProcessingState

  implicit val typedJsonFormat = TypedJsonFormat[OrderProcessingState](
    Subtype(jsonFormat0(() ⇒ NotPlanned)),
    Subtype(jsonFormat1(Planned.apply)),
    Subtype(jsonFormat1(Due.apply)),
    Subtype(jsonFormat2(WaitingInTask.apply)),
    Subtype(jsonFormat4(InTaskProcess.apply)),
    Subtype(jsonFormat1(OccupiedByClusterMember.apply)),
    Subtype(jsonFormat1(Setback.apply)),
    Subtype(jsonFormat0(() ⇒ WaitingForResource)),
    Subtype(jsonFormat0(() ⇒ Blacklisted)))
}
