using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using log4net;
using Microsoft.Win32;
using SoftActivate.Licensing;

namespace UIControlsINDSS
{
    public class Licenser
    {
        // Logger instance
        protected readonly ILog log = LogManager.GetLogger(typeof(Licenser));

        const int PRODUCT_ID = 178500;
        private String LicenseKey_ = "";
        private String ActivationKey_ = "";
        private String Email_;
        private String CurrentHWID_;
        private String HWID_ = "";
        private KeyTemplate Tmpl_;

        public String LicenseKey { get { return LicenseKey_; } }
        public String ActivationKey { get { return ActivationKey_; } }
        public String Email { get { return Email_; } }

        public enum STATE { LOADING, NOT_REGISTERED, ACTIVATED };

        public STATE State = STATE.LOADING;

        public Licenser()
        {
            LoadKeys();
            Tmpl_ = new KeyTemplate(Resources.LicenserTemplate);
        }

        public void Init(Action isActivatedCallback)
        {
            Task.Factory.StartNew(() => InitTask(isActivatedCallback));
        }

        public void InitTask(Action isActivatedCallback)
        {
            CurrentHWID_ = KeyHelper.GetCurrentHardwareId();
            if (CheckIfActivated())
            {
                State = STATE.ACTIVATED;
            }
            else
            {
                State = STATE.NOT_REGISTERED;
            }
            isActivatedCallback();
        }


        private void SaveKeys()
        {
            using (RegistryKey skey = Registry.LocalMachine.OpenSubKey("Software", true))
            {
                using (RegistryKey key = skey.CreateSubKey("IdentaMaster", RegistryKeyPermissionCheck.ReadWriteSubTree))
                {
                    key.SetValue("LicenseKey", LicenseKey_);
                    key.SetValue("ActivationKey", ActivationKey_);
                    key.SetValue("Email", Email_);
                    key.SetValue("HWID", HWID_);
                }

            }
        }

        private void LoadKeys()
        {
            try
            {
                using (RegistryKey skey = Registry.LocalMachine.OpenSubKey("Software", false))
                {
                    using (RegistryKey key = skey.OpenSubKey("IdentaMaster", RegistryKeyPermissionCheck.ReadSubTree))
                    {
                        LicenseKey_ = (string)key.GetValue("LicenseKey");
                        ActivationKey_ = (string)key.GetValue("ActivationKey");
                        Email_ = (string)key.GetValue("Email");
                        HWID_ = (string)key.GetValue("HWID");
                    }
                }
            }
            catch (Exception ex)
            {
                log.Warn("Error, while loading license keys? Check admin rights?", ex);
            }
        }

        private bool CheckIfActivated()
        {
            try
            {
                if (!KeyHelper.MatchCurrentHardwareId(HWID_))
                {
                    log.Error("HWID changed");
                    return false;
                }
                KeyValidator keyVal = new KeyValidator(Tmpl_);

                keyVal.SetKey(LicenseKey_);

                keyVal.SetValidationData("Email", Email_); // the key will not be valid if you set a different user name than the one you have set at key generation
                LicensingClient licensingClient = new LicensingClient(Tmpl_, LicenseKey_, keyVal.QueryValidationData(null), HWID_, ActivationKey_);
                if (licensingClient.IsLicenseValid())
                {
                    byte[] featureSet = keyVal.QueryKeyData("FeatureSet");
                    featureSet.ToString();
                    return true;
                }
                else
                {
                    return false;
                }
            }
            catch (Exception E)
            {
                log.Error("Program is not activated", E);
                return false;
            }
        }

        Action<bool, string> OnActivation = null;

        public void ActivationTask(String email, String key)
        {

            string activationStatusStr = "";
            KeyValidator keyVal = null;
            try
            {
                // validate the generated key. This sequence of code is also used in the actual product to validate the entered license key
                keyVal = new KeyValidator(Tmpl_);
                keyVal.SetKey(key);
            }
            catch (Exception ex)
            {
                OnActivation(false, "Key is incorrect");
                log.Error("Key validation failed " + ex.Message);
                return;
            }

            try
            {

                keyVal.SetValidationData("Email", email); // the key will not be valid if you set a different user name than the one you have set at key generation


                LicensingClient licensingClient = new LicensingClient("https://activation.identamaster.com:444",
                                                     Tmpl_,
                                                     key,
                                                    keyVal.QueryValidationData(null), CurrentHWID_,
                                                     PRODUCT_ID);

                licensingClient.AcquireLicense();

                LicenseKey_ = key;
                Email_ = email;
                ActivationKey_ = licensingClient.ActivationKey;
                HWID_ = CurrentHWID_;
                // save keys
                SaveKeys();

                if (!licensingClient.IsLicenseValid())
                {

                    switch (licensingClient.LicenseStatus)
                    {
                        case LICENSE_STATUS.InvalidActivationKey:
                            activationStatusStr = "invalid activation key";
                            break;

                        case LICENSE_STATUS.InvalidHardwareId:
                            activationStatusStr = "invalid hardware id";
                            break;

                        case LICENSE_STATUS.Expired:
                            {
                                // the license expiration date returned by LicenseExpirationDate property is only valid if IsLicenseValid() returns true, 
                                // or if IsLicenseValid() returns false and LicenseStatus returns LicenseStatus.Expired
                                DateTime expDate = licensingClient.LicenseExpirationDate;
                                activationStatusStr = "license expired (expiration date: " + expDate.Month + "/" + expDate.Day + "/" + expDate.Year + ")";
                            }
                            break;

                        default:
                            activationStatusStr = "unknown";
                            break;
                    }
                    OnActivation(false, activationStatusStr);
                }
                else
                {
                    State = STATE.ACTIVATED;
                    OnActivation(true, "");
                    return;
                }

            }
            catch (System.Net.WebException ex)
            {
                switch (ex.Status)
                {
                    case System.Net.WebExceptionStatus.ConnectFailure:
                        OnActivation(false, "Couldn't access activation server");
                        log.Error("Could not connect to server " + ex.Message);
                        break;
                    case System.Net.WebExceptionStatus.ProtocolError:
                        OnActivation(false, "Server didn't validate this key");
                        log.Error("Key validation failed " + ex.Message);
                        break;
                    default:
                        OnActivation(false, "Network error");
                        log.Error("Unknown network error " + ex.Message);
                        break;
                }
                return;
            }
            catch (Exception ex)
            {
                OnActivation(false, "Failed");
                log.Error("Key validation failed " + ex.Message);
                return;
            }
        }

        public void Activate(String key, String email, Action<bool, string> onActivation)
        {
            OnActivation = onActivation;

            Task.Factory.StartNew(() =>
            {
                ActivationTask(email, key);
            });
        }
    }
}
