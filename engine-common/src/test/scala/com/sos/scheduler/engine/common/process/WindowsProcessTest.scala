package com.sos.scheduler.engine.common.process

import com.sos.scheduler.engine.common.scalautil.AutoClosing.autoClosing
import com.sos.scheduler.engine.common.scalautil.FileUtils.implicits._
import com.sos.scheduler.engine.common.system.OperatingSystem.isWindows
import java.lang.ProcessBuilder.Redirect
import java.nio.file.Files
import java.nio.file.Files.{copy, createTempFile, delete}
import java.util.concurrent.TimeUnit.{MILLISECONDS, SECONDS}
import org.junit.runner.RunWith
import org.scalatest.FreeSpec
import org.scalatest.Matchers._
import org.scalatest.junit.JUnitRunner
import scala.collection.JavaConversions._

/**
  * @author Joacim Zschimmer
  */
@RunWith(classOf[JUnitRunner])
final class WindowsProcessTest extends FreeSpec {

  if (isWindows) {
    "CreateProcess" in {
      val stdoutFile = createTempFile("test-", ".log")
      val processBuilder = new ProcessBuilder()
        .command(List("ping.exe", "-n", "3", "127.0.0.1"))
        .redirectOutput(Redirect.to(stdoutFile))
      autoClosing(WindowsProcess.start(processBuilder)) { process ⇒
        assert(process.isAlive)
        process.waitFor(10, MILLISECONDS) shouldBe false
        process.waitFor(5, SECONDS) shouldBe true
        assert(!process.isAlive)
        assert(process.exitValue == 0)
      }
      println(s"$stdoutFile contains ${Files.size(stdoutFile)} bytes")
      copy(stdoutFile, System.out)
      assert(Files.size(stdoutFile) > 0)
      assert(stdoutFile.contentString contains "127.0.0.1")
      delete(stdoutFile)
    }

    "TerminateProcess" in {
      val processBuilder = new ProcessBuilder().command(List("ping.exe", "-n", "3", "127.0.0.1"))
      autoClosing(WindowsProcess.start(processBuilder)) { process ⇒
        process.destroy()
        process.waitFor(1, SECONDS)
        assert(!process.isAlive)
        assert(process.exitValue == WindowsProcess.TerminateProcessExitValue)
      }
    }

    "Inherit stdout (manual test)" in {
      val processBuilder = new ProcessBuilder()
        .command(List("ping.exe", "-n", "1", "127.0.0.1"))  // Please test stdout
      autoClosing(WindowsProcess.start(processBuilder)) { process ⇒
        process.waitFor(5, SECONDS) shouldBe true
        assert(process.exitValue == 0)
      }
    }

    "Environment" in {
      val testVariableName = "TEST_VARIABLE"
      val testVariableValue = "TEST-VALUE"
      val scriptFile = createTempFile("test-", ".cmd")
      scriptFile.contentString = s"""
        |@echo off
        |echo $testVariableName=%$testVariableName%
        |exit 0
        |""".stripMargin
      val stdoutFile = createTempFile("test-", ".log")
      val processBuilder = new ProcessBuilder()
        .command(List("cmd.exe", "/C", scriptFile.toString))
        .redirectOutput(Redirect.to(stdoutFile))
      processBuilder.environment ++= Map(testVariableName → testVariableValue)
      autoClosing(WindowsProcess.start(processBuilder)) { process ⇒
        process.waitFor(5, SECONDS) shouldBe true
        copy(stdoutFile, System.out)
        assert(process.exitValue == 0)
      }
      assert(stdoutFile.contentString contains s"$testVariableName=$testVariableValue")
      delete(stdoutFile)
    }

    //"Write to stdin" in {
    //  val scriptFile = createTempFile("test-", ".cmd")
    //  scriptFile.contentString = """
    //    |@echo off
    //    |set /p input=
    //    |echo input=%input%
    //    |""".stripMargin
    //  val stdoutFile = createTempFile("test-", ".log")
    //  val processBuilder = new ProcessBuilder()
    //    .command(List(scriptFile.toString))
    //    .redirectOutput(Redirect.to(stdoutFile))
    //  val testBytes = "HELLO, THIS IS A TEST\r\n".getBytes(US_ASCII)
    //  autoClosing(WindowsProcess.start(processBuilder)) { process ⇒
    //    process.getOutputStream.write(testBytes, 0, testBytes.length)
    //    process.getOutputStream.flush()
    //    process.waitFor(5, SECONDS) shouldBe true
    //  }
    //  stdoutFile.contentBytes shouldEqual testBytes
    //  delete(stdoutFile)
    //  delete(scriptFile)
    //}
  }
}
