package com.sos.scheduler.engine.plugins.jetty.log

import javax.servlet.{AsyncEvent, AsyncListener}
import javax.servlet.http.{HttpServletResponse, HttpServletRequest}
import com.sos.scheduler.engine.kernel.log.{PrefixLog, LogSubscription}
import org.slf4j.LoggerFactory

//TODO Datei erst nach unsubscribe() löschen.
//Weil Prefix_log zerstört ist, kann vielleicht die Verantwortung fürs Löschen an Java übergehen.

object LogServletAsyncOperation {
  private val logger = LoggerFactory.getLogger(getClass)

  def apply(request: HttpServletRequest, response: HttpServletResponse, log: PrefixLog): FileServletAsyncOperation = {
    val operation = new FileServletAsyncOperation(request, response)

    val logSubscription = new LogSubscription {
      def onStarted(): Unit = {
        logger.info("onStarted")
        operation.start(log.getFile)
      }

      def onClosed(): Unit = {
        logger.info("onClosed")
        operation.end()
        // Unter Windows kann jetzt der Fehler SCHEDULER-291 kommen, weil die Datei noch vom Servlet geöffnet ist.
      }

      def onLogged(): Unit = {
        operation.wake()
      }
    }

    log.subscribe(logSubscription)

    val asyncContext = request.startAsync()
    asyncContext.setTimeout(0)  // 0: Nie

    asyncContext.addListener(new AsyncListener {
      def onStartAsync(event: AsyncEvent): Unit = {}

      def onComplete(event: AsyncEvent): Unit = { close() }

      def onTimeout(event: AsyncEvent): Unit = { close() }

      def onError(event: AsyncEvent): Unit = {
        logger.error("onError", event.getThrowable)
        close()
      }

      private def close(): Unit = {
        log.unsubscribe(logSubscription)
        operation.close()
      }
    })

    if (log.isStarted)
      operation.start(log.getFile)
    operation
  }
}