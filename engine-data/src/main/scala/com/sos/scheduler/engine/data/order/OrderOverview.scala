package com.sos.scheduler.engine.data.order

import com.sos.jobscheduler.base.sprayjson.JavaTimeJsonFormats.implicits._
import com.sos.jobscheduler.base.sprayjson.SprayJson.lazyRootFormat
import com.sos.scheduler.engine.data.filebased.FileBasedState
import com.sos.scheduler.engine.data.jobchain.{JobChainPath, NodeId, NodeKey}
import com.sos.scheduler.engine.data.order.OrderProcessingState.OccupiedByClusterMember
import com.sos.scheduler.engine.data.queries.QueryableOrder
import java.time.Instant
import scala.collection.immutable
import scala.language.implicitConversions
import spray.json.DefaultJsonProtocol._
import spray.json.RootJsonFormat

/**
  * @author Joacim Zschimmer
  */
final case class OrderOverview(
  path: OrderKey,
  fileBasedState: FileBasedState,
  orderSourceType: OrderSourceType,
  jobChainPath: JobChainPath,
  nodeId: NodeId,
  orderProcessingState: OrderProcessingState,
  historyId: Option[OrderHistoryId] = None,
  obstacles: Set[OrderObstacle] = Set(),
  startedAt: Option[Instant] = None,
  nextStepAt: Option[Instant] = None,
  outerJobChainPath: Option[JobChainPath] = None)
extends OrderView with QueryableOrder {

  def orderKey: OrderKey = path

  def nodeKey: NodeKey = NodeKey(jobChainPath, nodeId)

  def isSetback = orderProcessingStateClass == classOf[OrderProcessingState.Setback]

  def isBlacklisted = orderProcessingStateClass == OrderProcessingState.Blacklisted.getClass

  def isSuspended = obstacles contains OrderObstacle.Suspended

  def occupyingClusterMemberId = orderProcessingState match {
    case o: OccupiedByClusterMember ⇒ Some(o.clusterMemberId)
    case _ ⇒ None
  }
}

object OrderOverview extends OrderView.Companion[OrderOverview] {
  implicit val jsonFormat: RootJsonFormat[OrderOverview] = {
    implicit val a = FileBasedState.MyJsonFormat
    implicit val b = OrderSourceType.MyJsonFormat
    lazyRootFormat(jsonFormat11(apply))
  }

  implicit val ordering: Ordering[OrderOverview] = Ordering by { o ⇒ (o.orderKey.jobChainPath, o.nodeId, o.orderKey.id) }

  final class Statistics(val orderOverviews: immutable.Seq[OrderOverview]) {
    def count = orderOverviews.size
    lazy val inProcessCount = orderOverviews count { _.orderProcessingStateClass == classOf[OrderProcessingState.InTaskProcess] }
    lazy val suspendedCount = orderOverviews count { _.isSuspended }
    lazy val blacklistedCount = orderOverviews count { _.isBlacklisted }
  }
}
