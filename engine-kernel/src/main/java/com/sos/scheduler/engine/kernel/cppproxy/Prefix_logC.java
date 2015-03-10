package com.sos.scheduler.engine.kernel.cppproxy;

import com.sos.scheduler.engine.cplusplus.runtime.CppProxyWithSister;
import com.sos.scheduler.engine.cplusplus.runtime.annotation.CppClass;
import com.sos.scheduler.engine.kernel.log.PrefixLog;

@CppClass(clas="sos::scheduler::Prefix_log", directory="scheduler", include="spooler.h")
public interface Prefix_logC extends CppProxyWithSister<PrefixLog> {
    PrefixLog.Type sisterType = new PrefixLog.Type();

    void java_log(int level, String line);
    String java_last(String log_level);
    boolean started();
    String this_filename();
}
