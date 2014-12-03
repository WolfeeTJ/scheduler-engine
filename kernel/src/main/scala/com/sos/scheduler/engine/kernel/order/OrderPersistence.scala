package com.sos.scheduler.engine.kernel.order

import OrderPersistence._
import com.sos.scheduler.engine.common.scalautil.Logger
import com.sos.scheduler.engine.common.scalautil.ScalaCollections.emptyToNone
import com.sos.scheduler.engine.cplusplus.runtime.annotation.ForCpp
import com.sos.scheduler.engine.data.order.OrderPersistentState
import com.sos.scheduler.engine.kernel.cppproxy.OrderC
import com.sos.scheduler.engine.kernel.persistence.hibernate.ScalaHibernate._
import org.joda.time.Instant.now
import org.joda.time.format.DateTimeFormat
import org.joda.time.{DateTimeZone, Instant}

trait OrderPersistence {
  this: Order =>

  protected val cppProxy: OrderC
  protected val subsystem: StandingOrderSubsystem

  import subsystem.{entityManagerFactory, orderStore}

  @ForCpp private[order] def persistState(): Unit = {
    transaction(entityManagerFactory) { implicit entityManager =>
      orderStore.store(persistentState)
    }
  }

  private def persistentState = OrderPersistentState(
    jobChainPath = jobChainPath,
    orderId = id,
    distributedNextTimeOption = parseInstantOption(cppProxy.calculate_db_distributed_next_time),
    occupyingClusterIdOption = None, //if (jobChain.isDistributed) Some(orderSubsystem.clusterMemberId) else None,
    priority = priority,
    ordering = nextOrderingNumber(),
    stateOption = Some(state),
    initialStateOption = Some(initialState),
    title = title.take(titleColumnSize),
    creationTimestampOption = createdAtOption,
    modificationTimestampOption = Some(now),
    payloadXmlOption = emptyToNone(cppProxy.string_payload),
    runtimeXmlOption = emptyToNone(cppProxy.database_runtime_xml),
    xmlOption = Some(cppProxy.database_xml))

  private def nextOrderingNumber(): Int =
    ??? // db()->get_id( "spooler_order_ordering", ta );
}


private object OrderPersistence {
  private val titleColumnSize = 200
  private val dateTimeFormatter = DateTimeFormat forPattern "yyyy-MM-dd HH:mm:ss" withZone DateTimeZone.UTC

  private def parseInstantOption(o: String): Option[Instant] =
    if (o.isEmpty) None
    else Some(dateTimeFormatter.parseDateTime(o).toInstant)
}
