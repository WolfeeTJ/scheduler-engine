package com.sos.scheduler.engine.cplusplus.generator.javaproxy.clas

import com.sos.scheduler.engine.cplusplus.generator.Configuration
import com.sos.scheduler.engine.cplusplus.generator.cpp._
import com.sos.scheduler.engine.cplusplus.generator.javaproxy.procedure._
import com.sos.scheduler.engine.cplusplus.generator.util.ClassOps._
import com.sos.scheduler.engine.cplusplus.generator.util._
import com.sos.scheduler.engine.cplusplus.runtime.CppProxy

class CppClass(val javaClass: Class[_], val knownClasses: Set[Class[_]]) extends CppCode {
  private val isCppProxy = classOf[CppProxy] isAssignableFrom javaClass
  private val suppressMethods = isCppProxy

  val name = CppName(javaClass)
  val cppClassClass = new CppClassClass(this)
  val cppObjectClass = if (javaClass == classOf[String]) new StringCppObjectClass(this)  else new CppObjectClass(this)

  require(!(name.fullName contains '$'), s"Class $name Dollar-sign '$$' not allowed in C++ (for example gcc on AIX)")

  val cppConstructors = {
    val signatures = validConstructors(javaClass) filter { c => parameterTypesAreKnown(c.getParameterTypes) } map ProcedureSignature.apply
    signatures.sorted map { new CppConstructor(this, _) }
  }

  val cppMethods = {
    val signatures = if (suppressMethods) Nil else
      (validMethods(javaClass) filter { m => parameterTypesAreKnown(m.getParameterTypes) && returnTypeIsKnown(m.getReturnType) }
        map ProcedureSignature.apply)
    signatures.sorted map { new CppMethod(this, _) }
  }

  val cppProcedures = cppConstructors ++ cppMethods

  val directlyUsedJavaClasses: Set[Class[_]] =
    ( ClassOps.directlyUsedJavaClasses(javaClass) filter { t => typeIsValidClass(t) && t != javaClass && returnTypeIsKnown(t) } ) +
        classOf[String]  // Für obj_name()

  private def parameterTypesAreKnown(types: Seq[Class[_]]) =
    types forall parameterTypeIsKnown

  private def parameterTypeIsKnown(t: Class[_]) =
    returnTypeIsKnown(t)

  private def returnTypeIsKnown(t: Class[_]) =
    t.isPrimitive || classIsByteArray(t) || typeIsValidClass(t) && knownClasses.contains(t)

  def headerPreprocessorMacro =
    ("_" + Configuration.generatedJavaProxyNamespace.simpleName + "_" + javaClass.getName.replace('.', '_') + "_H_").toUpperCase

  def neededForwardDeclarations = (directlyUsedJavaClasses map forwardDeclaration).toSeq.sorted.mkString

  private def forwardDeclaration(c: Class[_]) = {
    val cppName = CppName(c)
    cppName.namespace.nestedCode("struct " + cppName.simpleName + "; ")
  }

  def javaSuperclasses =
    superclasses(javaClass)

  override def headerCode =
    name.namespace.nestedCode(
      cppObjectClass.forwardDeclaration + "\n" +
      cppClassClass.headerCode + "\n\n" +
      cppObjectClass.headerCode
    )

  override def sourceCode =
    name.namespace.nestedCode(
      cppClassClass.sourceCode + "\n\n" +
      cppObjectClass.sourceCode
    )
}
