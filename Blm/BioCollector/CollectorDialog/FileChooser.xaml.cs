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

namespace IdentaZone.Collector.Dialog
{
    /// <summary>
    /// Interaction logic for FileChooser.xaml
    /// </summary>
    public partial class FileChooser : Window
    {
        // Logger instance
        protected readonly ILog log = LogManager.GetLogger(typeof(FileChooser));

        private TreeViewModel _treeModel;
        private CollectorDialog _owner;
        private String Filename { get; set; }

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
            InitializeComponent();
            _treeModel = new TreeViewModel(Filename, ItemSelectionChanged);
        }

        public FileChooser(CollectorDialog collectorDialog, string Filename)
        {
            this.Filename = Filename;
            Owner = collectorDialog;
            _owner = collectorDialog;
            _treeModel = new TreeViewModel(Filename, ItemSelectionChanged);
            InitializeComponent();
        }

        internal void AddFile(CollectorServices.BioFileInfo bioFileInfo)
        {
            log.InfoFormat("Adding file {0} with number {1}", bioFileInfo.Filename, bioFileInfo.FileNumber);
            TreeModel.Add(bioFileInfo, ItemSelectionChanged);
        }

        private void DecryptSelected_Click(object sender, RoutedEventArgs e)
        {
            _owner.NotifyDecryptList(GetSelectedNodes());
        }

        private List<int> GetSelectedNodes()
        {
            List<int> selectedNodes = new List<int>();
            FindSelectedNodes(_treeModel.Items[0], selectedNodes);
            return selectedNodes;
        }

        private void FindSelectedNodes(FileViewModel fileViewModel, List<int> selectedNodes)
        {
            foreach (var node in fileViewModel.Files)
            {
                if (node.BioFileInfo != null)
                {
                    if (node.IsChecked)
                    {
                        selectedNodes.Add(node.BioFileInfo.FileNumber);
                    }

                    if (node.BioFileInfo.PathType == CollectorServices.BioFileInfo.EntityAtPathType.Folder)
                    {
                        FindSelectedNodes(node, selectedNodes);
                    }
                }
            }
        }

        private void DecryptAll_Click(object sender, RoutedEventArgs e)
        {
            _owner.NotifyDecryptAll();
        }

        private void FileChooserUI_Closed(object sender, EventArgs e)
        {
            _owner.Destroy();
        }

        private void ItemSelectionChanged(bool newValue)
        {
            if (newValue)
            {
                this.DecryptSelected.IsEnabled = true;
            }
            else
            {
                List<int> selectedNodes = new List<int>();
                FindSelectedNodes(_treeModel.Items[0], selectedNodes);
                if (selectedNodes.Count == 0)
                {
                    this.DecryptSelected.IsEnabled = false;
                }
            }
        }
    }

    public class TreeViewModel
    {
        private ObservableCollection<FileViewModel> _items = new ObservableCollection<FileViewModel>();
        public ObservableCollection<FileViewModel> Items
        {
            get
            {
                return _items;
            }
        }

        public TreeViewModel(String rootname, Action<bool> ItemSelectionChanged)
        {
            _items.Add(new FileViewModel(rootname, ItemSelectionChanged));
        }


        internal void Add(CollectorServices.BioFileInfo bioFileInfo, Action<bool> ItemSelectionChanged)
        {
            String[] split = System.IO.Path.GetDirectoryName(bioFileInfo.Filename).Split(System.IO.Path.DirectorySeparatorChar);
            if (!String.IsNullOrEmpty(split[0]))
            {
                CheckAndCreateRoute(_items[0], ItemSelectionChanged, split.ToList<String>(), bioFileInfo);
            }
            else
            {
                _items[0].Files.Add(new FileViewModel(bioFileInfo.Filename, ItemSelectionChanged) { BioFileInfo = bioFileInfo });
            }
        }

        private void CheckAndCreateRoute(FileViewModel fileViewModel, Action<bool> SelectionChanged,List<String> split, CollectorServices.BioFileInfo bioFileInfo)
        {
            if (split.Count > 0)
            {
                var dirName = split[0];
                if (!fileViewModel.Files.Any(item => item.Name == dirName))
                {
                    fileViewModel.Files.Add(new FileViewModel(dirName, SelectionChanged));
                }
                split.RemoveAt(0);
                CheckAndCreateRoute(fileViewModel.Files.First(item => item.Name == dirName), SelectionChanged, split, bioFileInfo);
            }
            else
            {
                var fileName = System.IO.Path.GetFileName(bioFileInfo.Filename);
                if (bioFileInfo.PathType == CollectorServices.BioFileInfo.EntityAtPathType.Regular)
                {
                    if (!fileViewModel.Files.Any(item => item.Name == fileName))
                    {
                        fileViewModel.Files.Add(new FileViewModel(fileName, SelectionChanged) { BioFileInfo = bioFileInfo });
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
                        fileViewModel.Files.Add(new FileViewModel(fileName, SelectionChanged) { BioFileInfo = bioFileInfo });
                    }
                }
            }
        }

    }

    public class FileViewModel : INotifyPropertyChanged
    {
        private ObservableCollection<FileViewModel> _files = new ObservableCollection<FileViewModel>();
        private Action<bool> _selectionChanged;

        public CollectorServices.BioFileInfo BioFileInfo = null;
        public ObservableCollection<FileViewModel> Files { get { return _files; } }
        public String Name { get; set; }

        private bool _isChecked;
        public bool IsChecked
        {
            get
            {
                return _isChecked;
            }
            set
            {
                _isChecked = value;

                _selectionChanged(_isChecked);
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
                if (BioFileInfo.PathType == CollectorServices.BioFileInfo.EntityAtPathType.Folder)
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
                
                if (BioFileInfo.PathType == CollectorServices.BioFileInfo.EntityAtPathType.Regular)
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
                }
                else if (BioFileInfo.PathType == CollectorServices.BioFileInfo.EntityAtPathType.Folder)
                {
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



        public FileViewModel(String filename, Action<bool> SelectionChanged)
        {
            this._selectionChanged = SelectionChanged;
            this.Name = filename;
        }




        // boiler-plate
        public event PropertyChangedEventHandler PropertyChanged;
        protected virtual void OnPropertyChanged(String propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null) handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
