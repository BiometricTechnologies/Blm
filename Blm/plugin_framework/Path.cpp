#include "base.h"
#include "Path.h"
#include "Directory.h"
#include <boost/tokenizer.hpp>
#include <iterator>



#ifdef WIN32
  #include <windows.h>

  const char * Path::sep = "\\";
#else
  #include <sys/stat.h>
  #include <fstream>

  const char * Path::sep = "/";
#endif

Path::Path(const std::string & path) : path_(path)
{
}

static apr_status_t getInfo(const std::string & path, apr_int32_t wanted, apr_finfo_t & info)
{
  CHECK(!path.empty()) << "Can't get the info of an empty path";

  apr_status_t res;
  apr_pool_t * pool = NULL;
  
#ifdef WIN32 
  res = ::apr_pool_create(&pool, NULL);
#endif
  
  res = ::apr_stat(&info, path.c_str(), wanted, pool);
  
#ifdef WIN32
  ::apr_pool_destroy(pool);
#endif
  
  return res;
} 

bool Path::exists(const std::string & path)
{
  if (path.empty())
    return false;
  
  apr_finfo_t st;      
  apr_status_t res  = getInfo(path, APR_FINFO_TYPE, st);
  return res == APR_SUCCESS;
}

static apr_filetype_e getType(const std::string & path)
{
  apr_finfo_t st;
  apr_status_t res = getInfo(path, APR_FINFO_TYPE, st);
  CHECK(res == APR_SUCCESS) 
    << "Can't get info for '" << path << "', " << base::getErrorMessage();
  
  return st.filetype;
}


bool Path::isFile(const std::string & path)
{
  return getType(path) == APR_REG;
}

bool Path::isDirectory(const std::string & path)
{
  return getType(path) == APR_DIR;
}

bool Path::isSymbolicLink(const std::string & path)
{
  return getType(path) == APR_LNK;
}
  
bool Path::isAbsolute(const std::string & path)
{
  CHECK(!path.empty()) << "Empty path is invalid";
#ifdef WIN32
  if (path.size() < 2)
    return false;
  else
  return path[1] == ':';
#else
  return path[0] == '/';
#endif
}

bool Path::areEquivalent(const std::string & path1, const std::string & path2)
{
  apr_finfo_t st1;
  apr_finfo_t st2;
  apr_int32_t wanted = APR_FINFO_IDENT;
  getInfo(path1.c_str(), wanted, st1);
  getInfo(path2.c_str(), wanted, st2);
  bool res = true;
  res &= st1.device == st2.device;
  res &= st1.inode == st2.inode;
  res &= std::string(st1.fname) == std::string(st2.fname);
  
  return res;
}

std::string Path::getParent(const std::string & path)
{
  Path::StringVec sv;
  Path::split(path, sv);
  std::string root;
  if (path[0] == '/')
    root = "/";
  return root + Path::join(sv.begin(), sv.end()-1);
}

std::string Path::getBasename(const std::string & path)
{
  std::string::size_type index = path.find_last_of(Path::sep);
  
  if (index == std::string::npos)
    return path;
  
  return std::string(path.c_str() + index + 1, path.size() - index - 1);
}

std::string Path::getExtension(const std::string & path)
{
  std::string filename = Path::getBasename(path);
  std::string::size_type index = filename.find_last_of('.');
  
  // Don't include the dot, just the extension itself (unlike Python)
  return filename.substr(index + 1);
}

apr_size_t Path::getFileSize(const std::string & path)
{
  apr_finfo_t st;
  apr_int32_t wanted = APR_FINFO_TYPE | APR_FINFO_SIZE;
  getInfo(path.c_str(), wanted, st);
  CHECK(st.filetype == APR_REG) << "Can't get the size of a non-file object";
  
  return (apr_size_t)st.size;
}

std::string Path::normalize(const std::string & path)
{
  return path;
}

std::string Path::makeAbsolute(const std::string & path)
{
  if (Path::isAbsolute(path))
    return path;
    
  //std::string cwd = Directory::getCWD();
  std::string cwd = Directory::getExecutablePath();
  // If its already absolute just return the original path
  if (::strncmp(cwd.c_str(), path.c_str(), cwd.length()) == 0) 
    return path;
  
  // Get rid of trailing separators if any
  if (path.find_last_of(Path::sep) == path.length() - 1)
  {
    cwd = std::string(cwd.c_str(), cwd.length()-1);
  }
  // join the cwd to the path and return it (handle duplicate separators) 
  std::string result = cwd;
  if (path.find_first_of(Path::sep) == 0)
  {
    return cwd + path;
  }
  else
  {
    return cwd + Path::sep + path;
  }
  
  return "";
}

void Path::split(const std::string & path, StringVec & parts)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

  tokenizer tokens(path, boost::char_separator<char>(Path::sep));
  
  std::copy(tokens.begin(), tokens.end(), std::inserter(parts, parts.begin()));
}

std::string Path::join(StringVec::iterator begin, StringVec::iterator end)
{
  // Need to get rid of redundant separators

  if (begin == end)
    return "";
    
  std::string path(*begin++);
  
  while (begin != end)
  {
    path += Path::sep;
    path += *begin++;
  };
  
  return path;
}


void Path::copy(const std::string & source, const std::string & destination)
{
  CHECK(!source.empty()) 
    << "Can't copy from an empty source";

  CHECK(!destination.empty()) 
    << "Can't copy to an empty destination";

  CHECK(source != destination)
    << "Source and destination must be different";
    
  if (isDirectory(source))
  {
    Directory::copyTree(source, destination);
    return;
  } 

#ifdef WIN32
  // This will overwrite quitely destination file if exist
  BOOL res = ::CopyFile(LPTSTR(source.c_str()), LPTSTR(destination.c_str()), FALSE);
  CHECK(res != FALSE) << base::getErrorMessage();
#else

  try
  {
    std::ifstream  in(source.c_str());
    std::ofstream  out(destination.c_str()); 
    in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    out.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    out << in.rdbuf();
  }
  catch (...)
  {
    // Should I do it? maybe just let the standard exception propagate on its own?
    THROW << "Path::copy() failed. " << base::getErrorMessage();
  }
#endif
}

void Path::remove(const std::string & path)
{
  CHECK(!path.empty()) 
    << "Can't remove an empty path";

  // Just return if it doesn't exist already
  if (!Path::exists(path))
    return;
    
  if (isDirectory(path))
  {
    Directory::removeTree(path);
    return;
  } 

#ifdef WIN32
  BOOL res = ::DeleteFile(LPTSTR(path.c_str()));
  CHECK(res != FALSE) << base::getErrorMessage();
#else
  int res = ::remove(path.c_str());
  CHECK(res == 0) << base::getErrorMessage();
#endif
}
  
void Path::rename(const std::string & oldPath, const std::string & newPath)
{
  CHECK(!oldPath.empty() && !newPath.empty()) 
    << "Can't rename to/from empty path";
#ifdef WIN32
  BOOL res = ::MoveFile(LPTSTR(oldPath.c_str()), LPTSTR(newPath.c_str()));
  CHECK(res != FALSE) << base::getErrorMessage();
#else
  int res = ::rename(oldPath.c_str(), newPath.c_str());
  CHECK(res != -1) << base::getErrorMessage();
#endif
}

Path::operator const char *() const
{
  return path_.c_str();
}

Path & Path::operator+=(const Path & path)
{
  Path::StringVec sv;
  sv.push_back(std::string(path_));
  sv.push_back(std::string(path.path_));
  path_ = Path::join(sv.begin(), sv.end());
  return *this;
}

Path Path::getParent() const
{
  return Path::getParent(path_);
}

Path Path::getBasename() const
{
  return Path::getBasename(path_);
}

Path Path::getExtension() const
{
  return Path::getExtension(path_); 
}

apr_size_t Path::getFileSize() const
{
  return Path::getFileSize(path_); 
}
  
Path & Path::normalize()
{
  path_ = Path::normalize(path_);
  return *this;
}

Path & Path::makeAbsolute()
{
  if (!isAbsolute())
    path_ = Path::makeAbsolute(path_);
  return *this;
}
  
void Path::split(StringVec & parts) const
{
  Path::split(path_, parts);
}

void Path::remove() const
{
  Path::remove(path_);
}
  
void Path::rename(const std::string & newPath)
{
  Path::rename(path_, newPath);
  path_ = newPath;
}

bool Path::isDirectory() const
{
  return Path::isDirectory(path_);
}

bool Path::isFile() const
{
  return Path::isFile(path_);
}

bool Path::isSymbolicLink() const
{
  return Path::isSymbolicLink(path_);
}  
bool Path::isAbsolute() const
{
  return Path::isAbsolute(path_);
}

bool Path::exists() const
{
  return Path::exists(path_);
}

bool Path::isEmpty() const
{
  return path_.empty();
}

Path operator+(const Path & p1, const Path & p2)
{
  Path::StringVec sv;
  sv.push_back(std::string(p1));
  sv.push_back(std::string(p2));
  return Path::join(sv.begin(), sv.end());
}


