package com.sos.scheduler.engine.tests.jira.js856

import JS856IT.originalParameters
import com.sos.scheduler.engine.test.configuration.{DefaultDatabaseConfiguration, TestConfiguration}
import org.junit.runner.RunWith
import org.scalatest.junit.JUnitRunner

/** JS-856 */
@RunWith(classOf[JUnitRunner])
final class NewBehaviourJS856IT extends JS856IT("New behaviour: restore original order state") {

  override lazy val testConfiguration = TestConfiguration(
    database = Some(DefaultDatabaseConfiguration()),
    logCategories = "java.stackTrace-",   // Exceptions wegen fehlender Datenbanktabellen wollen wir nicht sehen.
    resourceNameMap = List("new-behaviour-scheduler.xml" -> "scheduler.xml"))

  val finallyExpectedParameters = originalParameters
  val whenSuspendedExpectedParameters = Map("a" -> "a-job", "b" -> "b-job")
}