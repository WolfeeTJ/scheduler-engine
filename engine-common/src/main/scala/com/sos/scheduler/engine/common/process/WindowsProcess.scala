package com.sos.scheduler.engine.common.process

import com.sos.scheduler.engine.common.process.Processes.Pid
import com.sos.scheduler.engine.common.process.WindowsCommandLineConversion.argsToCommandLine
import com.sos.scheduler.engine.common.process.WindowsProcess._
import com.sos.scheduler.engine.common.scalautil.Logger
import com.sun.jna.Pointer
import com.sun.jna.NativeString
import com.sun.jna.Memory
import com.sun.jna.platform.win32.Kernel32.{INSTANCE ⇒ kernel32}
import com.sun.jna.platform.win32.Kernel32Util.formatMessageFromLastErrorCode
import com.sun.jna.platform.win32.Advapi32Util.getEnvironmentBlock
import com.sun.jna.platform.win32.WinBase._
import com.sun.jna.platform.win32.WinDef._
import com.sun.jna.platform.win32.WinError._
import com.sun.jna.platform.win32.WinNT._
import com.sun.jna.ptr.IntByReference
import java.io.OutputStream
import java.lang.Math.{max, min}
import java.lang.ProcessBuilder.Redirect
import java.nio.charset.StandardCharsets
import java.util.concurrent.TimeUnit
import scala.collection.JavaConversions._

/**
  * A Windows process, started with CreateProcessW via JNA.
  *
  * @author Joacim Zschimmer
  */
final class WindowsProcess(startupInfo: STARTUPINFO, processInformation: PROCESS_INFORMATION, stdin: HANDLE, stdout: HANDLE, stderr: HANDLE)
extends Process with AutoCloseable {

  import processInformation.{hProcess, hThread}
  //import startupInfo.{hStdError, hStdInput, hStdOutput}

  val pid = Pid(processInformation.dwProcessId.intValue)
  val pidOption = Some(pid)
  private lazy val stdinStream: OutputStream = {
    if (stdin == null) throw new IllegalStateException("WindowsProcess has no handle for stdin attached")
    new OutputStream {
      def write(b: Int) = throw new UnsupportedOperationException("WindowsProcess provides no stdin OutputStream")  //stdinStream
  //    def write(b: Int) = write(Array(b.toByte), 0, 1)
  //
  //    override def write(bytes: Array[Byte], offset: Int, length: Int) = {
  //      val written = new IntByReference
  //      val a = if (offset == 0 && length == bytes.length) bytes else bytes.slice(offset, offset + length)
  //      require(length <= a.length, "Invalid length")
  //      if (!kernel32.WriteFile(stdin, bytes, length, written, null))
  //        throwLastError("WriteFile")
  //      if (written.getValue != length) sys.error(s"Less bytes written (${written.getValue} to process' stdin than expected ($length)")
  //    }
  //
  //    override def flush() = {
  //      if (!kernel32.FlushFileBuffers(stdin))
  //        throwLastError("FlushFileBuffers")
  //    }
  //
  //    override def close() = {
  //      if (stdin != null) {
  //        kernel32.CloseHandle(stdin)
  //        stdin.setPointer(null)
  //      }
  //    }
    }
  }
  private var closed = false

  def close() = {
    if (!closed) {
      closed = true
      if (hThread != null) {
        kernel32.CloseHandle(hThread)
      }
      if (hProcess != null) {
        kernel32.CloseHandle(hProcess)
      }
      if (stdin != null) {
        kernel32.CloseHandle(stdin)
      }
      if (stdout != null) {
        kernel32.CloseHandle(stdout)
      }
      if (stderr != null) {
        kernel32.CloseHandle(stderr)
      }
      kernel32.CloseHandle(startupInfo.hStdInput)
      kernel32.CloseHandle(startupInfo.hStdOutput)
      kernel32.CloseHandle(startupInfo.hStdError)
    }
  }

  override def waitFor(timeout: Long, unit: TimeUnit) = {
    val t = max(0, min(Int.MaxValue, TimeUnit.MILLISECONDS.convert(timeout, unit))).toInt
    logger.trace(s"WaitForSingleObject ${timeout}ms")
    waitForSingleObject(hProcess, t)
  }

  def waitFor() = {
    logger.trace(s"WaitForSingleObject eternal")
    val ok = waitForSingleObject(hProcess, INFINITE)
    if (!ok) throw new WindowsException("WaitForSingleTimeout INFINITE timeout")
    getExitCode(hProcess)
  }

  override def toString = pid.toString //s"WindowsProcess($pid,${if (isAlive) "alive" else "terminated"})"

  override def isAlive = !waitForSingleObject(hProcess, 0)

  def exitValue = {
    if (!waitForSingleObject(hProcess, 0)) throw new IllegalThreadStateException(s"Process $pid has not yet terminated")
    getExitCode(hProcess)
  }

  def destroy() = {
    logger.trace("TerminateProcess")
    if (!kernel32.TerminateProcess(hProcess, TerminateProcessExitValue))
      throwLastError("TerminateProcess")
  }

  def getOutputStream = stdinStream

  def getInputStream = throw new UnsupportedOperationException("WindowsProcess provides no stdout InputStream")

  def getErrorStream = throw new UnsupportedOperationException("WindowsProcess provides no stderr InputStream")
}

object WindowsProcess {
  private val logger = Logger(getClass)
  private val InvalidHANDLE = new HANDLE(new Pointer(-1L))
  val TerminateProcessExitValue = 999999999

  def start(processBuilder: ProcessBuilder): WindowsProcess = {
    val commandLine = argsToCommandLine(processBuilder.command.toIndexedSeq)
    val environment = {
      val env = getEnvironmentBlock(processBuilder.environment)
      val m = new Memory(env.length + 1); // WARNING: assumes ascii-only string
      m.setString(0, env)
      m
    }
    val startupInfo = new STARTUPINFO
    startupInfo.dwFlags |= STARTF_USESTDHANDLES
    val (hStdInput, stdinHandle) = redirectToHandle(0, processBuilder.redirectInput)
    val (hStdOutput, stdoutHandle) = redirectToHandle(1, processBuilder.redirectOutput)
    val (hStdError, stderrHandle) = redirectToHandle(2, processBuilder.redirectError)
    startupInfo.hStdInput = hStdInput
    startupInfo.hStdOutput = hStdOutput
    startupInfo.hStdError = hStdError
    val processInformation = new PROCESS_INFORMATION

    logger.trace("CreateProcessW")
    val ok = kernel32.CreateProcessW(
      null,
      (commandLine + "\0").toCharArray,
      null: SECURITY_ATTRIBUTES,
      null: SECURITY_ATTRIBUTES,
      true, //inheritHandles,
      new DWORD(CREATE_NO_WINDOW),
      environment,
      (Option(processBuilder.directory) map { _.toString }).orNull,
      startupInfo, processInformation)
    if (!ok) throwLastError("CreateProcessW")
    kernel32.CloseHandle(startupInfo.hStdError)
    startupInfo.hStdError = null
    new WindowsProcess(startupInfo, processInformation, stdinHandle, stdoutHandle, stderrHandle)
  }

  private def redirectToHandle(stdFile: Int, redirect: Redirect): (HANDLE, HANDLE) =
    redirect.`type` match {
      case Redirect.Type.INHERIT ⇒
        (kernel32.GetStdHandle(stdFile), null)
      case Redirect.Type.PIPE ⇒
        (new HANDLE, new HANDLE)
        //val readRef = new HANDLEByReference
        //val writeRef = new HANDLEByReference
        //val security = new SECURITY_ATTRIBUTES
        //security.bInheritHandle = true
        //kernel32.CreatePipe(readRef, writeRef, security, 4096)
        //if (!kernel32.SetHandleInformation(writeRef.getValue, HANDLE_FLAG_INHERIT, 0) )
        //  throwLastError("SetHandleInformation")
        //(readRef.getValue, writeRef.getValue)
      case Redirect.Type.WRITE ⇒
        val security = new SECURITY_ATTRIBUTES
        security.bInheritHandle = true
        val handle = kernel32.CreateFile(
          redirect.file.toString,
          GENERIC_WRITE,
          FILE_SHARE_READ /*|| FILE_SHARE_DELETE?*/,
          security,
          CREATE_ALWAYS,
          FILE_ATTRIBUTE_TEMPORARY /*|| FILE_FLAG_DELETE_ON_CLOSE?*/,
          null)
        if (handle == InvalidHANDLE) throwLastError(s"CreateFile '${redirect.file}'")
        (handle, null)
      case t ⇒ throw new IllegalArgumentException(s"Unsupported Redirect $t")
  }

  private def waitForSingleObject(handle: HANDLE, timeout: Int): Boolean = {
    kernel32.WaitForSingleObject(handle, timeout) match {
      case WAIT_OBJECT_0 ⇒ true
      case WAIT_TIMEOUT ⇒ false
      case WAIT_FAILED ⇒ throwLastError("WaitForSingleObject")
      case o ⇒ throw new WindowsException(s"WaitForSingleObject returns $o")
    }
  }

  private def getExitCode(hProcess: HANDLE): Int = {
    val ref = new IntByReference
    if (!kernel32.GetExitCodeProcess(hProcess, ref))
      throwLastError("GetExitCodeProcess")
    ref.getValue
  }

  private def throwLastError(function: String): Nothing = {
    val err = kernel32.GetLastError
    throw new WindowsException(s"WINDOWS-$err ($function) " + formatMessageFromLastErrorCode(err))
  }

  final class WindowsException private[WindowsProcess](message: String) extends RuntimeException(message)
}
