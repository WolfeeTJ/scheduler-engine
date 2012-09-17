package com.sos.scheduler.engine.plugins.js644;

import com.sos.scheduler.engine.data.folder.FileBasedActivatedEvent;
import com.sos.scheduler.engine.data.folder.JobChainPath;
import com.sos.scheduler.engine.data.folder.JobPath;
import com.sos.scheduler.engine.eventbus.HotEventHandler;
import com.sos.scheduler.engine.kernel.job.Job;
import com.sos.scheduler.engine.kernel.order.jobchain.JobChain;
import com.sos.scheduler.engine.kernel.util.Time;
import com.sos.scheduler.engine.kernel.util.sync.Gate;
import com.sos.scheduler.engine.test.SchedulerTest;
import org.junit.Test;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import static com.sos.scheduler.engine.plugins.js644.JS644PluginIT.M.jobActivated;
import static com.sos.scheduler.engine.plugins.js644.JS644PluginIT.M.jobchainActivated;
import static org.hamcrest.MatcherAssert.assertThat;

public final class JS644PluginIT extends SchedulerTest {
    private static final JobPath jobPath = JobPath.of("/a");
    private static final JobChainPath jobChainPath = JobChainPath.of("/A");
    private static final Time timeout = shortTimeout;

    enum M { jobActivated, jobchainActivated }
    private final Gate<M> gate = new Gate<M>();
    private volatile boolean schedulerIsActive = false;

    @Test public void test() throws Exception {
        controller().startScheduler();
        controller().waitUntilSchedulerIsActive();
        schedulerIsActive = true;
        modifyJobFile();
        gate.expect(jobActivated, timeout);
        gate.expect(jobchainActivated, timeout);
    }

    private void modifyJobFile() throws IOException {
        File jobFile = fileBasedFile(scheduler().getJobSubsystem().job(jobPath));
        assertThat(jobFile + " does not exist", jobFile.exists());
        OutputStream out = new FileOutputStream(jobFile, true);
        try {
            out.write(' ');
        } finally {
            out.close();
        }
    }

    @HotEventHandler public void handleEvent(FileBasedActivatedEvent e, Job job) throws InterruptedException {
        if (schedulerIsActive && job.getPath().equals(jobPath))
            gate.put(jobActivated);
    }

    @HotEventHandler public void handleEvent(FileBasedActivatedEvent e, JobChain jobChain) throws InterruptedException {
        if (schedulerIsActive && jobChain.getPath().equals(jobChainPath))
            gate.put(jobchainActivated);
    }

    private File fileBasedFile(Job o) {
        return new File(controller().environment().configDirectory(), o.getPath().asString() + ".job.xml");
    }
}
