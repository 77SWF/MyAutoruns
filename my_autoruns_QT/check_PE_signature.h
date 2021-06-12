﻿#ifndef CHECK_PE_SIGNATURE_H
#define CHECK_PE_SIGNATURE_H

//-------------------------------------------------------------------
// Copyright (C) Microsoft.  All rights reserved.
// Example of verifying the embedded signature of a PE file by using
// the WinVerifyTrust function.

#define _UNICODE 1
#define UNICODE 1
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <softpub.h>

// Link with the Wintrust.lib file.
//https://docs.microsoft.com/zh-cn/windows/win32/seccrypto/example-c-program--verifying-the-signature-of-a-pe-file
BOOL VerifyEmbeddedSignature(LPCWSTR pwszSourceFile)
{
    LONG lStatus;
    DWORD dwLastError;
    bool Success = 0;
    // Initialize the WINTRUST_FILE_INFO structure.

    WINTRUST_FILE_INFO FileData;
    memset(&FileData, 0, sizeof(FileData));
    FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
    FileData.pcwszFilePath = pwszSourceFile;
    FileData.hFile = NULL;
    FileData.pgKnownSubject = NULL;

    /*
    WVTPolicyGUID specifies the policy to apply on the file
    WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:

    1) The certificate used to sign the file chains up to a root
    certificate located in the trusted root certificate store. This
    implies that the identity of the publisher has been verified by
    a certification authority.

    2) In cases where user interface is displayed (which this example
    does not do), WinVerifyTrust will check for whether the
    end entity certificate is stored in the trusted publisher store,
    implying that the user trusts content from this publisher.

    3) The end entity certificate has sufficient permission to sign
    code, as indicated by the presence of a code signing EKU or no
    EKU.
    */

    GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    WINTRUST_DATA WinTrustData;

    // Initialize the WinVerifyTrust input data structure.

    // Default all fields to 0.
    memset(&WinTrustData, 0, sizeof(WinTrustData));

    WinTrustData.cbStruct = sizeof(WinTrustData);

    // Use default code signing EKU.
    WinTrustData.pPolicyCallbackData = NULL;

    // No data to pass to SIP.
    WinTrustData.pSIPClientData = NULL;

    // Disable WVT UI.
    WinTrustData.dwUIChoice = WTD_UI_NONE;

    // No revocation checking.
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;

    // Verify an embedded signature on a file.
    WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;

    // Verify action.
    WinTrustData.dwStateAction = WTD_STATEACTION_VERIFY;

    // Verification sets this value.
    WinTrustData.hWVTStateData = NULL;

    // Not used.
    WinTrustData.pwszURLReference = NULL;

    // This is not applicable if there is no UI because it changes
    // the UI to accommodate running applications instead of
    // installing applications.
    WinTrustData.dwUIContext = 0;

    // Set pFile.
    WinTrustData.pFile = &FileData;

    // WinVerifyTrust verifies signatures as specified by the GUID
    // and Wintrust_Data.
    lStatus = WinVerifyTrust(
        NULL,
        &WVTPolicyGUID,
        &WinTrustData);

    switch (lStatus)
    {
        case ERROR_SUCCESS:
            /*
            Signed file:
                - Hash that represents the subject is trusted.

                - Trusted publisher without any verification errors.

                - UI was disabled in dwUIChoice. No publisher or
                    time stamp chain errors.

                - UI was enabled in dwUIChoice and the user clicked
                    "Yes" when asked to install and run the signed
                    subject.
            */
            Success = 1;
            // wprintf_s(L"The file \"%s\" is signed and the signature "
            //     L"was verified.\n",
            //     pwszSourceFile);
            // wprintf_s(L"The file \"%d\" is signed and the signature "
            // L"was verified.\n",
            // WVTPolicyGUID);
            break;

        case TRUST_E_NOSIGNATURE:
            // The file was not signed or had a signature
            // that was not valid.

            // Get the reason for no signature.
            dwLastError = GetLastError();
            if (TRUST_E_NOSIGNATURE == dwLastError ||
                    TRUST_E_SUBJECT_FORM_UNKNOWN == dwLastError ||
                    TRUST_E_PROVIDER_UNKNOWN == dwLastError)
            {
                // The file was not signed.
                Success = 0;
                // wprintf_s(L"The file \"%s\" is not signed.\n",
                //     pwszSourceFile);
            }
            else
            {
                // The signature was not valid or there was an error
                // opening the file.
                Success = 0;
                // wprintf_s(L"An unknown error occurred trying to "
                //     L"verify the signature of the \"%s\" file.\n",
                //     pwszSourceFile);
            }

            break;

        case TRUST_E_EXPLICIT_DISTRUST:
            // The hash that represents the subject or the publisher
            // is not allowed by the admin or user.
            Success = 0;
            // wprintf_s(L"The signature is present, but specifically "
            //     L"disallowed.\n");
            break;

        case TRUST_E_SUBJECT_NOT_TRUSTED:
            // The user clicked "No" when asked to install and run.
            Success = 0;
            // wprintf_s(L"The signature is present, but not "
            //     L"trusted.\n");
            break;

        case CRYPT_E_SECURITY_SETTINGS:
            /*
            The hash that represents the subject or the publisher
            was not explicitly trusted by the admin and the
            admin policy has disabled user trust. No signature,
            publisher or time stamp errors.
            */
            Success = 0;
            // wprintf_s(L"CRYPT_E_SECURITY_SETTINGS - The hash "
            //     L"representing the subject or the publisher wasn't "
            //     L"explicitly trusted by the admin and admin policy "
            //     L"has disabled user trust. No signature, publisher "
            //     L"or timestamp errors.\n");
            break;

        default:
            // The UI was disabled in dwUIChoice or the admin policy
            // has disabled user trust. lStatus contains the
            // publisher or time stamp chain error.
            Success = 0;
            // wprintf_s(L"Error is: 0x%x.\n",
            //     lStatus);
            break;
    }

    // Any hWVTStateData must be released by a call with close.
    WinTrustData.dwStateAction = WTD_STATEACTION_CLOSE;

    lStatus = WinVerifyTrust(
        NULL,
        &WVTPolicyGUID,
        &WinTrustData);

    return Success;
}






/*
 * An example of file signature verification using WinTrust API
 * Derived from the sample vertrust.cpp in the Platform SDK
 *
 * Copyright (c) 2009 Mounir IDRASSI <mounir.idrassi@idrix.fr>. All rights reserved.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */







#define _WIN32_WINNT 0x0500
#define WINVER       0x0500




#include <windows.h>
#include <Softpub.h>
#include <Wincrypt.h>
#include <tchar.h>
#include <stdlib.h>




#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Wintrust.lib")


//以下函数无效

LPTSTR GetCertificateDescription(PCCERT_CONTEXT pCertCtx)
{
   DWORD dwStrType;
   DWORD dwCount;
   LPTSTR szSubjectRDN = NULL;




   dwStrType = CERT_X500_NAME_STR;
   dwCount = CertGetNameString(pCertCtx,
      CERT_NAME_RDN_TYPE,
      0,
      &dwStrType,
      NULL,
      0);
   if (dwCount)
   {
      szSubjectRDN = (LPTSTR) LocalAlloc(0, dwCount * sizeof(TCHAR));
      CertGetNameString(pCertCtx,
         CERT_NAME_RDN_TYPE,
         0,
         &dwStrType,
         szSubjectRDN,
         dwCount);
   }




   return szSubjectRDN;
}


int verify(LPCWSTR file_path)
{
   GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
   WINTRUST_FILE_INFO sWintrustFileInfo;
   WINTRUST_DATA      sWintrustData;
   HRESULT            hr;

   memset((void*)&sWintrustFileInfo, 0x00, sizeof(WINTRUST_FILE_INFO));
   memset((void*)&sWintrustData, 0x00, sizeof(WINTRUST_DATA));




   sWintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
   sWintrustFileInfo.pcwszFilePath = file_path;
   sWintrustFileInfo.hFile = NULL;




   sWintrustData.cbStruct            = sizeof(WINTRUST_DATA);
   sWintrustData.dwUIChoice          = WTD_UI_NONE;
   sWintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
   sWintrustData.dwUnionChoice       = WTD_CHOICE_FILE;
   sWintrustData.pFile               = &sWintrustFileInfo;
   sWintrustData.dwStateAction       = WTD_STATEACTION_VERIFY;




   hr = WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);




   if (TRUST_E_NOSIGNATURE == hr)
   {
      _tprintf(_T("No signature found on the file.\n"));
   }
   else if (TRUST_E_BAD_DIGEST == hr)
   {
      _tprintf(_T("The signature of the file is invalid\n"));
   }
   else if (TRUST_E_PROVIDER_UNKNOWN == hr)
   {
      _tprintf(_T("No trust provider on this machine can verify this type of files.\n"));
   }
   else if (S_OK != hr)
   {
      _tprintf(_T("WinVerifyTrust failed with error 0x%.8X\n"), hr);
   }
   else
   {//验证成功？
      _tprintf(_T("File signature is OK.\n"));


      // retreive the signer certificate and display its information
      CRYPT_PROVIDER_DATA const *psProvData     = NULL;
      CRYPT_PROVIDER_SGNR       *psProvSigner   = NULL;
      CRYPT_PROVIDER_CERT       *psProvCert     = NULL;
      FILETIME                   localFt;
      SYSTEMTIME                 sysTime;




      psProvData = WTHelperProvDataFromStateData(sWintrustData.hWVTStateData);
      if (psProvData)
      {
         psProvSigner = WTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA)psProvData, 0 , FALSE, 0);
         if (psProvSigner)
         {
            FileTimeToLocalFileTime(&psProvSigner->sftVerifyAsOf, &localFt);
            FileTimeToSystemTime(&localFt, &sysTime);




            _tprintf(_T("Signature Date = %.2d/%.2d/%.4d at %.2d:%2.d:%.2d\n"), sysTime.wDay, sysTime.wMonth,sysTime.wYear, sysTime.wHour,sysTime.wMinute,sysTime.wSecond);




            psProvCert = WTHelperGetProvCertFromChain(psProvSigner, 0);
            if (psProvCert)
            {
               LPTSTR szCertDesc = GetCertificateDescription(psProvCert->pCert);
               if (szCertDesc)
               {
                  _tprintf(_T("File Signer = %s\n"), szCertDesc);
                  LocalFree(szCertDesc);
               }
            }




            if (psProvSigner->csCounterSigners)
            {
               _tprintf(_T("\n"));
               // Timestamp information
               FileTimeToLocalFileTime(&psProvSigner->pasCounterSigners[0].sftVerifyAsOf, &localFt);
               FileTimeToSystemTime(&localFt, &sysTime);


               _tprintf(_T("Timestamp Date = %.2d/%.2d/%.4d at %.2d:%2.d:%.2d\n"), sysTime.wDay, sysTime.wMonth,sysTime.wYear, sysTime.wHour,sysTime.wMinute,sysTime.wSecond);               
               psProvCert = WTHelperGetProvCertFromChain(&psProvSigner->pasCounterSigners[0], 0);
               if (psProvCert)
               {
                  LPTSTR szCertDesc = GetCertificateDescription(psProvCert->pCert);
                  if (szCertDesc)
                  {
                     _tprintf(_T("Timestamp Signer = %s\n"), szCertDesc);
                     LocalFree(szCertDesc);
                  }
               }
            }
         }
      }
      return 1;
   }
   
   sWintrustData.dwUIChoice = WTD_UI_NONE;
   sWintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
   WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);




return 0;
}
#endif // CHECK_PE_SIGNATURE_H
