using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Management;
using System.Security.Principal;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Threading;


namespace IdentaZone.IdentaMaster
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        ObservableCollection<UserRow> userRows = new ObservableCollection<UserRow>();

        String currUserSID = System.Security.Principal.WindowsIdentity.GetCurrent().User.ToString();


        private WinUser Serialize(UserRow userRow)
        {
            WinUser user = new WinUser();
            user.Name = userRow.Username;
            user.SID = userRow.SID;
            user.FullName = userRow.Fullname;
            user.Identification = "Password OR Biometrics";
            return user;
        }

        /// <summary>
        /// Update of UserGrid, read user list from windows, compare them to db, sort
        /// </summary>
        private void UpdateUsers()
        {
            Dictionary<String, String> userStatus = new Dictionary<String, String>();
            List<UserRow> sortingQueue = new List<UserRow>();

            ManagementObjectSearcher usersSearcher = new ManagementObjectSearcher(@"SELECT * FROM Win32_UserAccount");// WHERE Disabled='false'");
            ManagementObjectCollection users = usersSearcher.Get();
            var localUsers = users.Cast<ManagementObject>().Where(
                u => (bool)u["LocalAccount"] == true &&
                     int.Parse(u["SIDType"].ToString()) == 1 &&
                     u["Name"].ToString() != "HomeGroupUser$"
                     );

            bool hasActiveAdmin = false;
            WinUser dbUser;
            String currUserName = WindowsIdentity.GetCurrent().Name;

            foreach (ManagementObject user in users)
            {
                UserRow newRow;
                bool isUserActive = false;
                bool isDisabled = (user["Disabled"].ToString() == "True");

                dbUser = _db.GetUser(user["SID"].ToString());
                if (dbUser != null)
                {
                    // Check whether name changed
                    if (dbUser.Name != user["Name"].ToString())
                    {
                        dbUser.Name = user["Name"].ToString();
                    }
                    // Check whether fullname changed
                    if (dbUser.FullName != user["FullName"].ToString())
                    {
                        dbUser.FullName = user["FullName"].ToString();
                    }
                    _db.UpdateUser(dbUser, false);

                    if (!dbUser.isDeactivated)
                    {
                        hasActiveAdmin = true;
                    }


                    isUserActive = true;
                    newRow = new UserRow(user["Name"].ToString(), user["FullName"].ToString(), user["SID"].ToString(), isUserActive);
                    if (dbUser.isDeactivated)
                    {
                        newRow.status = UserEdit.Base.STATUS.DEACTIVATED;
                    }
                    if (isDisabled)
                    {
                        userStatus.Add(user["SID"].ToString(), "Disabled");
                        newRow.status = UserEdit.Base.STATUS.DISABLED;
                    }
                    else
                    {
                        userStatus.Add(user["SID"].ToString(), "Active");
                    }
                    sortingQueue.Add(newRow);
                }
                else if (!isDisabled)
                {
                    userStatus.Add(user["SID"].ToString(), "Inactive");
                    newRow = new UserRow(user["Name"].ToString(), user["FullName"].ToString(), user["SID"].ToString(), isUserActive);
                    sortingQueue.Add(newRow);
                }

            }

            if (!hasActiveAdmin)
            {
                var user = _db.GetUser(currUserSID);
                if (user != null)
                {
                    _db.RestoreUser(currUserSID);
                    foreach (UserRow record in sortingQueue.Where(item => item.SID == currUserSID))
                    {
                        record.isActive = true;
                        record.status = UserEdit.Base.STATUS.OK;
                        userStatus[currUserSID] = "Active";
                    }
                }
                else
                {
                    foreach (UserRow record in sortingQueue)
                    {
                        if (record.SID == currUserSID)
                        {
                            WinUser newUser = Serialize(record);

                            if (_db.AddUser(newUser) == false)
                            {
                                MessageBox.Show("Can't add user");
                            }
                            else
                            {
                                record.isActive = true;
                            }
                            break;
                        }
                    }
                }
            }

            //Check whether there are deleted users in DB
            List<WinUser> dbList = _db.GetAllUsers();
            foreach (WinUser dbRecord in dbList)
            {
                if (!userStatus.ContainsKey(dbRecord.SID))
                {
                    UserRow tmpRow = new UserRow(dbRecord);
                    tmpRow.status = UserEdit.Base.STATUS.DELETED;
                    sortingQueue.Add(tmpRow);
                }
            }

            // sorting
            sortingQueue.Sort();
            sortingQueue.Reverse();
            userRows.Clear();
            Action EmptyDelegate = delegate() { };
            userGrid.Dispatcher.Invoke(DispatcherPriority.Render, EmptyDelegate);
            foreach (UserRow record in sortingQueue)
            {
                userRows.Add(record);
            }
        }

        /// <summary>
        /// Users the grid click.
        /// </summary>
        /// <param name="Sender">The Sender.</param>
        /// <param name="e">The <see cref="MouseButtonEventArgs"/> instance containing the event data.</param>
        private void userGridClick(object sender, MouseButtonEventArgs e)
        {
            userGridClickReaction();
        }
        private void userGridClickReaction()
        {
            String fullname;
            List<Credential> tmpCredentials = new List<Credential>();
            if (userGrid.SelectedIndex < 0)
            {
                return;
            }

            if (String.IsNullOrEmpty(userRows[userGrid.SelectedIndex].Fullname))
            {
                fullname = userRows[userGrid.SelectedIndex].UsernameView;
            }
            else
            {
                fullname = userRows[userGrid.SelectedIndex].FullnameView;
            }

            tmpCredentials = _db.GetCredentials(userRows[userGrid.SelectedIndex].SID);
            if (tmpCredentials == null)
            {
                tmpCredentials = new List<Credential>();
            }

            bool isLocalAdmin = false;
            if (userRows[userGrid.SelectedIndex].SID == currUserSID)
            {
                isLocalAdmin = true;
            }

            UserEdit.Base userEdit = new UserEdit.Base(fullname, tmpCredentials, pluginManager, appSet,
                _db.GetUser(userRows[userGrid.SelectedIndex].SID), 
                userRows[userGrid.SelectedIndex].status,
                userRows[userGrid.SelectedIndex].Username,
                userRows[userGrid.SelectedIndex].SID, 
                _db.GetAllUsers(), 
                Licenser,
                isLocalAdmin);
            userEdit.Owner = this;
            try
            {
                userEdit.ShowDialog();
            }
            catch (Exception ex)
            {
                Log.Error(ex.Message);
            }
            WinUser user = _db.GetUser(userRows[userGrid.SelectedIndex].SID);
            switch (userEdit.Result)
            {
                case UserEdit.Base.RESULT.UPDATE_USER:

                    String res = "ok";//userEdit.GetResultXml();

                    if (user == null)
                    {
                        user = Serialize(userRows[userGrid.SelectedIndex]);
                        _db.AddUser(user);
                    }
                    if (userEdit.LoginType == XmlDB.LOGIN_TYPE.BIO)
                    {
                        int credCount = 0;
                        foreach (var credo in tmpCredentials)
                        {
                            if (!(credo is PWDCredential))
                            {
                                if ((credo as FingerCredential).fingers.Count() > 0)
                                {
                                    credCount++;
                                }
                            }
                        }
                        if (credCount == 0)
                        {
                            userEdit.LoginType = XmlDB.LOGIN_TYPE.MIXED;
                            MessageBox.Show("You haven't registered any biometric.\n Login type is set to 'Password or Biometrics'", "", MessageBoxButton.OK, MessageBoxImage.Warning);
                        }
                    }
                    user.Password = userEdit.Password;
                    user.Identification = XmlDB.LOGIN_STRING[(int)userEdit.LoginType];
                    _db.UpdateUser(user, true);
                    if (res != null)
                    {
                        _db.SetCredentials(user.SID, tmpCredentials);
                    }
                    UpdateUsers();
                    userGrid.Items.Refresh();

                    break;
                case UserEdit.Base.RESULT.DELETE:
                    if (user != null)
                    {
                        _db.RemoveUser(userRows[userGrid.SelectedIndex].SID);
                        UpdateUsers();
                        userGrid.Items.Refresh();
                    }
                    break;
                case UserEdit.Base.RESULT.DEACTIVATE:
                    if (user != null)
                    {
                        _db.DeactivateUser(userRows[userGrid.SelectedIndex].SID);
                        UpdateUsers();
                        userGrid.Items.Refresh();
                    }
                    break;
                case UserEdit.Base.RESULT.RESTORE:
                    if (user != null)
                    {
                        _db.RestoreUser(userRows[userGrid.SelectedIndex].SID);
                        UpdateUsers();
                        userGrid.Items.Refresh();
                    }
                    break;

            }
            if (userEdit.Result != UserEdit.Base.RESULT.CANCELED)
            {
                if (Licenser.State == IdentaZone.BioControls.Auxiliary.Licenser.STATE.ACTIVATED)
                {
                    _db.Deploy(pluginManager.GetDeploymentList());
                }
            }
            //userGrid.SelectedItem = null;
        }

        /// <summary>
        /// This method is setting up colors for grid rows
        /// </summary>
        /// <param name="Sender"></param>
        /// <param name="e"></param>
        void userGrid_LoadingRow(object sender, DataGridRowEventArgs e)
        {
            if (e.Row.Item != null)
            {
                UserRow rowValue = e.Row.Item as UserRow;
                if (rowValue.status != UserEdit.Base.STATUS.OK && rowValue.status != UserEdit.Base.STATUS.DEACTIVATED)
                {
                    e.Row.Foreground = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#AAAAAA"));
                    rowValue.Color = "#AAAAAA";
                }
                else if (rowValue.isActive && rowValue.status != UserEdit.Base.STATUS.DEACTIVATED)
                {
                    e.Row.Foreground = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#b0cb1f"));
                    rowValue.Color = "#b0cb1f";
                }
                else
                {
                    e.Row.Foreground = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#EF7F1A"));
                    rowValue.Color = "#EF7F1A";
                }
            }
        }



        private void UserRefreshButton_Click(object sender, RoutedEventArgs e)
        {
            UpdateUsers();
            userGrid.Items.Refresh();
        }

        public class UserRow : IComparable, IEquatable<UserRow>
        {

            private const String NotAssigned = "<not assigned>";
            public const int NameMaxLength = 25;
            /// <summary>
            /// The username value
            /// </summary>
            private String UsernameValue;

            /// <summary>
            /// Gets or sets the username view.
            /// Displayable value, used to show in Grid.
            /// Too long value are truncated. Used Username property to set this one
            /// </summary>
            public String UsernameView { get; set; }

            /// <summary>
            /// Gets or sets the username and also sets UsernameView value.
            /// UsernameView value may be truncated
            /// </summary>
            public String Username
            {
                get
                {
                    return UsernameValue;
                }
                set
                {
                    UsernameValue = value;
                    if (value.Length > NameMaxLength)
                    {
                        UsernameView = value.Substring(0, NameMaxLength) + "...";
                    }
                    else
                    {
                        UsernameView = value;
                    }
                }
            }

            /// <summary>
            /// Value of Fullname
            /// </summary>
            private string FullnameValue;
            /// <summary>
            /// Displayable value, used to show in Grid
            /// Too long values are truncated, empty - changed to "Not Asssigned"
            /// </summary>
            public String FullnameView { get; set; }
            /// <summary>
            /// Get or Set Fullname value, also sets FullnameView
            /// Too long values are truncated, empty - changed to "Not Asssigned"
            /// </summary>
            public string Fullname
            {
                get
                {
                    return FullnameValue;
                }
                set
                {
                    FullnameValue = value;
                    if (String.IsNullOrEmpty(value))
                    {
                        FullnameView = NotAssigned;
                    }
                    else if (value.Length > NameMaxLength)
                    {
                        FullnameView = value.Substring(0, NameMaxLength) + "...";
                    }
                    else
                    {
                        FullnameView = value;
                    }
                }
            }
            /// <summary>
            /// Color of row depending on user state
            /// </summary>
            public string Color { get; set; }

            public string SID { get; set; }
            public bool isActive { get; set; }
            public UserEdit.Base.STATUS status { get; set; }

            public UserRow(string username, string fullname, string sid, bool isActive)
            {
                Username = username;
                Fullname = fullname;
                SID = sid;
                this.isActive = isActive;
                status = UserEdit.Base.STATUS.OK;
            }

            public UserRow(WinUser user)
            {
                Username = user.Name;
                Fullname = user.FullName;
                SID = user.SID;
                isActive = true;
                status = UserEdit.Base.STATUS.OK;
            }

            public int CompareTo(object obj)
            {
                UserRow other = (obj as UserRow);
                if (this.status != UserEdit.Base.STATUS.OK)
                {
                    return -1;
                }
                if (other.status != UserEdit.Base.STATUS.OK)
                {
                    return 1;
                }
                if (other.isActive && !isActive)
                {
                    return -1;
                }
                if (isActive && !other.isActive)
                {
                    return 1;
                }
                return -(this.Username.CompareTo(other.Username));
            }

            public bool Equals(UserRow other)
            {
                return this.Fullname.Equals(other.Fullname) &&
                    this.isActive == other.isActive &&
                    (this.status == other.status || (this.status != UserEdit.Base.STATUS.OK && other.status != UserEdit.Base.STATUS.OK)) &&
                    this.SID.Equals(other.SID) &&
                    this.Username.Equals(other.Username);
            }
        }
    }
}