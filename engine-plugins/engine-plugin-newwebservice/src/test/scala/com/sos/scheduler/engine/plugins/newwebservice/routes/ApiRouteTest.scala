package com.sos.scheduler.engine.plugins.newwebservice.routes

import com.sos.scheduler.engine.plugins.newwebservice.html.WebServiceContext
import org.junit.runner.RunWith
import org.scalatest.junit.JUnitRunner
import scala.concurrent.ExecutionContext
import spray.http.HttpHeaders.{Accept, Location}
import spray.http.MediaTypes._
import spray.http.{MediaRanges, StatusCodes, Uri}
import spray.routing.Directives._
import spray.testkit.ScalatestRouteTest

/**
  * @author Joacim Zschimmer
  */
@RunWith(classOf[JUnitRunner])
final class ApiRouteTest extends org.scalatest.FreeSpec with ScalatestRouteTest with ApiRoute {

  protected def client = throw new NotImplementedError

  protected def executionContext = ExecutionContext.global

  protected def webServiceContext = new WebServiceContext(htmlEnabled = true)

  def route = pathPrefix("test" / "api") {
    apiRoute
  }

  "/" in {
    Get("/test/api/") ~> Accept(MediaRanges.`*/*`) ~> route ~> check {
      assert(!handled)
    }
    Get("/test/api/") ~> Accept(`text/html`) ~> route ~> check {
      assert(status == StatusCodes.TemporaryRedirect)
      assert((response.headers collect { case Location(o) ⇒ o }) == List(Uri("/test/api")))
    }
  }
}
