package com.sos.scheduler.engine.common.time.alarm

import com.sos.scheduler.engine.common.scalautil.Logger
import com.sos.scheduler.engine.common.time.ScalaTime._
import com.sos.scheduler.engine.common.time.alarm.Alarm.nowMillis
import java.lang.Math.addExact
import java.time.{Duration, Instant}
import java.util.concurrent.atomic.AtomicBoolean
import org.scalactic.Requirements._
import scala.concurrent.{ExecutionContext, Future, blocking}

/**
 * @author Joacim Zschimmer
 */
final class AlarmClock(precision: Duration, idleTimeout: Option[Duration] = None) extends AutoCloseable {

  private val precisionMillis = precision.toMillis max 1
  private val queue = new ConcurrentOrderedQueue(new TreeMapOrderedQueue({ a: Alarm ⇒ millisToKey(a.atEpochMilli): java.lang.Long }))
  @volatile private var closed = false

  private object clock {
    private val logger = Logger(getClass)
    private val lock = new Object
    private val _isRunning = new AtomicBoolean
    @volatile private var stopped = false

    def stop(): Unit = {
      stopped = true
      wake()
    }

    def onAdded()(implicit ec: ExecutionContext): Unit = startOrWake()

    def startOrWake()(implicit ec: ExecutionContext): Unit = {
      if (_isRunning.compareAndSet(false, true)) {
        Future {
          blocking {
            try loop()
            catch { case t: Throwable ⇒
              logger.error(s"$t", t)
              throw t
            }
            finally _isRunning.set(false)
          }
        }
      } else {
        wake()
      }
    }

    def wake(): Unit = lock.synchronized { lock.notifyAll() }

    private def loop(): Unit = {
      var timedout = false
      var hot = false
      while (!stopped && !timedout) {
        queue.popNext(millisToKey(nowMillis())) match {
          case Left(key) ⇒
            if (key == neverKey) {
              hot = false
              timedout = idleUntilTimeout()
            } else {
              keyToMillis(key) - nowMillis() match {
                case duration if duration > 0 ⇒
                  hot = false
                  lock.synchronized { lock.wait(duration) }
                case d ⇒
                  if (hot) {
                    logger.warn(s"queue.popNext returns $d")
                    Thread.sleep(precisionMillis max 100)
                  }
                  hot = true
              }
            }
          case Right(alarm) ⇒
            hot = false
            alarm.run()
        }
      }
    }

    private def idleUntilTimeout(): Boolean = {
      lock.synchronized {
        idleTimeout match {
          case Some(d) ⇒ lock.wait(d.toMillis)
          case None ⇒ lock.wait()
        }
      }
      isEmpty && idleTimeout.isDefined
    }

    def isRunning = _isRunning.get
  }

  private val neverAlarm = Alarm(at = Instant.ofEpochMilli((Long.MaxValue / precisionMillis - 1) * precisionMillis), () ⇒ "Never", () ⇒ {})(null: ExecutionContext)
  private val neverKey = millisToKey(neverAlarm.atEpochMilli)
  queue add neverAlarm  // Marker at end of the never empty queue

  def close(): Unit = {
    closed = true
    clock.stop()
    queue.clear()
  }

  def delay(delay: Duration, name: ⇒ String = "")(call: ⇒ Unit)(implicit ec: ExecutionContext): Alarm =
    at(Instant.now() + delay, name = name)(call)

  def at(at: Instant, name: ⇒ String = "")(call: ⇒ Unit)(implicit ec: ExecutionContext) = {
    val alarm = Alarm(at, name = () ⇒ name, call = () ⇒ call)
    add(alarm)
    alarm
  }

  private def add(alarm: Alarm)(implicit ec: ExecutionContext): Unit = {
    require(millisToKey(alarm.at.toEpochMilli) < neverKey)
    requireState(!closed)

    val t = nextInstant
    queue.add(alarm)
    if (alarm.at < t) {
      clock.onAdded()
    }
  }

  override def toString = "AlarmClock" + (if (isEmpty) "" else s"(${queue.head.at}: ${queue.size} alarms)") mkString ""

  def isEmpty = queue.head.atEpochMilli == neverAlarm.atEpochMilli

  private def nextInstant = queue.head.at

  private[alarm] def isRunning = clock.isRunning

  private def millisToKey(millis: Long) = addExact(millis, precisionMillis - 1) / precisionMillis

  private def keyToMillis(key: Long) = key * precisionMillis
}

object AlarmClock {
  final class ClosedException extends RuntimeException("AlarmClock is closed, no more alarms possible")
}