package com.sos.scheduler.engine.kernel.command;

import org.w3c.dom.Element;

import static com.sos.scheduler.engine.common.xml.XmlUtils.newDocument;

class BResultXmlizer extends GenericResultXmlizer<BResult> {
    static final BResultXmlizer singleton = new BResultXmlizer();

    BResultXmlizer() {
        super(BResult.class);
    }

    @Override public final Element doToElement(BResult r) {
        Element e = newDocument().createElement("bResult");
        e.setAttribute("value2", r.value);
        return e;
    }
}
