package com.sos.scheduler.engine.jobapi.scripting

import com.google.common.collect.ImmutableMap
import com.sos.scheduler.engine.common.Lazy
import org.junit.runner.RunWith
import org.scalatest.FunSuite
import org.scalatest.junit.JUnitRunner

@RunWith(classOf[JUnitRunner])
final class JobScriptInstanceAdapterTest extends FunSuite {

  // Für weitere Tests siehe ScriptInstanceTest mit TestExecutor

  test("language") {
    testLanguage("ECMAScript")
    intercept[RuntimeException] { testLanguage("UNKNOWN-LANGUAGE") }
  }

  private def testLanguage(language: String): Unit = {
    val script = "//"
    val bindingsLazy = new Lazy[ImmutableMap[String, AnyRef]] { protected def compute() = ImmutableMap.of()}
    new JobScriptInstanceAdapter(language, bindingsLazy, script)
  }
}
