package com.sos.scheduler.engine.data.folder;

import com.fasterxml.jackson.annotation.JsonCreator;

public class JobPath extends TypedPath {
    public JobPath(AbsolutePath path) {
        super(FileBasedType.job, path);
    }

    @JsonCreator
    public static JobPath of(String absolutePath) {
        return new JobPath(new AbsolutePath(absolutePath));
    }
}