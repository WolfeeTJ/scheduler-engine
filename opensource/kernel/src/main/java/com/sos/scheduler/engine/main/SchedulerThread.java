package com.sos.scheduler.engine.main;

import com.google.common.collect.ImmutableList;
import com.sos.scheduler.engine.kernel.CppScheduler;
import com.sos.scheduler.engine.kernel.scheduler.SchedulerException;

import java.io.File;
import java.util.concurrent.atomic.AtomicReference;

/** Der Scheduler in einem eigenen Thread. */
class SchedulerThread extends Thread {
    private final SchedulerControllerBridge controllerBridge;
    private final CppScheduler cppScheduler = new CppScheduler();
    private ImmutableList<String> arguments = null;
    private final AtomicReference<Integer> exitCodeAtom = new AtomicReference<Integer>();

    SchedulerThread(SchedulerControllerBridge controllerBridge) {
        this.controllerBridge = controllerBridge;
        setName("Scheduler");
    }

    static void loadModule(File f) {
        CppScheduler.loadModule(f);
    }

    final void startThread(ImmutableList<String> args) {
        this.arguments = args;
        start();  // Thread läuft in run()
    }

    @Override public final void run() {
        int exitCode = -1;
        Throwable throwable = null;
        try {
            exitCode = cppScheduler.run(arguments, "", controllerBridge);
            exitCodeAtom.set(exitCode);
            if (exitCode != 0)
                throwable = new SchedulerException("Scheduler terminated with exit code " + exitCode);
        }
        catch (Exception x) {
            throwable = x;
        }
        catch (Error x) {
            throwable = x;
            throw x;
        }
        finally {
            controllerBridge.onSchedulerTerminated(exitCode, throwable);
        }
    }

    final int exitCode() {
        if (isAlive())  throw new IllegalStateException("Thread is still alive");
        return exitCodeAtom.get();
    }
}
