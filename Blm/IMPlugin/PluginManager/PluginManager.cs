using IdentaZone.IMPlugin;
using log4net;
using System;
using System.Collections.Generic;
using Microsoft.Win32;

namespace IdentaZone.IMPlugin.PluginManager
{
    public class PluginManager
    {
        public readonly ILog log = LogManager.GetLogger(typeof(PluginManager));

        // plugin list
        public List<IDevicePlugin> Plugins { get; set; }
        public List<IDeviceControl> DeviceControls { get; set; }


        public PluginManager()
        {
            Plugins = new List<IDevicePlugin>();
            DeviceControls = new List<IDeviceControl>();
            //log.Info("Plugin manager loaded");
        }

        public void UnloadPlugins()
        {
            foreach (var control in DeviceControls)
            {
                control.Dispose();
            }
            foreach (var plugin in Plugins)
            {
                plugin.Dispose();
            }
        }


        private IDevicePlugin LoadIDevicePlugins(System.Reflection.Assembly assembly)
        {
            IDevicePlugin p = null;
            foreach (Type t in assembly.GetTypes())
            {
                foreach (Type i in t.GetInterfaces())
                {
                    if (i.Equals(Type.GetType(typeof(IDevicePlugin).AssemblyQualifiedName)))
                    {
                        p = (IDevicePlugin)Activator.CreateInstance(t);
                        Plugins.Add(p);
                        p.Initialize(this);
                        break;
                    }
                }
            }
            return p;
        }

        private void LoadIDeviceControls(System.Reflection.Assembly assembly)
        {
            foreach (Type t in assembly.GetTypes())
            {
                foreach (Type i in t.GetInterfaces())
                {
                    if (i.Equals(Type.GetType(typeof(IDeviceControl).AssemblyQualifiedName)))
                    {
                        IDeviceControl c = (IDeviceControl)Activator.CreateInstance(t);
                        try
                        {
                            c.Initialize();
                            DeviceControls.Add(c);
                        }
                        catch (Exception ex)
                        {
                            log.Error(ex);
                        }
                        break;
                    }
                }
            }
        }

        public void LoadPlugins(String path)
        {
            //log.InfoFormat("Current Directory {0}", path);
            try
            {
                //log.InfoFormat("Found files count {0}", System.IO.Directory.GetFiles(path, "Plugins\\*.dll").Length);
            }
            catch (Exception ex)
            {
                log.Error(ex);
            }
            foreach (string f in System.IO.Directory.GetFiles(path, "Plugins\\*.dll"))
            {
                try
                {
                    Console.WriteLine("Loading plugin {0}", f);
                    System.Reflection.Assembly assembly = System.Reflection.Assembly.LoadFile(f);
                    IDevicePlugin p = LoadIDevicePlugins(assembly);
                   
                    if (p != null)
                    {
                        LoadIDeviceControls(assembly);
                    }
                }  
               catch (System.Reflection.ReflectionTypeLoadException e)
                {
                    log.Error(e);
                    foreach (var ex in e.LoaderExceptions)
                    {
                        log.Error(ex);
                    }
                }
                catch (Exception e)
                {
                    log.Error(e);
                }
            }
        }

        /// <summary>
        /// Find and load all plugins
        /// </summary>
        public void LoadPlugins()
        {
            //var sPath = Registry.GetValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\IdentaZone Inc", "PluginsPath", null);
            object sPath = null;
            try
            {
                //var indssAdminInstalled = Registry.LocalMachine.GetValue("SOFTWARE\\IdentaZone Inc\\INDSSAdminInstalled", null);

                using (RegistryKey skey = Registry.LocalMachine.OpenSubKey("Software", false))
                {
                    using (RegistryKey key = skey.OpenSubKey("IdentaZone", RegistryKeyPermissionCheck.ReadSubTree))
                    {
                        sPath = key.GetValue("PluginsPath");
                    }
                }
            }
            catch (Exception)
            {
                sPath = System.IO.Directory.GetCurrentDirectory();
            }


            //var sPath = Registry.LocalMachine.GetValue("SOFTWARE\\IdentaZone Inc", "PluginsPath");
            //if (sPath==null)
            //{
            //    sPath = System.IO.Directory.GetCurrentDirectory();
            //}
            LoadPlugins(sPath.ToString());
        }

        public FingerTemplate GetTemplate(int type, byte[] bytes)
        {
            FingerTemplate template = null;
            try
            {
                foreach (var control in DeviceControls)
                {
                    template = control.Deserialize(type, bytes);
                    if (template != null)
                    {
                        break;
                    }
                }
            }
            catch (Exception e)
            {
            }
            return template;
        }
        public Dictionary<String, String> GetDeploymentList()
        {
            Dictionary<String, String> list = new Dictionary<string, string>();
            foreach (var plugin in Plugins)
            {
                try
                {
                    list.Add(plugin.Name, plugin.DeploymentList);
                }
                catch(Exception ex)
                {
                    log.Error(ex.Message);
                }
            }
            return list;
        }
    }
}
