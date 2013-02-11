package com.sos.scheduler.engine.kernel.scheduler;

import com.sos.scheduler.engine.data.scheduler.ClusterMemberId;
import com.sos.scheduler.engine.data.scheduler.SchedulerId;
import com.sos.scheduler.engine.kernel.cppproxy.SpoolerC;
import com.sos.scheduler.engine.kernel.settings.SettingName;

import javax.inject.Inject;
import javax.inject.Singleton;
import java.io.File;

import static com.google.common.base.Objects.firstNonNull;
import static com.google.common.base.Strings.isNullOrEmpty;
import static com.sos.scheduler.engine.kernel.settings.SettingName.htmlDir;

@Singleton
public final class SchedulerConfiguration {
    private static final File workingDirectory = new File(".");

    private final SpoolerC spoolerC;

    @Inject private SchedulerConfiguration(SpoolerC spoolerC) {
        this.spoolerC = spoolerC;
    }

    public ClusterMemberId clusterMemberId() {
        return new ClusterMemberId(spoolerC.cluster_member_id());
    }

    public File mainConfigurationDirectory() {
        return firstNonNull(mainConfigurationFile().getParentFile(), workingDirectory);
    }

    public File mainConfigurationFile() {
        return new File(spoolerC.configuration_file_path());
    }

    /** Das Verzeichnis der Konfigurationsdatei scheduler.xml, Normalerweise "config" */
    public File localConfigurationDirectory() {
        return new File(spoolerC.local_configuration_directory());
    }

    public File logDirectory() {
        String result = spoolerC.log_directory();
        if (isNullOrEmpty(result) || result.equals("*stderr")) throw new SchedulerException("Scheduler runs without a log directory");
        return new File(result);
    }

    public SchedulerId schedulerId() {
        return new SchedulerId(spoolerC.id());
    }

//    public URL httpUrl() {
//        return new URL(spoolerC.http_url());
//    }

    public int tcpPort() {
        return spoolerC.tcp_port();
    }

    String setting(SettingName name) {
        return spoolerC.setting(name.getNumber());
    }

    public String webDirectory() {
        return setting(htmlDir);
    }
}
