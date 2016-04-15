package com.sos.scheduler.engine.kernel.util;

import com.sos.scheduler.engine.base.generic.StringValue;
import org.junit.Test;

public final class StringValueTest {
    @Test public void testEquals1() {
        A a = new A("a");
        B b = new B("b");
        assert !a.equals(b);
    }

    @Test public void testEquals2() {
        A a = new A("x");
        B b = new B("x");
        assert !a.equals(b);
    }

    @Test public void testEquals3() {
        A a1 = new A("a1");
        A a2 = new A("a2");
        assert !a1.equals(a2);
    }

    @Test public void testEquals4() {
        A a1 = new A("a");
        A a2 = new A("a");
        assert a1.equals(a2);
    }

    @Test public void testSet() {
        //TODO hashValue() mit Set testen
    }

    static class A extends StringValue {
        A(String s) {
            super(s);
        }
    }

    static class B extends StringValue {
        B(String s) {
            super(s);
        }
    }
}
