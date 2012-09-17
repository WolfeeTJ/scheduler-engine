package com.sos.scheduler.engine.test.binary;

import static com.sos.scheduler.engine.kernel.util.Files.makeExecutable;
import static com.sos.scheduler.engine.test.binary.ResourcesAsFilesProvider.provideResourcesAsFiles;

import java.io.File;

import org.springframework.core.io.Resource;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.sos.scheduler.engine.main.CppBinaries;
import com.sos.scheduler.engine.main.CppBinary;
import static scala.collection.JavaConversions.*;

/** Ablage der Scheduler-Binärdateien, die nötigenfalls aus der kernelcpp.jar entladen werden. */
public final class ResourceCppBinaries implements CppBinaries {
    private final ImmutableMap<String,ResourceFile> resourceFiles;

    ResourceCppBinaries(ImmutableList<Resource> resources, File temporaryBinDirectory) {
        File directory = temporaryBinDirectory.getAbsoluteFile();
        resourceFiles = ImmutableMap.copyOf(mapAsJavaMap(provideResourcesAsFiles(iterableAsScalaIterable(resources), directory)));
        checkFiles();
        setExecutable();
    }

    private void checkFiles() {
        for (CppBinary o: CppBinary.values()) file(o);
    }

    private void setExecutable() {
        ResourceFile r = resourceFiles.get(CppBinary.exeFilename.filename());
        if (r.isCopied())
            makeExecutable(r.getFile());
    }

    boolean someResourceHasBeenCopied() {
        for (ResourceFile f: resourceFiles.values())
            if (f.isCopied()) return true;
        return false;
    }

//    public void removeFileCopies() {
//        for (ResourceFile f: resourceFiles.values()) if (f.isCopied()) {
//            File file = newCppBinaries.getFile();
//            boolean ok = file.delete();
//            if (!ok)
//                file.deleteOnExit();
//        }
//    }

    public void removeCopiesOnExit() {
        for (ResourceFile f: resourceFiles.values()) if (f.isCopied()) f.getFile().deleteOnExit();
    }

//    public Iterable<File> createdFiles() {
//        return concat(transform(resourceFiles.values(), new Function<ResourceFile,Iterable<File>>() {
//            @Override public Iterable<File> apply(ResourceFile o) {
//                if (o.isCopied()) return Collections.ingleton(o.getFile());
//                else return Collections.emptyList();
//            }
//        }));
//    }

    @Override public File directory() {
        return file(CppBinary.moduleFilename).getParentFile();
    }

    @Override public File file(CppBinary o) {
        return get(o.filename());
    }

    File get(String name) {
        assertResourceFiles();
        ResourceFile result = resourceFiles.get(name);
        if (result == null) throw new RuntimeException("Unknown binary resource "+ name);
        return result.getFile();
    }

    private void assertResourceFiles() {
        if (resourceFiles == null)
            throw new IllegalStateException("provideResourcesAsFiles() not called");
    }
}
