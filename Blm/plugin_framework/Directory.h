#ifndef DIRECTORY_H
#define DIRECTORY_H

//----------------------------------------------------------------------

#include <apr.h>
#include <apr_file_info.h>
#include <string>
#ifdef PF_PLATFORM_LINUX
#include <dirent.h>
#include <sys/stat.h>
#endif

//----------------------------------------------------------------------

class Path;
  
namespace Directory
{
  // check if a directory exists
  bool exists(const std::string & path);

  // get executable directory
  std::string getExecutablePath();

  // get current working directory
  std::string getCWD();

  // set current working directories
  void setCWD(const std::string & path);

  // Copy directory tree rooted in 'source' to 'destination'
  void copyTree(const std::string & source, const std::string & destination);

  // Remove directory tree rooted in 'path'
  void removeTree(const std::string & path);

  // Create directory 'path' including all parent directories if missing
  void create(const std::string & path);
  struct Entry  
  #ifndef PF_PLATFORM_LINUX
    : public apr_finfo_t
  #endif
  {
    enum Type { FILE, DIRECTORY, LINK };

    Type type;
    std::string path;
  };

  class Iterator
  {
  public:

    Iterator(const Path & path);    
    Iterator(const std::string & path);
    ~Iterator();

    // Resets directory to start. Subsequent call to next() 
    // will retrieve the first entry
    void reset();
    // get next directory entry
    Entry * next(Entry & e);

  private:
    Iterator();
    Iterator(const Iterator &);

    void init(const std::string & path);
  private:
    std::string path_;
#ifdef PF_PLATFORM_LINUX
    DIR * handle_;
#else
    apr_dir_t * handle_;
    apr_pool_t * pool_;
#endif    
  };
}


#endif // DIRECTORY_H


