package com.sos.scheduler.engine.data.order

import com.sos.jobscheduler.data.agent.AgentAddress
import com.sos.jobscheduler.data.job.TaskId
import com.sos.scheduler.engine.data.filebased.FileBasedState
import com.sos.scheduler.engine.data.jobchain.{JobChainPath, NodeId}
import com.sos.scheduler.engine.data.processclass.ProcessClassPath
import java.time.Instant
import org.junit.runner.RunWith
import org.scalatest.FreeSpec
import org.scalatest.junit.JUnitRunner
import scala.collection.immutable.ListSet
import spray.json._

/**
  * @author Joacim Zschimmer
  */
@RunWith(classOf[JUnitRunner])
final class OrderDetailedTest extends FreeSpec {

  "JSON" in {
    val obj = OrderDetailed(
      OrderOverview(
        JobChainPath("/outer") orderKey "1",
        FileBasedState.active,
        OrderSourceType.AdHoc,
        JobChainPath("/inner"),
        NodeId("100"),
        OrderProcessingState.InTaskProcess(
          TaskId(123),
          ProcessClassPath("/TEST"),
          Instant.parse("2016-08-01T01:02:03.044Z"),
          Some(AgentAddress("http://1.2.3.4:5678"))),
        obstacles = ListSet(OrderObstacle.Suspended, OrderObstacle.Setback(Instant.parse("2016-08-02T11:22:33.444Z"))),
        nextStepAt = Some(Instant.parse("2016-07-18T12:00:00Z"))),
      priority = 7,
      initialNodeId = Some(NodeId("INITIAL")),
      endNodeId = Some(NodeId("END")),
      stateText = "STATE-TEXT",
      title = "TITLE",
      variables = Map("a" → "A", "b" → "B"))
    val jsValue = """{
      "overview": {
        "path": "/outer,1",
        "fileBasedState": "active",
        "orderSourceType": "AdHoc",
        "jobChainPath": "/inner",
        "nodeId": "100",
        "orderProcessingState": {
          "TYPE": "InTaskProcess",
          "taskId": "123",
          "processClassPath": "/TEST",
          "since": "2016-08-01T01:02:03.044Z",
          "agentUri": "http://1.2.3.4:5678"
        },
        "obstacles": [
          {
            "TYPE": "Suspended"
          },
          {
            "TYPE": "Setback",
            "until": "2016-08-02T11:22:33.444Z"
          }
        ],
        "nextStepAt": "2016-07-18T12:00:00Z"
      },
      "priority": 7,
      "initialNodeId": "INITIAL",
      "endNodeId": "END",
      "stateText": "STATE-TEXT",
      "title": "TITLE",
      "variables": {
        "a": "A",
        "b": "B"
      }
    }""".parseJson
    assert(obj.toJson == jsValue)
    assert(obj == jsValue.convertTo[OrderDetailed])
  }
}
