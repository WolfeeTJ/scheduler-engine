package com.sos.scheduler.taskserver.comrpc

import com.sos.scheduler.taskserver.comrpc.types.CLSID

/**
 * @author Joacim Zschimmer
 */
private[comrpc] trait ProxySerializer
extends COMSerializer {

  protected val iunknownProxyRegister: ProxyRegister

  final def writeIUnknown(iUnknown: IUnknown): Unit = {
    val (proxyId, isNew) = iunknownProxyRegister.registerIUnknown(iUnknown)
    writeInt64(proxyId.value)
    writeBoolean(isNew)
    if (isNew) {
      writeString(iUnknown.getClass.getSimpleName)
      writeUUID(CLSID.Null.uuid)
      writeInt32(0)
//      writeUUID(localProxy.clsid.uuid)
//      writeInt32(localProxy.properties.size)
//      for ((name, v) ← localProxy.properties) {
//        writeString(name)
//        writeVariant(v)
//      }
    }
  }
}
