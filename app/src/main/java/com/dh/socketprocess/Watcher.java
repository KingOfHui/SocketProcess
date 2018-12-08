package com.dh.socketprocess;

public class Watcher {
    static {
        System.loadLibrary("native-lib");
    }
    public native void createWatcher(String userId);

    public native void connectMonitor();
}
