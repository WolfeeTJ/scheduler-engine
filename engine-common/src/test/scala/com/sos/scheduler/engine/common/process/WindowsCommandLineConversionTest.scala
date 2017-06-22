package com.sos.scheduler.engine.common.process

import org.scalatest.FreeSpec
import WindowsCommandLineConversion.argsToCommandLine

/**
  * @author Joacim Zschimmer
  */
final class WindowsCommandLineConversionTest extends FreeSpec {

  "argsToCommandLine" in {
    assert(argsToCommandLine(List("./a", "b c")) == """.\a "b c"""")
    assert(argsToCommandLine(List("""C:\Program Files\x""", "b c")) == """"C:\Program Files\x" "b c"""")
    intercept[IllegalArgumentException] {
      argsToCommandLine(List("""a"b"""))
    }
    intercept[IllegalArgumentException] {
      argsToCommandLine(List("a", """a""""))
    }
  }
}
