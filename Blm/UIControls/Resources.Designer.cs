﻿//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.18444
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

namespace UIControlsINDSS {
    using System;
    
    
    /// <summary>
    ///   A strongly-typed resource class, for looking up localized strings, etc.
    /// </summary>
    // This class was auto-generated by the StronglyTypedResourceBuilder
    // class via a tool like ResGen or Visual Studio.
    // To add or remove a member, edit your .ResX file then rerun ResGen
    // with the /str option, or rebuild your VS project.
    [global::System.CodeDom.Compiler.GeneratedCodeAttribute("System.Resources.Tools.StronglyTypedResourceBuilder", "4.0.0.0")]
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
    [global::System.Runtime.CompilerServices.CompilerGeneratedAttribute()]
    internal class Resources {
        
        private static global::System.Resources.ResourceManager resourceMan;
        
        private static global::System.Globalization.CultureInfo resourceCulture;
        
        [global::System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
        internal Resources() {
        }
        
        /// <summary>
        ///   Returns the cached ResourceManager instance used by this class.
        /// </summary>
        [global::System.ComponentModel.EditorBrowsableAttribute(global::System.ComponentModel.EditorBrowsableState.Advanced)]
        internal static global::System.Resources.ResourceManager ResourceManager {
            get {
                if (object.ReferenceEquals(resourceMan, null)) {
                    global::System.Resources.ResourceManager temp = new global::System.Resources.ResourceManager("UIControlsINDSS.Resources", typeof(Resources).Assembly);
                    resourceMan = temp;
                }
                return resourceMan;
            }
        }
        
        /// <summary>
        ///   Overrides the current thread's CurrentUICulture property for all
        ///   resource lookups using this strongly typed resource class.
        /// </summary>
        [global::System.ComponentModel.EditorBrowsableAttribute(global::System.ComponentModel.EditorBrowsableState.Advanced)]
        internal static global::System.Globalization.CultureInfo Culture {
            get {
                return resourceCulture;
            }
            set {
                resourceCulture = value;
            }
        }
        
        /// <summary>
        ///   Looks up a localized string similar to &lt;?xml version=&quot;1.0&quot; encoding=&quot;UTF-8&quot; ?&gt;
        ///&lt;LicenseKeyTemplate version=&quot;1&quot;&gt;
        ///  &lt;LicenseKey encoding=&quot;BASE64X&quot; characterGroups=&quot;6&quot; charactersPerGroup=&quot;6&quot; groupSeparator=&quot;-&quot; header=&quot;&quot; footer=&quot;&quot;&gt;
        ///    &lt;Data&gt;
        ///      &lt;DataFields size=&quot;55&quot;&gt;
        ///        &lt;Field name=&quot;FeatureSet&quot; type=&quot;Raw&quot; size=&quot;55&quot; offset=&quot;0&quot; /&gt;
        ///      &lt;/DataFields&gt;
        ///      &lt;ValidationFields&gt;
        ///        &lt;Field name=&quot;Email&quot; type=&quot;String&quot; size=&quot;800&quot; offset=&quot;0&quot; /&gt;
        ///      &lt;/ValidationFields&gt;
        ///    &lt;/Data&gt;
        ///    &lt;Signature size=&quot;161&quot;&gt;
        ///      &lt;SignaturePublicKey&gt; [rest of string was truncated]&quot;;.
        /// </summary>
        internal static string LicenserTemplate {
            get {
                return ResourceManager.GetString("LicenserTemplate", resourceCulture);
            }
        }
    }
}
