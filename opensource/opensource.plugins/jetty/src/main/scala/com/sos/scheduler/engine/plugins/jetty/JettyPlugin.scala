package com.sos.scheduler.engine.plugins.jetty

import com.sos.scheduler.engine.kernel.plugin.{Plugin, UseGuiceModule, AbstractPlugin}
import com.sos.scheduler.engine.kernel.scheduler.SchedulerConfiguration
import javax.inject.{Named, Inject}
import org.eclipse.jetty.server.Server
import org.w3c.dom.Element

/** JS-795: Einbau von Jetty in den JobScheduler. */
@UseGuiceModule(classOf[JettyModule])
final class JettyPlugin @Inject()(
    @Named(Plugin.configurationXMLName) pluginElement: Element,
    schedulerConf: SchedulerConfiguration)
extends AbstractPlugin {

  private val server: Server = new ServerBuilder(pluginElement, schedulerConf).build()
  private var started = false

  /** Der Port des ersten Connector */
  def tcpPortNumber = server.getConnectors.head.getPort

  override def activate() {
    server.start()
    started = true
  }

  override def close() {
    server.stop()
    if (started) {
      server.join()
      started = false
    }
  }
}

// jobscheduler/engine/  Übersicht über den Zustand und weitere URIs
// jobscheduler/engine/log  Hauptprotokoll
// jobscheduler/engine/log.snapshot  Hauptprotokoll
// jobscheduler/engine/configuration
// jobscheduler/engine/folders?folder=PATH?type={folder,job,jobChain,...}&deep={false,true}
// jobscheduler/engine/{folders,jobs,jobChains}?folder=PATH&deep={false,true}
// jobscheduler/engine/{folder,job,jobChain}?job=PATH  Übersicht über alle Informationen zum Objekt
// jobscheduler/engine/tasks => Liste der Tasks
// jobscheduler/engine/task/TASKID/state
// jobscheduler/engine/task/TASKID/log
// jobscheduler/engine/task/TASKID/log.state
// jobscheduler/engine/order?jobChain=PATH&order=ORDERID
// jobscheduler/engine/order/configuration?jobChain=PATH&order=ORDERID
// jobscheduler/engine/order/log?jobChain=PATH&order=ORDERID&historyId=...
// jobscheduler/engine/order/log?jobChain=PATH&order=ORDERID
// jobscheduler/engine/order/log.snapshot?jobChain=PATH&order=ORDERID
// jobscheduler/engine/job_chain/configuration?jobChain=PATH
// jobscheduler/engine/job_chain/state?jobChain=PATH
// / -> jobscheduler/ -> jobscheduler/engine/

// ERLEDIGT:
// jobscheduler/engine/gui/  Operations GUI, wenn über web.xml konfiguriert
// jobscheduler/engine/z/  Alte GUI
// jobscheduler/engine/command&command=XMLCOMMAND
// jobscheduler/engine/command  POST
// jobscheduler/engine/job/configuration?job=PATH => Job-Konfiguration
// jobscheduler/engine/job/description?job=PATH
// jobscheduler/engine/job/log.snapshot?job=PATH
