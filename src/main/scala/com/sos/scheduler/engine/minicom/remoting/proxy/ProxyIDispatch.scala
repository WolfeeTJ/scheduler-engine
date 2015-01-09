package com.sos.scheduler.engine.minicom.remoting.proxy

import com.sos.scheduler.engine.minicom.idispatch.{DISPATCH_METHOD, DISPATCH_PROPERTYGET, DISPID, DispatchType, IDispatch}
import com.sos.scheduler.engine.minicom.remoting.calls.ProxyId

/**
 * @author Joacim Zschimmer
 */
trait ProxyIDispatch extends IDispatch {
  val id: ProxyId
  val name: String
  protected val remoting: ClientRemoting

  final def getIdOfName(name: String) = remoting.getIdOfName(id, name)

  final def invokeGet(dispId: DISPID): Any = invoke(dispId, Set(DISPATCH_PROPERTYGET))

  final def invokeMethod(dispId: DISPID, arguments: Seq[Any]): Any = invoke(dispId, Set(DISPATCH_METHOD), arguments)

  final def invoke(dispId: DISPID, dispatchTypes: Set[DispatchType], arguments: Seq[Any] = Nil) =
    remoting.invoke(id, dispId, dispatchTypes, arguments)
}
