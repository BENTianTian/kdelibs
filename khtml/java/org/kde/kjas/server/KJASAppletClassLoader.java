package org.kde.kjas.server;

import java.net.*;
import java.io.*;
import java.util.*;
import java.util.zip.*;
import java.util.jar.*;
import java.security.*;

/**
 * ClassLoader used to download and instantiate Applets.
 * <P>
 * NOTE: The class loader extends Java 1.2 specific class.
 */
public class KJASAppletClassLoader
    extends URLClassLoader
{
    private static Hashtable loaders = new Hashtable();
    public static KJASAppletClassLoader getLoader( String docBase, String codeBase )
    {
        URL docBaseURL;
        KJASAppletClassLoader loader = null;
        try
        {
            docBaseURL = new URL( docBase );
        
            URL codeBaseURL = getCodeBaseURL( docBaseURL, codeBase );
            Main.debug( "CL: getLoader: key = " + codeBaseURL );

            loader = (KJASAppletClassLoader) loaders.get( codeBaseURL.toString() );
            if( loader == null )
            {
                URL [] urlList = {};
                loader = new KJASAppletClassLoader( urlList, docBaseURL, codeBaseURL);
                loaders.put( codeBaseURL.toString(), loader );
            }
            else
            {
                Main.debug( "CL: reusing classloader" );
                loader.setActive();
            }
        } catch( MalformedURLException e ) { Main.kjas_err( "bad DocBase URL", e ); }
        return loader;
    }

    public static URL getCodeBaseURL( URL docBaseURL, String codeBase )
    {
        URL codeBaseURL = null;
        try
        {
            //first determine what the real codeBase is: 3 cases
            //#1. codeBase is absolute URL- use that
            //#2. codeBase is relative to docBase, create url from those
            //#3. last resort, use docBase as the codeBase
            if(codeBase != null)
            {
                //we need to do this since codeBase should be a directory
                if( !codeBase.endsWith("/") )
                    codeBase = codeBase + "/";

                try
                {
                    codeBaseURL = new URL( codeBase );
                } catch( MalformedURLException mue )
                {
                    try
                    {
                        codeBaseURL = new URL( docBaseURL, codeBase );
                    } catch( MalformedURLException mue2 ) {}
                }
            }

            if(codeBaseURL == null)
            {
                //fall back to docBase but fix it up...
                String file = docBaseURL.getFile();
                if( file == null || (file.length() == 0)  )
                    codeBaseURL = docBaseURL;
                else if( file.endsWith( "/" ) )
                    codeBaseURL = docBaseURL;
                else
                {
                    //delete up to the ending '/'
                    String urlString = docBaseURL.toString();
                    int dot_index = urlString.lastIndexOf( '/' );
                    String newfile = urlString.substring( 0, dot_index+1 );
                    codeBaseURL = new URL( newfile );
                }
            }
        }catch( Exception e ) { Main.kjas_err( "CL: exception ", e ); }
        return codeBaseURL;    
    }

    public static KJASAppletClassLoader getLoader( String key )
    {
        if( loaders.containsKey( key ) )
            return (KJASAppletClassLoader) loaders.get( key );
        
        return null;
    }

    /*********************************************************************************
     ****************** KJASAppletClassLoader Implementation *************************
     **********************************************************************************/
    private URL docBaseURL;
    private URL codeBaseURL;
    private Vector archives;
    private Hashtable rawdata;
    private Hashtable certificates;
    private boolean archives_loaded;
    private int archive_count;
    private String dbgID;
    private boolean active;
    private KJASAppletContext appletContext = null;
    
    public KJASAppletClassLoader( URL[] urlList, URL _docBaseURL, URL _codeBaseURL)
    {
        super(urlList);
        docBaseURL   = _docBaseURL;
        codeBaseURL  = _codeBaseURL;
        archives     = new Vector();
        rawdata      = new Hashtable();
        certificates = new Hashtable();
        
        archives_loaded = false;
        archive_count   = 0;
        active          = true;
        appletContext   = null;

        dbgID = "CL(" + codeBaseURL.toString() + "): ";
    }
    
    public void setAppletContext(KJASAppletContext context) {
        appletContext = context;
    }

    public void setActive()
    {
        active = true;
    }

    public void setInactive()
    {
        active = false;
    }

    public void paramsDone() {
        // simply builds up the search path
        // first search in already loaded archives
        if( !archives_loaded ) {
            for( int i = 0; i < archives.size(); ++i ) {
                String jar = (String)archives.elementAt( i );
                try {
                    URL httpURL = new URL(codeBaseURL, jar);
                    String jarUrlString = "jar:" + httpURL.toString() + "!/";
                    try {
                        addURL(new URL(jarUrlString));
                        Main.debug("added URL " + jarUrlString + " to KJASAppletClassLoader");
                    } catch (MalformedURLException e) {
                        Main.kjas_err("Could not construct jar URL: " + jarUrlString, e);
                    }
                 } catch (MalformedURLException e) {
                    Main.kjas_err("Could not construct http URL for: " + codeBaseURL + " + " + jar, e);
                 }
            }
            archives_loaded = true;
        }
        else archives_loaded = true;
        // finally add code base url
        addURL(codeBaseURL);
   }

    void addArchiveName( String jarname )
    {
        if( !archives.contains( jarname ) )
        {
            archives.add( jarname );
            archives_loaded = false;
        }
    }
    

    public URL getDocBase()
    {
        return docBaseURL;
    }

    public URL getCodeBase()
    {
        return codeBaseURL;
    }

    /***************************************************************************
     **** Class Loading Methods
     **************************************************************************/
    public Class findClass( String name ) throws ClassNotFoundException
    {
        Class rval;
        
        try
        {
            //check for a system class
            rval = findSystemClass( name );
            if( rval != null )
                return rval;
        } catch (ClassNotFoundException e )
        {
            if (appletContext != null) {
                appletContext.showStatus("Loading: " + name);
            }
            //check the loaded classes 
            rval = findLoadedClass( name );
            if( rval != null )
                return rval;
            return super.findClass(name);
        }
        throw new ClassNotFoundException("Class:" + name);
    }
    
    public Class loadClass( String name ) throws ClassNotFoundException
    {
        Main.debug( dbgID + "loadClass, class name = " + name );
        //We need to be able to handle foo.class, so strip off the suffix
        String fixed_name = name;
        Class rval = null;
        if( name.endsWith( ".class" ) )
        {
            fixed_name = name.substring( 0, name.lastIndexOf( ".class" ) );
        }
        Class cl = super.loadClass(fixed_name);
        Main.debug(this.getClass().getName() + " returns class " + cl.getName());
        return cl;
    }

    public InputStream getResourceAsStream( String name )
    {
        Main.debug( dbgID + "getResourceAsStream, name = " + name );
        InputStream stream = super.getResourceAsStream(name);
        Main.debug("got stream " + stream);
        return stream;
    }
    
    public URL getResource( String name )
    {
        Main.debug( dbgID + "getResource, name = " + name );
        return super.getResource( name );
    }
    
    public URL findResource( String name)
    {
        Main.debug( dbgID + "findResource, name = " + name );
        if (appletContext != null) {
            appletContext.showStatus("Loading: " + name);
        }
        URL url =  super.findResource( name );
        Main.debug("findResource for " + name + " returns " + url);
        return url;
    }
   
    protected PermissionCollection getPermissions(CodeSource cs) {
        Main.debug(dbgID + " getPermissions(" + cs + ")");
        return super.getPermissions(cs);
    }
    
}
