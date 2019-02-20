#include "logging.h"


static HANDLE fh = INVALID_HANDLE_VALUE;

#define LOG_FILE		L"C:\\identazone_AP_debug.txt"

char * months[] = {
	"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

/*
 * Open the log file
 */

BOOLEAN
open_log(void)
{

	/* ALready open? */
	if (fh != INVALID_HANDLE_VALUE)
		return TRUE;

	/* Keep appending */
	fh = CreateFile (LOG_FILE, 
					FILE_APPEND_DATA,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					NULL, 
					OPEN_ALWAYS, 
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 
					NULL);

	if (fh == INVALID_HANDLE_VALUE)
	    return FALSE;

	iz_log("Log opening");
	return TRUE;

}

/*
 * Close the log
 */

BOOLEAN
close_log(void)
{

	if (fh == INVALID_HANDLE_VALUE)
		return TRUE;

	iz_log("Closing log");
	CloseHandle(fh);
	fh = INVALID_HANDLE_VALUE;

	return TRUE;

}
/*
 * Create a log entry in our logfile 
 */

int
iz_log (const char *format, ...)
{
  char bufTime[256];
  char buf[256];
  DWORD wr;
  DWORD ret;
  va_list ap;
  SYSTEMTIME systemTime;

  if (fh == INVALID_HANDLE_VALUE)
    return 0;

#if 1
  va_start (ap, format);
  StringCbVPrintfA(buf, 255, format, ap);
  va_end (ap);
#endif

  /* Get Current time and output to log */
  GetLocalTime(&systemTime);
  StringCbPrintfA(bufTime, 255, "[%02d-%03s-%04d %02d:%02d:%02d] ", 
	  systemTime.wDay,
	  months[systemTime.wMonth - 1],
	  systemTime.wYear,
	  systemTime.wHour,
	  systemTime.wMinute,
	  systemTime.wSecond
	  );

  for (ret = 0; ret < 256 && format[ret] != '\0'; ++ret);

  // Don't allow log entries to interleave
  EnterCriticalSection( &DllCriticalSection );
  WriteFile(fh, bufTime, (DWORD) strlen(bufTime), &wr, NULL);
  WriteFile(fh, buf, (DWORD) strlen(buf), &wr, NULL);
  WriteFile(fh, "\r\n", 2, &wr, NULL);

  FlushFileBuffers(fh);
  LeaveCriticalSection( &DllCriticalSection);
  
  
  return wr;
}