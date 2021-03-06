package com.sos.scheduler.engine.data.order

import com.sos.jobscheduler.data.agent.AgentAddress
import com.sos.jobscheduler.data.job.TaskId
import com.sos.scheduler.engine.data.filebased.{FileBasedObstacle, FileBasedState}
import com.sos.scheduler.engine.data.jobchain.{JobChainPath, NodeId}
import com.sos.scheduler.engine.data.order.OrderObstacle.FileBasedObstacles
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
final class OrderOverviewTest extends FreeSpec {

  "JSON" in {
    val path = JobChainPath("/outer") orderKey "1"
    val obj = OrderOverview(
      path,
      FileBasedState.active,
      OrderSourceType.AdHoc,
      JobChainPath("/inner"),
      NodeId("100"),
      OrderProcessingState.InTaskProcess(TaskId(123), ProcessClassPath("/TEST"),
        Instant.parse("2016-08-01T01:02:03.044Z"), Some(AgentAddress("http://1.2.3.4:5678"))),
      Some(OrderHistoryId(22)),
      ListSet(
        OrderObstacle.Suspended,
        OrderObstacle.Setback(Instant.parse("2016-08-02T11:22:33.444Z")),
        FileBasedObstacles(Set(FileBasedObstacle.Replaced()))),
      startedAt = Some(Instant.parse("2016-07-18T11:11:11Z")),
      nextStepAt = Some(Instant.parse("2016-07-18T12:00:00Z")),
      outerJobChainPath = Some(JobChainPath("/OUTER")))
    val jsValue = """{
      "path": "/outer,1",
      "fileBasedState": "active",
      "historyId": 22,
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
        },
        {
          "TYPE": "FileBasedObstacles",
          "fileBasedObstacles": [
            {
              "TYPE": "Replaced"
            }
          ]
        }
      ],
      "startedAt": "2016-07-18T11:11:11Z",
      "nextStepAt": "2016-07-18T12:00:00Z",
      "outerJobChainPath": "/OUTER"
    }""".parseJson
    assert(obj.toJson == jsValue)
    assert(obj == jsValue.convertTo[OrderOverview])
  }
}
