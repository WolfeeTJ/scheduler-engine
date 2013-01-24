package com.sos.scheduler.engine.test.util;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.channels.OverlappingFileLockException;
import java.util.Observable;

public class LockFileThread extends Observable implements Runnable  {

	private static final Logger logger = LoggerFactory.getLogger(LockFileThread.class);
	
	private final File file;
	private final int duration;
	private boolean isAlreadyLocked;
	
	public LockFileThread(File fileToLock, int durationInSeconds) {
		file = fileToLock;
		duration = durationInSeconds;
		logger.debug("File {} will be locked for {} seconds", file, duration);
	}
	
	@Override
	public void run() {
		isAlreadyLocked = false;
		FileInputStream in = null;
		try {
			in = new FileInputStream(file);
		    java.nio.channels.FileLock lock = in.getChannel().tryLock(0L, Long.MAX_VALUE, true);
		    try {
		    	Thread.sleep(duration*1000);
		    } catch (InterruptedException e) {
		    	tellObserver("sleep interrupted",e);
			} finally {
		        lock.release();
		    }
		} catch (FileNotFoundException e) {
			String message = "file '" + file.getAbsolutePath() + "' does not exist";
	    	tellObserver(message,e);
		} catch (OverlappingFileLockException e) {
			isAlreadyLocked = true;
	    	tellObserver("file " + file.getAbsolutePath() + " is already locked",e);
		} catch (IOException e) {
	    	tellObserver(e.getMessage(),e);
		} finally {
		    if (in != null)
				try {
					in.close();
				} catch (IOException e) {
					String message = "file '" + file.getAbsolutePath() + "' could not be closed";
			    	tellObserver(message, e);
				}
		}
		logger.debug("File {} unlocked", file);
	}
	
	private void tellObserver(String message, Exception e) {
    	logger.error(message);
    	logger.trace(message, e);
		setChanged();
		notifyObservers( getException(message,e) );
	}
	
	private RuntimeException getException(String message, Exception e) {
    	if (e instanceof OverlappingFileLockException)
    		return new FileAlreadyLockedException(message,e);
   		return new RuntimeException(message,e);
	}
	
	public boolean isAlreadyLocked() {
		return isAlreadyLocked;
	}
}
