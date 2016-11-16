package com.sos.scheduler.engine.client.web

import akka.actor.ActorSystem
import com.sos.scheduler.engine.client.web.StandardWebSchedulerClient._
import com.sos.scheduler.engine.common.auth.UserAndPassword
import com.sos.scheduler.engine.common.configutils.Configs
import com.sos.scheduler.engine.common.utils.JavaResource

/**
  * @author Joacim Zschimmer
  */
final class StandardWebSchedulerClient(
  val uris: SchedulerUris,
  override protected val credentials: Option[UserAndPassword])
extends WebSchedulerClient with AutoCloseable {

  def this(schedulerUri: String, credentials: Option[UserAndPassword] = None) =
    this(SchedulerUris(schedulerUri), credentials)

  protected def hostConnectorSetupOption = None  // That means, no HTTPS

  private val actorSystem = ActorSystem("StandardWebSchedulerClient", Configs.loadResource(ConfigResource))
  protected val actorRefFactory = actorSystem

  def close() = {
    actorSystem.shutdown()
    actorSystem.awaitTermination()
  }
}

private object StandardWebSchedulerClient {
  private val ConfigResource = JavaResource("com/sos/scheduler/engine/client/web/WebSchedulerClient.conf")
}
