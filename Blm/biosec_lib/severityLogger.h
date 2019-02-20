#ifndef __severityLogger_h__
#define __severityLogger_h__


#include <fstream>
#include <string>
#include <mutex>


#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/sources/record_ostream.hpp>


namespace blm_utils {


    enum class SeverityLevel {
        NORMAL,
        NOTIFICATION,
        WARNING,
        ERR,
        CRITICAL
    };


    inline std::ostream &operator<<(std::ostream &ostream, SeverityLevel severity) {
        switch(severity) {
            case SeverityLevel::NORMAL:
                ostream << "NORMAL";
                break;
            case SeverityLevel::NOTIFICATION:
                ostream << "NOTIFY";
                break;
            case SeverityLevel::WARNING:
                ostream << "WARNING";
                break;
            case SeverityLevel::ERR:
                ostream << "ERROR";
                break;
            case SeverityLevel::CRITICAL:
                ostream << "CRITICAL";
                break;
            default:
                break;
        }
        return ostream;
    }


    BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
    BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", SeverityLevel)


	static std::once_flag onceFlag;

    class SeverityLogger {
    public:
        SeverityLogger() {
			std::call_once(onceFlag, &SeverityLogger::initSink, this);
        }

		

		void log(SeverityLevel sev, const std::string& message){
			BOOST_LOG_SEV(logger_, sev) << message;
		}


		void log(SeverityLevel sev){
			BOOST_LOG_SEV(logger_, sev) << unknownMessage();
		}


    private:
		static const char* unknownMessage(){
			return "Unknown";
		}

		void initSink(){
			boost::shared_ptr<boost::log::sinks::text_file_backend> backend =
				boost::make_shared<boost::log::sinks::text_file_backend>(
				boost::log::keywords::file_name = this->logFilePath(),
				boost::log::keywords::rotation_size = 5 * 1024 * 1024,
				boost::log::keywords::open_mode = std::ios_base::out | std::ios_base::app,
				boost::log::keywords::auto_flush = true
				);

			typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend> sink_t;
			boost::shared_ptr<sink_t> sink(new sink_t(backend));

			boost::log::formatter fmt = boost::log::expressions::stream
				<< boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
				// << "line " << std::setw(6) << std::setfill('0') << line_id << std::setfill(' ')
				<< ": <" << severity << ">\t"
				<< boost::log::expressions::smessage;

			sink->set_formatter(fmt);
#define DEBUG
#ifdef DEBUG
			sink->set_filter(severity >= SeverityLevel::NORMAL);
#else
			sink->set_filter(severity >= SeverityLevel::WARNING);
#endif
#undef  DEBUG
			boost::log::core::get()->add_sink(sink);
			boost::log::add_common_attributes();
		}
		
		static const char* logFilePath() {
			return "C:\\logs\\IdentaZone\\Biosec\\biosecure_lib_log_%N.txt";
		}

        boost::log::sources::severity_logger<SeverityLevel> logger_;
		
    };


} // namespace

#endif // __severityLogger_h__
