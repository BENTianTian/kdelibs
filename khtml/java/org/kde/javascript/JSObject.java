package org.kde.javascript;

import java.applet.Applet;
import org.kde.kjas.server.*;

public class JSObject extends netscape.javascript.JSObject {
    public String returnvalue = null;
    public Thread thread;

    private String jsobject;
    private int id;
    private Applet applet;
    private String appletID = null;

    /* JavaScript code:
     * __lc=[[JS objects],call func(index,script,appletname,isglobal)]
     */
    private final String decls = "if(!window.__lc) window.__lc=[[window],function(i,s,a,g){var r;var len=window.__lc[0].length;if(i>=len)r='E ';else{try{r=eval((g?'':'window.__lc[0][i]')+s);}catch(e){r='E ';}finally{var t=typeof r;var v;if(t=='undefined')v='V ';else if(t=='number')v='N '+r;else if(t=='string')if(r=='E ')v='E ';else v='S '+r;else{window.__lc[0][len]=r;v=''+len+' '+(r==window.__lc?'[array]':r);}}}a.__lc_ret=v},0]";

    public JSObject(Applet a, String name, int _id) {
        Main.info("JSObject.ctor: " + name);
        jsobject = new String(name);
        applet = a;
        id = _id;
        KJASAppletContext kc = (KJASAppletContext)applet.getAppletContext();
        appletID = kc.getAppletID(a);
        if (id == 0) {
            kc.evaluateJavaScript(decls, appletID, null);
        } 
    }

    int getId() {
        return id;
    }

    private String escapeString(String string) {
        // the following line only works from java 1.4 on
        //String string = _script.replaceAll("\\\\", "\\\\\\\\\\\\\\\\").replaceAll("\"", "\\\\\\\\\\\\\\\"");

        // and this is the replacement for older versions:
        StringBuffer sb = new StringBuffer();
        int idx;
        int off;

        for (off = 0; (idx = string.indexOf("\\", off)) >= 0; off = idx + 1) {
            sb.append(string.substring(off, idx));
            sb.append("\\\\");
        }
        sb.append(string.substring(off, string.length()));
        string = sb.toString();
        sb = new StringBuffer();
        for (off = 0; (idx = string.indexOf("\"", off)) >= 0; off = idx + 1) {
            sb.append(string.substring(off, idx));
            sb.append("\\\"");
        }
        sb.append(string.substring(off, string.length()));
        // end of replacement
        return sb.toString();
    }

    private Object evaluate(String script, boolean global) throws netscape.javascript.JSException {
        Main.info("evaluate (\"" + script + "\")");
        if (!applet.isActive()) {
            Main.debug("evaluate on not active applet");
            return null;
        }
         
        KJASAppletContext kc = (KJASAppletContext) applet.getAppletContext();
        //String appletname = kc.getAppletName(appletID);
        thread = Thread.currentThread();

        kc.evaluateJavaScript("window.__lc[1](" + id + ",\"" + escapeString(script) + "\",this" + (global ? ",true)" : ")"), appletID, this);
        boolean timedout = true;
        try {
            Thread.currentThread().sleep(30000);
        } catch (InterruptedException ex) {
            timedout = false;
        }
        thread = null;
        if (timedout || returnvalue == null)
            return null;

        /* lets see what we've got */
        String retval = returnvalue;
        int pos = retval.indexOf(' ');
        String type = retval.substring(0, pos);
        if (type.equals("E")) // Error
            throw new netscape.javascript.JSException("Script error");
        if (type.equals("V")) // Void
            return null;
        String value = retval.substring(pos+1);
        Main.info("value=" + value + " (type=" + type + ")");
        if (type.equals("N")) // Number
            return new Double(value);
        if (type.equals("S")) // String
            return value;

        /* Is it an applet? */
        if (value.startsWith("[object APPLET ref=")) {
            int p1 = value.indexOf('=');
            int p2 = value.indexOf(']', p1+1);
            int applethashcode = Integer.parseInt(value.substring(p1+1, p2));
            java.util.Enumeration e = kc.getApplets();
            while (e.hasMoreElements()) {
                Applet applet = (Applet) e.nextElement();
                if (applet.hashCode() == applethashcode)
                    return applet;
            }
            return null;
        }
        /* Is it a Java object then? */
        if (value.startsWith("[object ") && value.indexOf("ref=") > 0) {
            int p1 = value.indexOf("ref=");
            int p2 = value.indexOf(']', p1+4);
            int objecthashcode = Integer.parseInt(value.substring(p1+4, p2));
            return kc.getJSReferencedObject(objecthashcode);
        }
        /* Ok, make it a JSObject */
        return new JSObject(applet, value, Integer.parseInt(type));
    }
    private String convertValueJ2JS(Object o) {
        if (o == null)
            return new String("null");
        if (o instanceof java.lang.Number || o instanceof java.lang.Boolean)
            return o.toString();
        if (o instanceof netscape.javascript.JSObject)
            return new String("window.__lc[0][" + ((JSObject)o).getId() + "]");
        return new String("\"" + escapeString(o.toString()) + "\"");
    }
    public Object call(String func, Object [] args) throws netscape.javascript.JSException {
        Main.info("JSObject.call: " + jsobject + "." + func);
        String script = new String("." + func + "(");
        for (int i = 0; i < args.length; i++)
            script += (i > 0 ? "," : "") + convertValueJ2JS(args[i]);
        script += ")";
        return evaluate(script, false);
    }
    public Object eval(String s) throws netscape.javascript.JSException {
        return evaluate(s, true);
    }
    public boolean equals(Object obj) {
        Main.info("JSObject.equals");
        return super.equals(obj);
    }
    public Object getMember(String name) throws netscape.javascript.JSException {
        Main.info("JSObject.getMember: " + jsobject + "." + name);
        return evaluate("." + name, false);
    }
    public void setMember(String name, java.lang.Object o) throws netscape.javascript.JSException {
        Main.info("JSObject.setMember: " + jsobject + "." + name);
        evaluate("." + name + "=" + convertValueJ2JS(o), false);
    }
    public void removeMember(String name) throws netscape.javascript.JSException {
        Main.info("JSObject.removeMember: " + jsobject + "." + name);
        evaluate("." + name + "=null", false);
    }
    /* get array element; JS: this[index] */
    public Object getSlot(int index)throws netscape.javascript.JSException {
        Main.info("JSObject.getSlot: " + jsobject + "[" + index + "]");
        return evaluate("[" + index + "]", false);
    }
    public void setSlot(int index, Object o) throws netscape.javascript.JSException {
        Main.info("JSObject.setSlot: " + jsobject + "[" + index + "]");
        evaluate("[" + index + "]=" + convertValueJ2JS(o), false);
    }
    public String toString(){
        Main.info("JSObject.toString: " + jsobject);
        return new String(jsobject);
    }
    public static JSObject getWindow(Applet a, int dummy) {
        Main.info("JSObject.getWindow");
        return new JSObject(a, "[WINDOW]", 0);
    }
}
