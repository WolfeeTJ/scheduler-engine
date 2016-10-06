package com.sos.scheduler.engine.kernel.order.jobchain

import com.google.inject.Injector
import com.sos.scheduler.engine.common.guice.GuiceImplicits._
import com.sos.scheduler.engine.common.scalautil.{HasCloser, SetOnce}
import com.sos.scheduler.engine.common.xml.XmlUtils.nodeListToSeq
import com.sos.scheduler.engine.cplusplus.runtime.Sister
import com.sos.scheduler.engine.cplusplus.runtime.annotation.ForCpp
import com.sos.scheduler.engine.data.jobchain.{JobChainNodeAction, JobChainNodePersistentState, JobChainPath, NodeId, NodeKey, NodeOverview}
import com.sos.scheduler.engine.kernel.async.SchedulerThreadCallQueue
import com.sos.scheduler.engine.kernel.async.SchedulerThreadFutures.inSchedulerThread
import com.sos.scheduler.engine.kernel.cppproxy.NodeCI
import com.sos.scheduler.engine.kernel.order.OrderSubsystem
import com.sos.scheduler.engine.kernel.plugin.{AttachableNamespaceXmlPlugin, PluginSubsystem, PluginXmlConfigurable}
import java.time.Duration
import org.w3c.dom

/**
 * @author Joacim Zschimmer
 */
@ForCpp
abstract class Node extends Sister with PluginXmlConfigurable with HasCloser {

  protected def injector: Injector
  protected implicit def schedulerThreadCallQueue: SchedulerThreadCallQueue

  private[kernel] def overview: NodeOverview

  protected[kernel] val cppProxy: NodeCI
  private lazy val orderSubsystem = injector.instance[OrderSubsystem]
  private val jobChainPathOnce = new SetOnce[JobChainPath]
  private val nodeKeyOnce = new SetOnce[NodeKey]

  def onCppProxyInvalidated() = close()

  @ForCpp
  private[kernel] def processConfigurationDomElement(nodeElement: dom.Element): Unit = {
    val elementPluginOption = nodeListToSeq(nodeElement.getChildNodes) collect {
      case e: dom.Element ⇒ e → injector.instance[PluginSubsystem].xmlNamespaceToPlugins[AttachableNamespaceXmlPlugin](e.getNamespaceURI)
    }
    for ((element, plugins) ← elementPluginOption; plugin ← plugins) {
      plugin.attachPluginXmlConfigurable(this, element)
    }
  }

  private[kernel] final def persistentState = new JobChainNodePersistentState(jobChainPath, nodeId, action)

  final lazy val nodeKey = nodeKeyOnce getOrUpdate
    inSchedulerThread {
      NodeKey(jobChainPath, NodeId(cppProxy.string_order_state))
    }

  private[order] def jobChain = orderSubsystem.jobChain(jobChainPath)

  def jobChainPath = inSchedulerThread {
    jobChainPathOnce getOrUpdate JobChainPath(cppProxy.job_chain_path)
  }

  protected[kernel] final def nodeId = nodeKey.nodeId

  protected[kernel] final def nextNodeId = NodeId(cppProxy.string_next_state)

  protected[kernel] final def errorNodeId = NodeId(cppProxy.string_error_state)

  final def action = inSchedulerThread { JobChainNodeAction.ofCppName(cppProxy.string_action) }

  final def action_=(o: JobChainNodeAction): Unit = inSchedulerThread { cppProxy.set_action_string(o.toCppName) }

  private[kernel] def delay = Duration.ofMillis(cppProxy.delay)
}
