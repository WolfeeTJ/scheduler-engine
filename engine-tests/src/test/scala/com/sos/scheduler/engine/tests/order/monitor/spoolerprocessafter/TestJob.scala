package com.sos.scheduler.engine.tests.order.monitor.spoolerprocessafter

import sos.spooler.Job_impl

class TestJob extends Job_impl {
  override def spooler_process() = {
    import setting.SpoolerProcessNames.{logError, returns, throwException}

    val params = spooler_task.order.params
    if (!params.value(logError).isEmpty)  spooler_log.error(params.value(logError) +" ...")
    if (!params.value(throwException).isEmpty)  throw new RuntimeException("EXCEPTION IN SPOOLER_PROCESS")
    params.value(returns).toBoolean
  }
}
