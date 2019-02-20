#include <string>
#include <algorithm>
#include "base.h"
#include "Directory.h"
#include "Path.h"
#include <apr_file_io.h>

#ifdef WIN32
  #include <windows.h>
  #include <tchar.h>
#else
  #include <sys/stat.h>
#endif

#include "blm_exception.h"

namespace Directory
{
  bool exists(const std::string & path)
  {
    return Path::exists(path);
  }
  
  std::string getExecutablePath(){
	  char cwd[APR_PATH_MAX];
	  DWORD length = GetModuleFileName( NULL, cwd, APR_PATH_MAX );
	  std::string str(cwd);
	  std::string pat("\\");
	  std::string::iterator it = std::find_end(std::begin(str), std::end(str), std::begin(pat), std::end(pat));
	  if(it ==  str.end()){
		  throw blm_utils::BlmException("Wrong executable path", __FILE__, __FUNCTION__, __LINE__);
	  }
	  ++it;
	  *it = 0;
	  return str;
  }

  std::string getCWD()
  {
    char cwd[APR_PATH_MAX];
  #ifdef WIN32
    DWORD res = ::GetCurrentDirectoryA(APR_PATH_MAX, cwd);
    CHECK(res > 0) << "Couldn't get current working directory. Error code: " 
      << base::getErrorMessage();
  #else
    cwd[0] = '\0';
    char * res = ::getcwd(cwd, APR_PATH_MAX);
    CHECK(res != NULL) << "Couldn't get current working directory. Error code: " << errno;
    
  #endif
    return std::string(cwd);
  }
  
  void setCWD(const std::string & path)
  {
    int res = 0;
  #ifdef WIN32
    res = ::SetCurrentDirectoryA(path.c_str()) ? 0 : -1;
  #else
    res = ::chdir(path.c_str());
  #endif
  
    CHECK(res == 0) << base::getErrorMessage();
  }

  static void removeEmptyDir(const std::string & path)
  {
    int res = 0;
  #ifdef WIN32
    res = ::RemoveDirectoryA(path.c_str()) ? 0 : -1;
  #else
    res = ::rmdir(path.c_str());
  #endif
    CHECK(res == 0) << base::getErrorMessage();
  }



  void copyTree(const std::string & source, const std::string & destination)
  {
    CHECK(Path::isDirectory(source));
    std::string baseSource(Path::getBasename(source));
    std::string dest(destination);
    dest += baseSource;
    if (!Path::exists(dest))
      Directory::create(dest);
    CHECK(Path::isDirectory(dest));
    
    Iterator i(source);
    Entry e;
    while (i.next(e))
    {
      std::string fullSource(source);
      fullSource += e.path;
      Path::copy(fullSource, dest);
    }
  }

  
  void removeTree(const std::string & path)
  {
    CHECK(!path.empty()) << "Can't remove directory with no name";
    
    Iterator i(path);
    Entry e;
    while (i.next(e))
    {
      Path fullPath = Path(path) + Path(e.path);
      if (e.type == Entry::DIRECTORY)
        removeTree(std::string(fullPath));
      else
      {
        apr_status_t st = ::apr_file_remove(fullPath, NULL);
        CHECK(st == APR_SUCCESS) << base::getErrorMessage();
      }
    }
    
    removeEmptyDir(path);
  }

  static void createSingleDir(const char * path)
  {
    bool success;
  #ifdef WIN32
      success = ::CreateDirectoryA(path, NULL) != FALSE;
  #else
      int res = ::mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      success = res == 0;
  #endif
    CHECK(success) << "Faile to create directory. " << base::getErrorMessage();
  }
  
  // Create directory recursively (creates parent if doesn't exist)
  void create(const std::string & path)
  {
    CHECK(!path.empty()) << "Can't create directory with no name";
    std::string p = Path::makeAbsolute(path);
    
    if (Path::exists(p))
    {
      CHECK(Path::isDirectory(p)) 
        << path << " already exists but it's not a directory";
        
      return;
    }

    std::string parent = Path::getParent(p);
    
    if (!Directory::exists(parent))
      create(parent);
      
    createSingleDir(p.c_str());
  }

  Iterator::Iterator(const Path & path)
  {
    init(std::string(path));
  }
      
  Iterator::Iterator(const std::string & path) : handle_(NULL)
  {
    init(path);
  }

  void Iterator::init(const std::string & path)
  {
    std::string absolutePath = Path::makeAbsolute(path);
  #ifdef PF_PLATFORM_LINUX
    handle_ = ::opendir(absolutePath.c_str());
    CHECK(handle_) << "Can't open directory " << path
                   << ". Error code: "    << errno;

  #else
    apr_status_t res = apr_pool_create(&pool_, NULL);
    CHECK(res == 0) << "Can't create pool";
    res = ::apr_dir_open(&handle_, absolutePath.c_str(), pool_);
    CHECK(res == 0) << "Can't open directory " << path
                    << ". Error code: " << APR_TO_OS_ERROR(res);

  #endif
  }
  
  Iterator::~Iterator()
  {
	  try{  
		#ifdef PF_PLATFORM_LINUX
		  int res = ::closedir(handle_);
		  CHECK(res == 0) << "Couldn't close directory." 
			  << " Error code: " << errno;
		#else
		  apr_status_t res = ::apr_dir_close(handle_);
		  apr_pool_destroy(pool_);
		  CHECK(res == 0) << "Couldn't close directory." 
			  << " Error code: " << APR_TO_OS_ERROR(res);
		#endif
	  }
	  catch(const StreamingException &e){

	  }

  }
  
  void Iterator::reset()
  {
  #ifdef PF_PLATFORM_LINUX
    ::rewinddir(handle_);
  #else
    apr_status_t res = ::apr_dir_rewind(handle_);
    CHECK(res == 0) 
      << "Couldn't reset directory iterator." 
      << " Error code: " << APR_TO_OS_ERROR(res);
  #endif
  }

#ifdef PF_PLATFORM_LINUX  
  Entry * Iterator::next(Entry & e)
  {
    errno = 0;
    struct dirent * p = ::readdir(handle_);
    CHECK(!errno) << "Couldn't read next dir entry." 
                  << " Error code: " << errno;
    
    if (!p)
      return NULL;
    
    e.type = (p->d_type == DT_DIR) ? Directory::Entry::DIRECTORY 
                                   : Directory::Entry::FILE;
    
    e.path = std::string(p->d_name);                               

    // Skip '.' and '..' directories
    if (e.type == Directory::Entry::DIRECTORY && 
       (e.path == std::string(".") || e.path == std::string("..")))
      return next(e);
    else
      return &e;
  }
  
#else  
  Entry * Iterator::next(Entry & e)
  {
    apr_int32_t wanted = APR_FINFO_LINK | APR_FINFO_NAME | APR_FINFO_TYPE;
    apr_status_t res = ::apr_dir_read(&e, wanted, handle_);
    
    // No more entries
    if (APR_STATUS_IS_ENOENT(res))
      return NULL;
      
    if (res != 0)
    {
      CHECK(res == APR_INCOMPLETE) 
        << "Couldn't read next dir entry." 
        << " Error code: " << APR_TO_OS_ERROR(res);
      CHECK(((e.valid & wanted) | APR_FINFO_LINK) == wanted) 
        << "Couldn't retrieve all fields. Valid mask=" << e.valid; 
    } 

    
    e.type = (e.filetype == APR_DIR) ? Directory::Entry::DIRECTORY 
                                       : Directory::Entry::FILE;
    e.path = e.name;                               

    // Skip '.' and '..' directories
    if (e.type == Directory::Entry::DIRECTORY && 
       (e.name == std::string(".") || e.name == std::string("..")))
      return next(e);
    else
      return &e;
  }
#endif
  
}

