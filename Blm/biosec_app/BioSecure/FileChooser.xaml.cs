using log4net;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace IdentaZone.BioSecure
{
    /// <summary>
    /// Interaction logic for FileChooser.xaml
    /// </summary>
    public partial class FileChooser : Window, INotifyPropertyChanged
    {

        private TreeViewModel _treeModel;
        private CollectorDialog _owner;
        private bool _isDecryptSelectedEnabled;
        private bool _isDecryptAllEnabled;
        private bool _startDecryption;
        private bool _externalDestroy;
        private String Filename { get; set; }
        private String _identifiedAsString;

        public Action DecryptSelected_Notify { get; set; }
        public Action DecryptAll_Notify { get; set; }



        public String IdentifiedAsString
        {
            get
            {
                return _identifiedAsString;
            }

            set
            {
                _identifiedAsString = value;
                OnPropertyChanged("IdentifiedAsString");
            }
      
        }


        public bool IsDecryptSelectedEnabled
        {
            get
            {
                return _isDecryptSelectedEnabled;
            }
            set
            {
                _isDecryptSelectedEnabled = value;
                OnPropertyChanged("IsDecryptSelectedEnabled");
            }
        }

        public bool IsDecryptAllEnabled
        {
            get
            {
                return _isDecryptAllEnabled;
            }
            set
            {
                _isDecryptAllEnabled = value;
                OnPropertyChanged("IsDecryptAllEnabled");
            }
        }


        public void Destroy()
        {
            _externalDestroy = true;
            this.Dispatcher.BeginInvoke(new Action(() => this.Close()));          
        }


        public TreeViewModel TreeModel
        {
            get
            {
                return _treeModel;
            }
        }

        public FileChooser(CollectorDialog owner)
        {
            Owner = owner;
            _owner = owner;
            _treeModel = new TreeViewModel(this, Filename);
            _isDecryptSelectedEnabled = false;
            _isDecryptAllEnabled = false;
            _startDecryption = false;
            _externalDestroy = false;
            InitializeComponent();
        }

        public FileChooser(CollectorDialog collectorDialog, string Filename)
        {
            this.Filename = Filename;
            Owner = collectorDialog;
            _owner = collectorDialog;
            IsDecryptSelectedEnabled = false;
            IsDecryptAllEnabled = false;
            _externalDestroy = false;
            _treeModel = new TreeViewModel(this, Filename);
            InitializeComponent();
        }
        

        private void Window_Closing(object sender, CancelEventArgs e)
        {
            if (!_startDecryption && !_externalDestroy)
            {
                _owner.Close();
            }
        }


        public void ShowFromList(List<BioFileInfo> itemsList)
        {
            //var timePoint0 = DateTime.Now;
            foreach(var item in itemsList){
                TreeModel.Add(item);
            }
            TreeModel.Update();
            IsDecryptAllEnabled = true;
            OnPropertyChanged("TreeModel");
            //var timePoint1 = DateTime.Now;
            //var diff01 = timePoint0.Subtract(timePoint1);
            //log.Info("total timespan: " + diff01.ToString() + " ");
            this.Show();
        }


        internal void AddFile(BioFileInfo bioFileInfo)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                //log.InfoFormat("Adding file {0} with number {1}", bioFileInfo.Filename, bioFileInfo.FileNumber);
                TreeModel.Add(bioFileInfo);
                TreeModel.Update();
                //OnPropertyChanged("TreeModel");
            }));
        }

        private void DecryptSelected_Click(object sender, RoutedEventArgs e)
        {
            _startDecryption = true;
            DecryptSelected_Notify();
        }

        public List<ulong> GetSelectedNodes()
        {
            var selectedNodes = new List<ulong>();
            FindSelectedNodes(_treeModel.Items[0], selectedNodes);
            return selectedNodes;
        }

        public List<ulong> GetAllNodes()
        {
            var itemsCount = _treeModel.getItemsCount();
            List<ulong> allNodes = new List<ulong>();
            for (ulong itemIndex = 0; itemIndex < itemsCount; ++itemIndex)
            {
                allNodes.Add(itemIndex);
            }

            return allNodes;
        }

        private void FindSelectedNodes(FileViewModel fileViewModel, List<ulong> selectedNodes)
        {
            foreach (var node in fileViewModel.Files)
            {
                if (node.BioFileInfo != null)
                {
                    if (node.IsChecked)
                    {
                        selectedNodes.Add(node.BioFileInfo.FileNumber);
                    }

                    if (node.BioFileInfo.Type == BioFileInfo.PathType.Folder)
                    {
                        FindSelectedNodes(node, selectedNodes);
                    }
                }
            }
        }

        private void DecryptAll_Click(object sender, RoutedEventArgs e)
        {
            _startDecryption = true;
            // _treeModel.Items[0].IsChecked = true;
            DecryptAll_Notify();
        }



        // boiler-plate
        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged(String propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class BioFileInfo
    {
        public enum PathType
        {
            File,
            Folder
        }

        public String Filename;
        public ulong FileNumber;
        public ulong FileSize;
        public long Timestamp;
        public PathType Type;

        public BioFileInfo()
        {
        }

    }


    public class TreeViewModel: INotifyPropertyChanged
    {
        private ObservableCollection<FileViewModel> _items = new ObservableCollection<FileViewModel>();
        private FileChooser _fileChooser;
        public int _checkedCount;
        private ulong _itemCount;
        private  HashSet<string> _hashSet; 


        public ulong getItemsCount()
        {
            return _itemCount;
        }


        public ObservableCollection<FileViewModel> Items
        {
            get
            {
                return _items;
            }
        }
        public TreeViewModel(FileChooser fileChooser, String rootname)
        {
            _fileChooser = fileChooser;
            _items.Add(new FileViewModel(this, rootname));
            _itemCount = 0;
            _hashSet=new HashSet<string>();
        }

        public void IncrementChecked()
        {
            if (_checkedCount == 0)
            {
                _fileChooser.IsDecryptSelectedEnabled = true;
            }
            _checkedCount++;
        }

        public void DecrementChecked()
        {
            _checkedCount--;
            if (_checkedCount == 0)
            {
                _fileChooser.IsDecryptSelectedEnabled = false;
            }
        }

        internal void Add(BioFileInfo bioFileInfo)
        {
            //var timePoint0 = DateTime.Now;
            ++_itemCount;
            String[] split = System.IO.Path.GetDirectoryName(bioFileInfo.Filename).Split(System.IO.Path.DirectorySeparatorChar);
            //var timePoint1 = DateTime.Now;
            //var diff01 = timePoint1.Millisecond - timePoint0.Millisecond;
            //log.Info("timespan 1: " + diff01.ToString() + " ms");
            if (!String.IsNullOrEmpty(split[0]))
            {
                CheckAndCreateRoute(_items[0], split.ToList<String>(), bioFileInfo);
            }
            else
            {
                _items[0].Files.Add(new FileViewModel(this, bioFileInfo.Filename) { BioFileInfo = bioFileInfo });
                _hashSet.Add(bioFileInfo.Filename);
            }
            //var timePoint2 = DateTime.Now;
            //var diff12 = timePoint2.Millisecond - timePoint1.Millisecond;
            // log.Info("timespan 2: " + diff12.ToString() + " ms");
        }


        internal void Update()
        {
            _items[0].Update();
        }


        private void CheckAndCreateRoute(FileViewModel fileViewModel, List<String> split, BioFileInfo bioFileInfo)
        {
            if (split.Count > 0)
            {
                
                

                

                var dirName = split[0];
                /*
                if (!fileViewModel.Files.Any(item => item.Name == dirName))
                {
                    fileViewModel.Files.Add(new FileViewModel(this, dirName));
                }

                HashSet<string> hash=new HashSet<string>();*/
                


                if (!_hashSet.Contains(dirName))
                {
                    fileViewModel.Files.Add(new FileViewModel(this, dirName));
                    _hashSet.Add(dirName);

                }
                
                split.RemoveAt(0);
                CheckAndCreateRoute(fileViewModel.Files.First(item => item.Name == dirName), split, bioFileInfo);
            }
            else
            {
                var fileName = System.IO.Path.GetFileName(bioFileInfo.Filename);
                if (bioFileInfo.Type == BioFileInfo.PathType.File)
                {
                   // if (!fileViewModel.Files.Any(item => item.Name == fileName))
                    if (!_hashSet.Contains(bioFileInfo.Filename))
                    {
                        fileViewModel.Files.Add(new FileViewModel(this, fileName) { BioFileInfo = bioFileInfo });
                        _hashSet.Add(bioFileInfo.Filename);
                    }
                }
                else
                {
                    var existingNode = fileViewModel.Files.FirstOrDefault((obj) => obj.Name == fileName);
                    if (existingNode != null)
                    {
                        existingNode.BioFileInfo = bioFileInfo;
                    }
                    else
                    {                        
                        fileViewModel.Files.Add(new FileViewModel(this, fileName) { BioFileInfo = bioFileInfo });
                        _hashSet.Add(fileName);
                    }
                }
            }
        }
          // boiler-plate
        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged(String propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class FileViewModel : INotifyPropertyChanged
    {
        private ObservableCollection<FileViewModel> _files = new ObservableCollection<FileViewModel>();

        public BioFileInfo BioFileInfo = null;
        public ObservableCollection<FileViewModel> Files { get { return _files; } }
        public String Name { get; set; }
        TreeViewModel _treeViewModel;

        private bool _isChecked;
        public bool IsChecked
        {
            get
            {
                return _isChecked;
            }
            set
            {
                
                if(_isChecked != value){
                    if(value){
                        _treeViewModel.IncrementChecked();
                    } else {
                        _treeViewModel.DecrementChecked();
                    }
                }
                
                _isChecked = value;
               foreach (var file in _files)
                {
                    file.IsChecked = value;
                }
                OnPropertyChanged("IsChecked");
            }
        }

        Random _random = new Random();
        public String ImageUrl
        {
            get
            {
                if (BioFileInfo.Type == BioSecure.BioFileInfo.PathType.Folder)
                {
                    return "Images/folder.png";
                }
                else
                {
                    return "Images/file.png";
                }
            }
        }

        public String Size
        {
            get
            {
                if (BioFileInfo.Type == BioSecure.BioFileInfo.PathType.File)
                {
                    string[] sizes = { "B", "KB", "MB", "GB" };
                    double len = BioFileInfo.FileSize;
                    int order = 0;
                    while (len >= 1024 && order + 1 < sizes.Length)
                    {
                        order++;
                        len = len / 1024;
                    }

                    // Adjust the format string to your preferences. For example "{0:0.#}{1}" would
                    // show a single decimal place, and no space.
                    string result = String.Format("{0:0.##} {1}", len, sizes[order]);

                    return " " + result + ")";
                } else if (BioFileInfo.Type == BioSecure.BioFileInfo.PathType.Folder) {
                    return ")";
                }
                return ")";
            }
        }

        public String ChangedDate
        {
            get
            {
                var date = (new DateTime(1970, 1, 1, 0, 0, 0)).AddSeconds(BioFileInfo.Timestamp);
                return "(" + date.ToLocalTime().ToString("G");
            }
        }


        public FileViewModel(TreeViewModel treeViewModel, String filename)
        {
            this.Name = filename;
            this._treeViewModel = treeViewModel;
        }


        // boiler-plate
        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged(String propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }

        internal void Update()
        {
            OnPropertyChanged("Files");
        }
    }
        
}
