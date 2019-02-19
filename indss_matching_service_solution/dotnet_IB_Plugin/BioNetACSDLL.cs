/*
****************************************************************************************************
* BioNetAcsDll.h
*
* DESCRIPTION:
*     C# interface to BioNetAcsDLL.dll.  (/unsafe project modifier required)
*     http://www.integratedbiometrics.com
*
* NOTES:
*     Copyright (c) Integrated Biometrics, 2009-2013
*
* HISTORY:
*     2004/04/24  Created.
*     2010/07/26  Add ControlLED().
*     2012/02/15  Add functions and optimize.
*     2013/07/25  Add GetSecurityLevel() and PutSecurityLevel().
*     2013/09/04  Correct prototype of ChangeGain() and improve comments.  Add miscellaneous 
*                 constants.
****************************************************************************************************
*/

using System;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Text;

namespace BioNetACSLib
{
    [ComVisible(false)]
    public class BioNetACSDLL
    {
        private const string DLLFilename = "BioNetAcsDll.dll";

		/*
		********************************************************************************************
		* GLOBAL CONSTANTS
		********************************************************************************************
		*/

		/* Return values of _IsOperatingUsbMode(). */
		public const int NA2_USB_HIGH = 1;
		public const int NA2_USB_LOW  = 0;

		/* Return values of _IB_comp_nfiq(). */
		public const int EMPTY_IMG        = 1;
		public const int TOO_FEW_MINUTIAE = 2;

		/* Input to _GetPreProcessingImage(). */
		public const int GABOR    = 0;
		public const int BINARY   = 1;
		public const int THINNING = 2;

        public const int SIZE_WIDTH = 352;
        public const int SIZE_HEIGHT = 288;
        public const int SIZE_IMAGE = SIZE_WIDTH * SIZE_HEIGHT;
        public const int SIZE_FEAT_STORAGE = 9456;  // Full 9052 template storage size

		/*
		********************************************************************************************
		* _GetSDKVersion()
		* 
		* DESCRIPTION:
		*     Get SDK version.
		********************************************************************************************
		*/
        [DllImport(DLLFilename, CallingConvention = CallingConvention.StdCall)]
        private static extern void GetSDKVersion([Out] StringBuilder sVersion);
        
        public static string _GetSDKVersion()
        {
            var sVersion = new StringBuilder(100);
            GetSDKVersion(sVersion);
            string result = sVersion.ToString();
            return result;
        }

		/*
		********************************************************************************************
		* GLOBAL FUNCTIONS (CAPTURE)
		********************************************************************************************
		*/

		/*
		********************************************************************************************
		* _GetImgWidth()
		* 
		* DESCRIPTION:
		*     Get Width of finger print Image acquired by scanner.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short GetImgWidth();
        
        public static int _GetImgWidth()
        {
            int result = (int)GetImgWidth();
            return result;
        }

		/*
		********************************************************************************************
		* _GetImgHeight()
		* 
		* DESCRIPTION:
		*     Get Height of finger print Image acquired by scanner.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short GetImgHeight();

        public static int _GetImgHeight()
        {
            int result = (int)GetImgHeight();
            return result;
        }

		/*
		********************************************************************************************
		* _GetImgSize()
		* 
		* DESCRIPTION:
		*     Get size of finger print Image acquired by scanner.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern long GetImgSize();

        public static int _GetImgSize()
        {
            int result = (int)GetImgSize();
            return result;
        }

		/*
		********************************************************************************************
		* _GetUSBKey()
		* 
		* DESCRIPTION:
		*     Get unique USB key stored in scanner during manufacture.  The embedded key value must 
		*     be displayed or interpreted as an unsigned long (the printf format "%lu").
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern long GetUSBKey();
        
        public static int _GetUSBKey()
        {
            int result = (int)GetUSBKey();
            return result;
        }

		/*
		********************************************************************************************
		* _GetUSN()
		* 
		* DESCRIPTION:
		*     Get embedded USN (Unique Serial Number) for first connected scanner.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern long GetUSN([Out] StringBuilder szUSN);

        public static string _GetUSN()
        {
            var usn = new StringBuilder(1000);
            GetUSN(usn);
            string result = usn.ToString();
            return result;
        }

		/*
		********************************************************************************************
		* _GetUSNList()
		* 
		* DESCRIPTION:
		*     Get comma-separated list of embedded USNs (Unique Serial Numbers) for all connected scanners.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern long GetUSNList([Out] StringBuilder szUSNList);
        
        public static string _GetUSNList()
        {
            var snList = new StringBuilder(1000);

            GetUSNList(snList);
            string result = snList.ToString();
            return result;
        }

		/*
		********************************************************************************************
		* _IsOperatingUsbMOde()
		* 
		* DESCRIPTION:
		*     Determine whether the scanner is connected to a USB 1.1 or 2.0 port.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short IsOperatingUsbMode();
        
        public static int _IsOperatingUsbMode()
        {
            int result = (int)IsOperatingUsbMode();
            return result;
        }

		/*
		********************************************************************************************
		* _OpenNetAccessDevice()
		* 
		* DESCRIPTION:
		*     Initialize default fingerprint scanner.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short OpenNetAccessDevice();
        
        public static int _OpenNetAccessDevice()
        {
            int result = (int)OpenNetAccessDevice();
            return result;
        }

		/*
		********************************************************************************************
		* _OpenNetAccessDeviceByUSN()
		* 
		* DESCRIPTION:
		*     Initialize fingerprint scanner with specified USN (Unique Serial Number).
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern int OpenNetAccessDeviceByUSN(byte[] usn);
        
        public static int _OpenNetAccessDeviceByUSN(string usn)
        {
            int result = OpenNetAccessDeviceByUSN(System.Text.ASCIIEncoding.UTF8.GetBytes(usn));
            return result;
        }

		/*
		********************************************************************************************
		* _CloseNetAccessDevice()
		* 
		* DESCRIPTION:
		*     Free resources associated with scanner initialized with OpenNetAccessDevice() or  
		*     OpenNetAccessDeviceByUSN().  The scanner should always be closed when scanner accesses have
		*     concluded.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short CloseNetAccessDevice();
        
        public static int _CloseNetAccessDevice()
        {
            int result = (int)CloseNetAccessDevice();
            return result;
        }

		/*
		********************************************************************************************
		* _GetNetAccessImage()
		* 
		* DESCRIPTION:
		*     Get an Image from the fingerprint scanner.  The sensor gain will be adjusted automatically for
		*     optimum result, and the capture quality will be evaluated as indicated by the return value.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short GetNetAccessImage([Out] byte[] pSourceImg);
        
        public static int _GetNetAccessImage(byte[] pSourceImg)
        {
            int result = (int)GetNetAccessImage(pSourceImg);
            return result;
        }

		/*
		********************************************************************************************
		* _GetNetAccessImageByManual()
		* 
		* DESCRIPTION:
		*     Get a raw Image at the gain set by ChangeGain() from the fingerprint scanner.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short GetNetAccessImageByManual([Out] byte[] pSourceImg);

        public static int _GetNetAccessImageByManual(byte[] pSourceImg)
        {
            int result = 0;
            result = (int)GetNetAccessImageByManual(pSourceImg);
            return result;
        }

		/*
		********************************************************************************************
		* _GetDeviceStatus()
		* 
		* DESCRIPTION:
		*     Get connection status between scanner and host.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short GetDeviceStatus();
        
        public static int _GetDeviceStatus()
        {
            int result = (int)GetDeviceStatus();
            return result;
        }

		/*
		********************************************************************************************
		* _ChangeGain()
		* 
		* DESCRIPTION:
		*     Change scanner gain for capture with GetNetAccessImageByManual() or 
		*     GetNetAccessLargeImageByManual().
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short ChangeGain(byte gain);
        
        public static int _ChangeGain(byte gain)
        {
            int result = (int)ChangeGain(gain);
            return result;
        }

		/*
		********************************************************************************************
		* _ControlLED()
		* 
		* DESCRIPTION:
		*     Turn LEDs on or off.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short ControlLED(byte redLed, byte greenLed, byte blueLed);

        public static short _ControlLED(bool redLed, bool greenLed, bool blueLed)
        {
            byte red = redLed ? (byte)1 : (byte)0;
            byte green = greenLed ? (byte)1 : (byte)0;
            byte blue = blueLed ? (byte)1 : (byte)0;
            short result = ControlLED(red, green, blue);
            return result;
        }

		/*
		********************************************************************************************
		* _ControlTOUCH()
		* 
		* DESCRIPTION:
		*     Turn touch sensor on or off.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short ControlTOUCH(byte TOUCH_ONOFF);

        public static short _ControlTOUCH(bool TOUCH_ON)
        {
            byte touchOn = TOUCH_ON ? (byte)1 : (byte)0;
            short result = ControlTOUCH(touchOn);
            return result;
        }

		/*
		********************************************************************************************
		* GLOBAL FUNCTIONS (MATCHING)
		********************************************************************************************
		*/

		/*
		********************************************************************************************
		* _AlgoInit()
		* 
		* DESCRIPTION:
		*     Allocate memory necessary for template extraction and matching.  This must be called only once
		*     before any other extracting and matching functions are called.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short AlgoInit();
        
        public static int _AlgoInit()
        {
            int result = (int)AlgoInit();
            return result;
        }

		/*
		********************************************************************************************
		* _PutSecurityLevel()
		* 
		* DESCRIPTION:
		*     Set security level for matching templates.  The seven security levels determine the 
		*     tradeoff between the expected FAR (False Acceptance Rate) and FRR (False Rejection Rate) from
		*     template comparison.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern void PutSecurityLevel(short nLevel);
        
        public static void _PutSecurityLevel(int nLevel)
        {
            PutSecurityLevel((short)nLevel);
        }

		/*
		********************************************************************************************
		* _GetSecurityLevel()
		* 
		* DESCRIPTION:
		*     Get security level for matching templates.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short GetSecurityLevel();
        
        public static int _GetSecurityLevel()
        {
            int result = (int)GetSecurityLevel();
            return result;
        }

		/*
		********************************************************************************************
		* _PutIndexingLevel()
		* 
		* DESCRIPTION:
		*     Set indexing level for generating templates.  Features are provided for recognizing and 
		*     recording fingerprint Image traits common in fingerprints.  For one-to-many comparisons in 
		*     larger databases (> 50,000 templates), a small increase in search speed will decrease overall
		*     matching time.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)] 
        private static extern void PutIndexingLevel(short nLevel);
        
        public static void _PutIndexingLevel(int nLevel)
        {
            PutIndexingLevel((short)nLevel);
        }

		/*
		********************************************************************************************
		* _GetIndexingLevel()
		* 
		* DESCRIPTION:
		*     Get indexing level for generating templates.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short GetIndexingLevel();
        
        public static int _GetIndexingLevel()
        {
            int result = 0;
            result = (int)GetIndexingLevel();
            return result;
        }

		/*
		********************************************************************************************
		* _GetFeatSize()
		* 
		* DESCRIPTION:
		*     Get size of feature data of templates.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short GetFeatSize();
        
        public static int _GetFeatSize()
        {
            int result = (int)GetFeatSize();
            return result;
        }

		/*
		********************************************************************************************
		* _ExtractFt()
		* 
		* DESCRIPTION:
		*     Extract feature data from a 352x288 fingerprint Image.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short ExtractFt([In] byte[] pImg, [Out] byte[] pFp);
        
        public static int _ExtractFt(byte[] pImg, byte[] pFp)
        {
            int result = (int)ExtractFt(pImg, pFp);
            return result;
        }

		/*
		********************************************************************************************
		* _Extract404From9456Template()
		* 
		* DESCRIPTION:
		*     Extract 404-byte template from full 9456-byte template.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern void Extract404From9456Template([In] byte[] pFt9456, [Out] byte[] pFt404);

        public static byte[] _Extract404From9456Template(byte[] ft9456)
        {
            var p404 = new byte[404];
            Extract404From9456Template(ft9456, p404);
            return p404;
        }

		/*
		********************************************************************************************
		* _CompareFt404vs404()
		* 
		* DESCRIPTION:
		*     Compare two 404-byte fingerprint templates.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern long CompareFt404vs404(byte[] pFt404First, byte[] pFt404Second);
        
        public static int _CompareFt404vs404([In] byte[] ft404First, [In] byte[] ft404Second)
        {
            int result = (int)CompareFt404vs404(ft404First, ft404Second);
            return result;
        }

		/*
		********************************************************************************************
		* _CompareFt404vs9052()
		* 
		* DESCRIPTION:
		*     Compare a first 404-byte and a second 9052- or 9456-byte fingerprint template.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern long CompareFt404vs9052([In] byte[] pFt404First, [In] byte[] pFt9052Second);
        
        public static int _CompareFt404vs9052(byte[] ft404First, byte[] ft9052Second)
        {
            int result = (int)CompareFt404vs9052(ft404First, ft9052Second);
            return result;
        }

		/*
		********************************************************************************************
		* _CompareFt9052vs9052()
		* 
		* DESCRIPTION:
		*     Compare a first 9052- or 9456-byte and a 9052-byte or 9456-byte fingerprint template.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern long CompareFt9052vs9052([In] byte[] pFt9052First, [In] byte[] pFt9052Second);
        
        public static int _CompareFt9052vs9052(byte[] ft9052First, byte[] ft9052Second)
        {
            int result = (int)CompareFt9052vs9052(ft9052First, ft9052Second);
            return result;
        }

		/*
		********************************************************************************************
		* _Enroll_SingleTemplate()
		* 
		* DESCRIPTION:
		*     Generate template for enrollment from three fingerprint images.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short Enroll_SingleTemplate([In] byte[] Img1, [In] byte[] Img2, [In] byte[] Img3,
            [Out] byte[] Fea1, [Out] byte[] Fea2, [Out] byte[] Fea3,
            [Out] byte[] ResultTemplate);
            
        public static short _Enroll_SingleTemplate(byte[] Img1, byte[] Img2, byte[] Img3,
            byte[] Fea1, byte[] Fea2, byte[] Fea3,
            byte[] ResultTemplate)
        {
            short result =  Enroll_SingleTemplate(Img1, Img2, Img3, Fea1, Fea2, Fea3, ResultTemplate);
            return result;
        }

		/*
		********************************************************************************************
		* _Enroll_MultiTemplate()
		* 
		* DESCRIPTION:
		*     Generate template for enrollment from three templates and three fingerprint images.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short Enroll_MultiTemplate([In] byte[] Fea1, [In] byte[] Fea2, [In] byte[] Fea3,
            [In] byte[] Img4, [In] byte[] Img5, [In] byte[] Img6,
            [Out] byte[] ResultTemplate1, [Out] byte[] ResultTemplate2);
            
        public static short _Enroll_MultiTemplate(byte[] Fea1, byte[] Fea2, byte[] Fea3,
            byte[] Img4, byte[] Img5, byte[] Img6,
            byte[] ResultTemplate1, byte[] ResultTemplate2)
        {
            short result = Enroll_MultiTemplate(Fea1, Fea2, Fea3, Img4, Img5, Img6, ResultTemplate1, ResultTemplate2);
            return result;
        }

		/*
		********************************************************************************************
		* _GetPreProcessingImage()
		* 
		* DESCRIPTION:
		*     Get a pre-processing Image from last template extraction.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short GetPreProcessingImage(int level, byte[] img);

        public static byte[] _GetPreProcessingImage(int level)
        {
            var preprocImage = new byte[SIZE_IMAGE];
            var rc = GetPreProcessingImage(level, preprocImage);
            if (rc == 0) preprocImage = null;  // Error

            return preprocImage;
        }

		/*
		********************************************************************************************
		* GLOBAL FUNCTIONS (IMAGE PROCESSING)
		********************************************************************************************
		*/

		/*
		********************************************************************************************
		* _GetIBQualityScore()
		* 
		* DESCRIPTION:
		*     Calculate quality score from fingerprint Image using IB's algorithm.  This function is 
		*     experimental and not thread-safe.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern short GetIBQualityScore([In] byte[] pImg);

        public static short _GetIBQualityScore(byte[] img)
        {
            return GetIBQualityScore(img);
        }

		/*
		********************************************************************************************
		* _EnhanceImage()
		* 
		* DESCRIPTION:
		*     Improve Image characteristics.  Typically, light images become darker, but the contrast 
		*     between light and dark areas is also increased.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern void EnhanceImage([In] byte[] pInputImg, [Out] byte[] pOutputImg);

        public static byte[] _EnhanceImage(byte[] inputImg)
        {
            var enhancedImage = new byte[SIZE_IMAGE];
            EnhanceImage(inputImg, enhancedImage);
            return enhancedImage;
        }

		/*
		********************************************************************************************
		* _IB_comp_nfiq()
		*
		* DESCRIPTION:
		*     Compute NFIQ score from a 352x288 fingerprint Image.  Default statistics are used for 
		*     Z-normalization, and default weights are used for MLP classification.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static unsafe extern int IB_comp_nfiq([In] byte[] pImg, byte* onfiq);

        public static byte _IB_comp_nfiq(byte[] img)
        {
            int result = 0;
            byte onfiq = 0;
            unsafe
            {
                result = IB_comp_nfiq(img, &onfiq);
            }
            if (result == 0) 
                return onfiq;
            else 
                return 0;
        }

		/*
		********************************************************************************************
		* _IB_WSQCompressImage()
		*
		* DESCRIPTION:
		*     Compress 352x288 Image with WSQ algorithm.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern int IB_WSQCompressImage([In] byte[] idata, [Out] byte[] compressedData);

        public static byte[] _IB_WSQCompressImage(byte[] idata)
        {
            byte[] compressedData = new byte[SIZE_IMAGE];
            int result = IB_WSQCompressImage(idata, compressedData);
            if (result > 0)
            {
                int compressedLength = result;
                Array.Resize(ref compressedData, compressedLength);
            }
            else
            {
                compressedData = null;
            }
            return compressedData;
        }

		/*
		********************************************************************************************
		* _IB_WSQExpandImage()
		*
		* DESCRIPTION:
		*     Expand 352x288 Image with WSQ algorithm.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern int IB_WSQExpandImage([In] byte[] pCompressedImage, [In] int compressedLength, [Out] byte[] expandedImage);

        public static byte[] _IB_WSQExpandImage(byte[] compressedImage)
        {
            int result = 0;
            var expandedImage = new byte[SIZE_IMAGE];
            result = IB_WSQExpandImage(compressedImage, compressedImage.Length, expandedImage);
            if (result == SIZE_IMAGE)
            {
                // Good result
                return expandedImage;
            }
            else
            {
                return null;
            }
        }

		/*
		********************************************************************************************
		* _InvertImage()
		*
		* DESCRIPTION:
		*     Invert 352x288 Image, transforming a fingerprint Image with black background to a fingerprint
		*     Image with white background (or vice versa).
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern void InvertImage([In, Out] byte[] pImage);

        public static void _InvertImage(byte[] img)
        {
            InvertImage(img);
        }

		/*
		********************************************************************************************
		* _RotateImage()
		*
		* DESCRIPTION:
		*     Rotate 352x288 Image 90 degrees clockwise to become a 288x352 Image, possibly inverting pixels
		*     in the process.
		********************************************************************************************
		*/
        [DllImport(DLLFilename)]
        private static extern void RotateImage([In] byte[] inImage, [Out] byte[] outImage, byte invert);

        public static byte[] _RotateImage(byte[] imgIn, bool invert)
        {
            byte doInvert = invert ? (byte)1 : (byte)0;
            var outImage = new byte[SIZE_IMAGE];
            RotateImage(imgIn, outImage, doInvert);

            return outImage;
        }

        public BioNetACSDLL()
        {
        
        }
    }
}
