package com.sos.scheduler.engine.data.folder

trait AbsolutePath
extends Path {

  @Deprecated
  final def asString =
    string

  @Deprecated
  final def getName =
    name

  final def name: String = {
    val s = string
    s.substring(s.lastIndexOf('/') + 1)
  }

  final def withTrailingSlash: String = {
    val r = string
    if (r endsWith "/") r else r + "/"
  }

  final def withoutStartingSlash: String = {
    val s = string
    assert(s startsWith "/")
    s.substring(1)
  }
}


object AbsolutePath {

  /** @param path ist absolut oder relativ zur Wurzel. */
  @Deprecated
  def of(path: String): AbsolutePath =
    new AbsolutePath { def string = makeAbsolute(path) }

  def makeAbsolute(path: String): String =
    if (path.startsWith("/")) path else "/" + path

  @Deprecated
  def of(parentPath: AbsolutePath, subpath: String): AbsolutePath = {
    val a = new StringBuilder
    a.append(stripTrailingSlash(parentPath.string))
    a.append('/')
    a.append(subpath)
    of(a.toString())
  }

  private def stripTrailingSlash(a: String): String =
    if (a.endsWith("/")) a.substring(0, a.length - 1) else a
}
