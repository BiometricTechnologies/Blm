#include "tarEncryptor.h"

namespace blm_utils {

    static boost::filesystem::path getLegalPathname(const boost::filesystem::path &pathName);

    static boost::filesystem::path getLegalPathname(const boost::filesystem::path &pathName) {
        boost::filesystem::path substitudePathname(pathName);
        int i = 1;

        while(boost::filesystem::exists(substitudePathname)) {
            std::wostringstream newEntryPathnameStrStream;
            newEntryPathnameStrStream << pathName.parent_path().wstring();
            newEntryPathnameStrStream << L"\\" << pathName.stem().wstring();
            newEntryPathnameStrStream << "(" << i << ")";
            newEntryPathnameStrStream << pathName.extension().wstring();
            substitudePathname = newEntryPathnameStrStream.str();
            i++;
        }
        return substitudePathname;
    }


    std::wstring TarEncryptor::getBiosecureFilePath() {
        return getTargetPath().wstring();
    }


    void TarEncryptor::produceTargetPath() {

        auto sourcePaths = getSourcePaths();
        if(sourcePaths.empty()) {
            throw std::logic_error("error: empty source list");
        }

        boost::filesystem::path desiredPathname;
        if(sourcePaths.size() > 1) {
            desiredPathname =  produceTargetPathForMultipleSources(sourcePaths);
        }  else if(sourcePaths.size() == 1) {
            desiredPathname = produceTargetPathForSingleSource(*sourcePaths.at(0));
        }

        setTargetPath(getLegalPathname(desiredPathname));
    }


    IArchiever::Result TarEncryptor::setSourcePathlist(const std::vector<SourcePathInfo> &sources) {
        for(auto &thisPath : sources) {
            addSourcePath(thisPath.path);
            this->produceTargetPath();
        }
        return IArchiever::Result::SUCCESS;
    }

    boost::filesystem::path TarEncryptor::produceTargetPathForMultipleSources(const std::vector<std::shared_ptr<boost::filesystem::path>> &sources) {
        auto resultPathStr = sources[0]->parent_path().wstring();
        resultPathStr.append(L"\\");
        resultPathStr.append(sources[0]->parent_path().filename().wstring());
        resultPathStr.append(biosecFileExtension());
        return boost::filesystem::path(resultPathStr);
    }


    boost::filesystem::path TarEncryptor::produceTargetPathForSingleSource(const boost::filesystem::path &source) {
		if(boost::filesystem::is_directory(source)) {
			return boost::filesystem::path(source.wstring() + biosecFileExtension());
		} else {
			boost::filesystem::path sourcePath(source);
			return boost::filesystem::path(sourcePath.replace_extension(biosecFileExtension()));
		}
    }

}

