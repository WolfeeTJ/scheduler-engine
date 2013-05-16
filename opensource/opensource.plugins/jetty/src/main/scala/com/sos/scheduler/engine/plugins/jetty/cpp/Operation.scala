package com.sos.scheduler.engine.plugins.jetty.cpp

import Operation._
import com.sos.scheduler.engine.common.scalautil.Logger
import com.sos.scheduler.engine.cplusplus.runtime.DisposableCppProxyRegister
import com.sos.scheduler.engine.kernel.http.SchedulerHttpResponse
import com.sos.scheduler.engine.kernel.scheduler.{SchedulerIsClosed, SchedulerHttpService}
import java.util.concurrent.atomic.AtomicBoolean
import javax.annotation.Nullable
import javax.servlet.http.{HttpServletResponse, HttpServletRequest}
import javax.servlet.{AsyncEvent, AsyncListener}

private[cpp] final class Operation(
    request: HttpServletRequest,
    response: HttpServletResponse,
    schedulerHttpService: SchedulerHttpService,
    cppProxyRegister: DisposableCppProxyRegister,
    schedulerIsClosed: SchedulerIsClosed
) extends SchedulerHttpResponse {

  private val _isClosed = new AtomicBoolean(false)

  private val asyncListener = new AsyncListener {
    def onComplete(event: AsyncEvent) {
      close()
    }

    def onTimeout(event: AsyncEvent) {
      close()
    }

    def onError(event: AsyncEvent) {
      for (t <- Option(event.getThrowable)) logger.error(s"AsyncListener.onError: $t", t)
      close()
    }

    def onStartAsync(event: AsyncEvent) {}
  }

  /** Das C++-Objekt httpResponseC MUSS mit Release() wieder freigegeben werden, sonst Speicherleck. */
  private lazy val httpResponseCRef = cppProxyRegister.reference(
    schedulerHttpService.executeHttpRequest(new ServletSchedulerHttpRequest(request), this))

  private def httpResponseC = httpResponseCRef.get

  @Nullable private lazy val chunkReaderC = httpResponseC.chunk_reader

  def start() {
    response.setStatus(httpResponseC.status)
    splittedHeaders(httpResponseC.header_string) foreach { h => response.setHeader(h._1, h._2) }
    if (chunkReaderC != null) continue()
    else close()
  }

  def tryClose() {
    try close()
    catch {
      case x: Throwable => if (schedulerIsClosed.isClosed) logger.error(x.toString) else logger.error(x.toString, x)
    }
  }

  def close() {
    if (!_isClosed.getAndSet(true))
      closeHttpResponseC()
  }

  private def closeHttpResponseC() {
    try httpResponseC.close()
    finally {
      logger.debug("httpResponseC.dispose()")
      cppProxyRegister.dispose(httpResponseCRef)
    }
  }

  def continue() {
    serveChunks()
    response.getOutputStream.flush()
    if (!isClosed) startAsync()
  }

  private def startAsync() {
    val asyncConcext = request.startAsync(request, response)
    asyncConcext.setTimeout(0)  // Nie
    asyncConcext.addListener(asyncListener)
  }

  def serveChunks() {
    while (!isClosed && chunkReaderC != null && chunkReaderC.next_chunk_is_ready) {
      chunkReaderC.get_next_chunk_size match {
        case 0 => close()
        case size => response.getOutputStream.write(chunkReaderC.read_from_chunk(size))
      }
    }
  }

  def onNextChunkIsReady() {
    try
      if (request.isAsyncStarted)
        request.getAsyncContext.dispatch()
    catch {
      case x: Throwable => logger.error(x.toString, x)
    }
  }

  def isClosed = _isClosed.get
}

object Operation {
  private val logger = Logger(classOf[Operation])

  private def splittedHeaders(headers: String): Iterable[(String, String)] = headers match {
    case "" => Iterable()
    case _ => headers.split("\r\n") map { _.split(": ", 2) } map { h => h(0) -> h(1) }
  }
}