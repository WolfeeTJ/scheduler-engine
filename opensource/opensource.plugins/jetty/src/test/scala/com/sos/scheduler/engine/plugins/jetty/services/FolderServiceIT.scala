package com.sos.scheduler.engine.plugins.jetty.services

import com.sos.scheduler.engine.plugins.jetty.JettyPlugin
import com.sos.scheduler.engine.plugins.jetty.JettyPluginTests._
import com.sos.scheduler.engine.test.scala.ScalaSchedulerTest
import java.io.Reader
import javax.ws.rs.core.MediaType._
import org.junit.runner.RunWith
import org.scalatest.junit.JUnitRunner
import com.fasterxml.jackson.databind.ObjectMapper

@RunWith(classOf[JUnitRunner])
final class FolderServiceIT extends ScalaSchedulerTest {
  override val configurationPackage = classOf[JettyPlugin].getPackage
  private lazy val resource = javaResource(injector).path("folder")
  private val objectMapper = new ObjectMapper

  test("Read job list as JSON") {
    val reader = resource.queryParam("type", "job").queryParam("folder", "/").accept(APPLICATION_JSON_TYPE).get(classOf[Reader])
    val tree = objectMapper.readValue(reader, classOf[java.util.Map[String,Object]])
    assert(tree.get("folderPath") === "/")
    assert(tree.get("typeName") === "job")
    val names = tree.get("entries").asInstanceOf[java.util.List[_]]
    assert(names.size > 0)
  }

//  ignore("Read job list as XML") {
//    val doc = resource.queryParam("type", "job").queryParam("folder", "/").accept(TEXT_XML_TYPE).get(classOf[Document])
//    assert(stringXPath(doc, "/names/name[@name='a']/@uri") != "")
//  }

//  test("Read job list as HTML") {
//    val result = resource.queryParam("type", "job").queryParam("folder", "/").accept(TEXT_HTML_TYPE).get(classOf[String])
//    assert(result contains "<html")
//    assert(result contains "/a")
//  }
}
