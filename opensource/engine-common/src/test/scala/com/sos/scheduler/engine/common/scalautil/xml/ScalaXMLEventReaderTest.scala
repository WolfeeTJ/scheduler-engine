package com.sos.scheduler.engine.common.scalautil.xml

import com.sos.scheduler.engine.common.scalautil.ScalaUtils._
import com.sos.scheduler.engine.common.scalautil.xml.ScalaXMLEventReader._
import com.sos.scheduler.engine.common.scalautil.xml.ScalaXMLEventReaderTest._
import org.junit.runner.RunWith
import org.scalatest.FreeSpec
import org.scalatest.Matchers._
import org.scalatest.junit.JUnitRunner
import scala.collection.immutable

/**
 * @author Joacim Zschimmer
 */
@RunWith(classOf[JUnitRunner])
final class ScalaXMLEventReaderTest extends FreeSpec {
  "Methods" in {
    case class X(y: Y, z: immutable.Seq[Z])
    trait T
    case class Y() extends T
    case class Z() extends T
    val x = parseString(<X><Y/><Z/><Z/></X>.toString()) { eventReader ⇒
      import eventReader._
      parseElement("X") {
        val children = forEachStartElement {
          case "Y" ⇒ parseElement() { Y() }
          case "Z" ⇒ parseElement() { Z() }
        }

        (children.values: immutable.IndexedSeq[T]) shouldEqual List(Y(), Z(), Z())

        (children.one[Y]("Y"): Y) shouldEqual Y()
        (children.one[Y]: Y)  shouldEqual Y()
        (children.option[Y]("Y"): Option[Y]) shouldEqual Some(Y())
        (children.option[Y]: Option[Y])  shouldEqual Some(Y())
        (children.byClass[Y]: immutable.Seq[Y]) shouldEqual List(Y())
        (children.byClass[Y]: immutable.Seq[Y]) shouldEqual List(Y())
        (children.byName[Y]("Y"): immutable.Seq[Y]) shouldEqual List(Y())

        intercept[IllegalArgumentException] { children.one[Z]("Z") }
        intercept[IllegalArgumentException] { children.one[Z] }
        intercept[IllegalArgumentException] { children.option[Z]("Z") }
        intercept[IllegalArgumentException] { children.option[Z] }
        (children.byClass[Z]: immutable.Seq[Z]) shouldEqual List(Z(), Z())
        (children.byClass[Z]: immutable.Seq[Z]) shouldEqual List(Z(), Z())
        (children.byName[Z]("Z"): immutable.Seq[Z]) shouldEqual List(Z(), Z())

        intercept[ClassCastException] { children.one[Y]("Z") }
        intercept[ClassCastException] { children.option[Y]("Z") }
        intercept[ClassCastException] { children.byName[Y]("Z") }

        X(children.one[Y], children.byClass[Z])
      }
    }
    x shouldEqual X(Y(), List(Z(), Z()))
  }

  "ScalaXMLEventReader" in {
    val testXmlString = <A><B/><C x="xx" optional="oo"><D/><D/></C></A>.toString()
    parseString(testXmlString)(parseA) shouldEqual A(B(), C(x = "xx", o = "oo", List(D(), D())))
  }

  "Optional attribute" in {
    val testXmlString = <A><B/><C x="xx"><D/><D/></C></A>.toString()
    parseString(testXmlString)(parseA) shouldEqual A(B(), C(x = "xx", o = "DEFAULT", List(D(), D())))
  }

//  "parseElementAsXmlString" in {
//    val testXmlString = <A><B b="b">text<C/></B><B/></A>.toString()
//    assertResult("""<B b="b">text<C/></B>, <B/>""") {
//      parseString(testXmlString) { eventReader ⇒
//        import eventReader._
//        parseElement("A") {
//          val children = forEachStartElement {
//            case "B" ⇒ parseElementAsXmlString()
//          }
//          children("B") mkString " ,"
//        }
//      }
//    }
//  }

  "Detects extra attribute" in {
    val testXmlString = <A><B/><C x="xx" optional="oo" z="zz"><D/><D/></C></A>.toString()
    intercept[WrappedException] { parseString(testXmlString)(parseA) }
      .rootCause.asInstanceOf[UnparsedAttributesException].names shouldEqual List("z")
  }

  "Detects missing attribute" in {
    val testXmlString = <A><B/><C><D/><D/></C></A>.toString()
    intercept[WrappedException] { parseString(testXmlString)(parseA) }
      .rootCause.asInstanceOf[NoSuchElementException]
  }

  "Detects extra element" in {
    val testXmlString = <A><B/><C x="xx"><D/><D/></C><EXTRA/></A>.toString()
    intercept[WrappedException] { parseString(testXmlString)(parseA) }
  }

  "Detects extra repeating element" in {
    val testXmlString = <A><B/><C x="xx"><D/><D/><EXTRA/></C></A>.toString()
    intercept[WrappedException] { parseString(testXmlString)(parseA) }
  }

  "Detects missing element" in {
    val testXmlString = <A><C x="xx"><D/><D/></C></A>.toString()
    intercept[Exception] { parseString(testXmlString)(parseA) }
      .rootCause.asInstanceOf[NoSuchElementException]
  }
}

private object ScalaXMLEventReaderTest {
  private case class A(b: B, c: C)
  private case class B()
  private case class C(x: String, o: String, ds: immutable.Seq[D])
  private case class D()

  private def parseA(eventReader: ScalaXMLEventReader): A = {
    import eventReader._

    def parseC(): C =
      parseElement("C") {
        val x = attributeMap("x")
        val o = attributeMap.getOrElse("optional", "DEFAULT")
        val ds = parseEachRepeatingElement("D") { D() }
        C(x, o, ds.to[immutable.Seq])
      }

    parseElement("A") {
      val children = forEachStartElement {
        case "B" ⇒ parseElement() { B() }
        case "C" ⇒ parseC()
      }
      A(children.one[B]("B"), children.one[C])
    }
  }
}