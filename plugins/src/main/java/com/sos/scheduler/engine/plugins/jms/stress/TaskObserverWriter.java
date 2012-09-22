package com.sos.scheduler.engine.plugins.jms.stress;

import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

public class TaskObserverWriter extends TaskObserver implements TaskInfoListener {

	private final String filename;
	private final PrintWriter out;

	public TaskObserverWriter(String filename, TaskInfo classInTest) throws IOException {
		super(classInTest);
		this.filename = filename;
		FileWriter outFile = new FileWriter(this.filename);
		out = new PrintWriter(outFile);
		out.println("duration;running;highwater;ended;estimated");
	}
	
	public void close() {
		out.close();
	}

	@Override
	public void onInterval(TaskInfo info) {
		out.println(runningSince() + ";" + info.currentlyRunningTasks() + ";" + info.highwaterTasks() + ";" + info.endedTasks() + ";" + info.estimatedTasks());
	}
	
}
